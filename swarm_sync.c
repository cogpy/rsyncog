/*
 * HyperGNN Swarm Sync Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "swarm_sync.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Define strlcpy if not available */
#ifndef HAVE_STRLCPY
static size_t strlcpy(char *dest, const char *src, size_t size)
{
    size_t len = strlen(src);
    if (size > 0) {
        size_t copy_len = (len >= size) ? size - 1 : len;
        memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
    }
    return len;
}
#endif

/* Global swarm registry */
static struct swarm_formation *swarm_registry = NULL;
static uint64_t next_swarm_id = 1;

/**
 * swarm_create - Create new swarm formation
 * @coordinator: Coordinator agent for the swarm
 * @atomspace: Shared AtomSpace
 * @swarm_name: Name for the swarm
 *
 * Returns: New swarm formation or NULL on failure
 */
struct swarm_formation *swarm_create(struct cog_agent *coordinator,
                                    struct atom_space *atomspace,
                                    const char *swarm_name)
{
    struct swarm_formation *swarm;
    
    if (!coordinator || !atomspace || !swarm_name)
        return NULL;
    
    swarm = malloc(sizeof(struct swarm_formation));
    if (!swarm)
        return NULL;
    
    memset(swarm, 0, sizeof(struct swarm_formation));
    
    swarm->swarm_id = next_swarm_id++;
    strlcpy(swarm->name, swarm_name, sizeof(swarm->name));
    swarm->state = SWARM_FORMING;
    swarm->coordinator_agent = coordinator;
    swarm->formation_time = time(NULL);
    swarm->last_activity = swarm->formation_time;
    
    /* Create swarm atom in AtomSpace */
    swarm->swarm_atom = atomspace_add_node(atomspace, ATOM_SWARM, swarm_name);
    if (!swarm->swarm_atom) {
        free(swarm);
        return NULL;
    }
    
    /* Add to global registry */
    swarm->next = swarm_registry;
    swarm_registry = swarm;
    
    return swarm;
}

/**
 * swarm_destroy - Free swarm formation resources
 * @swarm: Swarm to destroy
 */
void swarm_destroy(struct swarm_formation *swarm)
{
    struct swarm_member *member, *next_member;
    
    if (!swarm)
        return;
    
    /* Free all members */
    member = swarm->members;
    while (member) {
        next_member = member->next;
        free(member);
        member = next_member;
    }
    
    free(swarm);
}

/**
 * swarm_add_member - Add member to swarm formation
 * @swarm: Swarm formation
 * @member_atom: AtomSpace atom for the member
 * @hostname: Member hostname
 * @port: Member rsync port
 *
 * Returns: 0 on success, -1 on failure
 */
int swarm_add_member(struct swarm_formation *swarm, struct atom *member_atom,
                    const char *hostname, int port)
{
    struct swarm_member *member;
    
    if (!swarm || !member_atom || !hostname)
        return -1;
    
    member = malloc(sizeof(struct swarm_member));
    if (!member)
        return -1;
    
    memset(member, 0, sizeof(struct swarm_member));
    
    member->member_atom = member_atom;
    strlcpy(member->hostname, hostname, sizeof(member->hostname));
    member->port = port;
    member->state = SWARM_FORMING;
    member->last_sync = time(NULL);
    
    /* Add to member list */
    member->next = swarm->members;
    swarm->members = member;
    swarm->member_count++;
    
    /* Update member's attention value */
    atom_set_sti(member_atom, 50);
    
    return 0;
}

/**
 * swarm_activate - Activate swarm for sync operations
 * @swarm: Swarm formation
 *
 * Returns: 0 on success, -1 on failure
 */
int swarm_activate(struct swarm_formation *swarm)
{
    struct swarm_member *member;
    
    if (!swarm)
        return -1;
    
    if (swarm->state != SWARM_FORMING)
        return -1;
    
    /* Activate all members */
    for (member = swarm->members; member; member = member->next) {
        member->state = SWARM_ACTIVE;
    }
    
    swarm->state = SWARM_ACTIVE;
    swarm->last_activity = time(NULL);
    
    /* Update swarm atom truth value */
    atom_set_tv(swarm->swarm_atom, 1.0, 0.95);
    
    return 0;
}

/**
 * swarm_disband - Disband swarm formation
 * @swarm: Swarm formation
 *
 * Returns: 0 on success, -1 on failure
 */
int swarm_disband(struct swarm_formation *swarm)
{
    struct swarm_member *member;
    
    if (!swarm)
        return -1;
    
    swarm->state = SWARM_DISBANDING;
    
    /* Update all members */
    for (member = swarm->members; member; member = member->next) {
        member->state = SWARM_DISBANDING;
    }
    
    /* Update swarm atom truth value */
    atom_set_tv(swarm->swarm_atom, 0.0, 1.0);
    
    return 0;
}

/**
 * swarm_sync_initiate - Initiate sync operation across swarm
 * @swarm: Swarm formation
 * @source_module: Source module name
 * @target_module: Target module name
 * @sync_flags: rsync flags
 *
 * Returns: 0 on success, -1 on failure
 */
