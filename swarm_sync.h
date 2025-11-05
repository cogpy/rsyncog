/*
 * HyperGNN Swarm Sync Coordination
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef SWARM_SYNC_H
#define SWARM_SYNC_H

#include "cogagent.h"
#include "atomspace.h"

/**
 * Swarm Formation State
 */
typedef enum {
    SWARM_FORMING,         /* Forming swarm topology */
    SWARM_ACTIVE,          /* Active sync operations */
    SWARM_COORDINATING,    /* Coordinating between members */
    SWARM_IDLE,            /* Idle, waiting for sync */
    SWARM_DISBANDING       /* Disbanding formation */
} swarm_state;

/**
 * Swarm Member Info
 */
struct swarm_member {
    struct atom *member_atom;
    struct cog_agent *member_agent;
    char hostname[256];
    int port;
    swarm_state state;
    time_t last_sync;
    uint64_t bytes_synced;
    struct swarm_member *next;
};

/**
 * Swarm Formation Structure
 */
struct swarm_formation {
    uint64_t swarm_id;
    char name[256];
    swarm_state state;
    
    /* Swarm topology */
    struct swarm_member *members;
    size_t member_count;
    
    /* Coordination */
    struct cog_agent *coordinator_agent;
    struct atom *swarm_atom;
    
    /* Sync statistics */
    uint64_t total_syncs;
    uint64_t total_bytes;
    time_t formation_time;
    time_t last_activity;
    
    struct swarm_formation *next;
};

/**
 * Swarm Coordination Message Payloads
 */
struct swarm_form_request {
    char swarm_name[256];
    size_t member_count;
    char member_names[32][256];
};

struct swarm_sync_request {
    uint64_t swarm_id;
    char source_module[256];
    char target_module[256];
    int sync_flags;
};

struct swarm_status_update {
    uint64_t swarm_id;
    swarm_state state;
    uint64_t bytes_synced;
    time_t timestamp;
};

/**
 * Swarm Formation Management
 */
struct swarm_formation *swarm_create(struct cog_agent *coordinator,
                                    struct atom_space *atomspace,
                                    const char *swarm_name);
void swarm_destroy(struct swarm_formation *swarm);
int swarm_add_member(struct swarm_formation *swarm, struct atom *member_atom,
                    const char *hostname, int port);
int swarm_activate(struct swarm_formation *swarm);
int swarm_disband(struct swarm_formation *swarm);

/**
 * Swarm Sync Operations
 */
int swarm_sync_initiate(struct swarm_formation *swarm,
                       const char *source_module,
                       const char *target_module,
                       int sync_flags);
int swarm_sync_coordinate(struct swarm_formation *swarm);
int swarm_sync_broadcast(struct swarm_formation *swarm,
                        const char *module_name);

/**
 * Swarm State Management
 */
swarm_state swarm_get_state(struct swarm_formation *swarm);
int swarm_set_state(struct swarm_formation *swarm, swarm_state state);
struct swarm_member *swarm_get_members(struct swarm_formation *swarm,
                                      size_t *count);

/**
 * Swarm Monitoring
 */
int swarm_get_statistics(struct swarm_formation *swarm,
                        uint64_t *total_syncs,
                        uint64_t *total_bytes);
int swarm_check_health(struct swarm_formation *swarm);

#endif /* SWARM_SYNC_H */