int swarm_sync_initiate(struct swarm_formation *swarm,
                       const char *source_module,
                       const char *target_module,
                       int sync_flags)
{
    struct swarm_sync_request *req;
    struct swarm_member *member;
    
    if (!swarm || !source_module || !target_module)
        return -1;
    
    if (swarm->state != SWARM_ACTIVE)
        return -1;
    
    swarm->state = SWARM_COORDINATING;
    
    /* Create sync request */
    req = malloc(sizeof(struct swarm_sync_request));
    if (!req)
        return -1;
    
    memset(req, 0, sizeof(struct swarm_sync_request));
    req->swarm_id = swarm->swarm_id;
    strlcpy(req->source_module, source_module, sizeof(req->source_module));
    strlcpy(req->target_module, target_module, sizeof(req->target_module));
    req->sync_flags = sync_flags;
    
    /* Send sync request to all members via their agents */
    for (member = swarm->members; member; member = member->next) {
        if (member->member_agent) {
            cog_agent_send_message(swarm->coordinator_agent,
                                  member->member_agent,
                                  COG_MSG_SYNC_REQ,
                                  req, sizeof(*req));
        }
    }
    
    free(req);
    
    swarm->total_syncs++;
    swarm->last_activity = time(NULL);
    
    return 0;
}

/**
 * swarm_sync_coordinate - Coordinate swarm sync operations
 * @swarm: Swarm formation
 *
 * Returns: Number of active syncs, -1 on failure
 */
int swarm_sync_coordinate(struct swarm_formation *swarm)
{
    struct swarm_member *member;
    int active_count = 0;
    
    if (!swarm)
        return -1;
    
    /* Check status of all members */
    for (member = swarm->members; member; member = member->next) {
        if (member->state == SWARM_COORDINATING ||
            member->state == SWARM_ACTIVE) {
            active_count++;
        }
    }
    
    /* If all members complete, return to active state */
    if (active_count == 0 && swarm->state == SWARM_COORDINATING) {
        swarm->state = SWARM_ACTIVE;
    }
    
    return active_count;
}

/**
 * swarm_sync_broadcast - Broadcast sync to all swarm members
 * @swarm: Swarm formation
 * @module_name: Module to broadcast
 *
 * Returns: Number of members notified, -1 on failure
 */
int swarm_sync_broadcast(struct swarm_formation *swarm,
                        const char *module_name)
{
    struct swarm_member *member;
    int count = 0;
    
    if (!swarm || !module_name)
        return -1;
    
    /* Send swarm formation message to all members */
    for (member = swarm->members; member; member = member->next) {
        if (member->member_agent) {
            struct swarm_form_request req;
            memset(&req, 0, sizeof(req));
            strlcpy(req.swarm_name, swarm->name, sizeof(req.swarm_name));
            
            cog_agent_send_message(swarm->coordinator_agent,
                                  member->member_agent,
                                  COG_MSG_SWARM_FORM,
                                  &req, sizeof(req));
            count++;
        }
    }
    
    return count;
}

/**
 * swarm_get_state - Get current swarm state
 * @swarm: Swarm formation
 *
 * Returns: Current state
 */
swarm_state swarm_get_state(struct swarm_formation *swarm)
{
    if (!swarm)
        return SWARM_IDLE;
    
    return swarm->state;
}

/**
 * swarm_set_state - Set swarm state
 * @swarm: Swarm formation
 * @state: New state
 *
 * Returns: 0 on success, -1 on failure
 */
int swarm_set_state(struct swarm_formation *swarm, swarm_state state)
{
    if (!swarm)
        return -1;
    
    swarm->state = state;
    swarm->last_activity = time(NULL);
    
    return 0;
}

/**
 * swarm_get_statistics - Get swarm statistics
 * @swarm: Swarm formation
 * @total_syncs: Output for total sync count
 * @total_bytes: Output for total bytes synced
 *
 * Returns: 0 on success, -1 on failure
 */
int swarm_get_statistics(struct swarm_formation *swarm,
                        uint64_t *total_syncs,
                        uint64_t *total_bytes)
{
    if (!swarm)
        return -1;
    
    if (total_syncs)
        *total_syncs = swarm->total_syncs;
    
    if (total_bytes)
        *total_bytes = swarm->total_bytes;
    
    return 0;
}

/**
 * swarm_check_health - Check health of swarm formation
 * @swarm: Swarm formation
 *
 * Returns: Health score (0-100), -1 on failure
 */
int swarm_check_health(struct swarm_formation *swarm)
{
    struct swarm_member *member;
    int active_members = 0;
    int total_members = 0;
    time_t now = time(NULL);
    
    if (!swarm)
        return -1;
    
    /* Count active members */
    for (member = swarm->members; member; member = member->next) {
        total_members++;
        
        /* Consider member active if synced in last hour */
        if (now - member->last_sync < 3600) {
            active_members++;
        }
    }
    
    if (total_members == 0)
        return 0;
    
    /* Return health as percentage of active members */
    return (active_members * 100) / total_members;
}
