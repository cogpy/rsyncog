/*
 * Cognitive Agent Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "cogagent.h"
#include "atomspace.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

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

/* Global agent registry */
static struct cog_agent *agent_registry = NULL;
static uint64_t next_agent_id = 1;
static uint64_t next_msg_id = 1;

/**
 * cog_agent_create - Create a new cognitive agent
 * @type: Type of agent to create
 * @name: Human-readable name for the agent
 *
 * Returns: New agent instance or NULL on failure
 */
struct cog_agent *cog_agent_create(cog_agent_type type, const char *name)
{
    struct cog_agent *agent;
    
    agent = malloc(sizeof(struct cog_agent));
    if (!agent)
        return NULL;
    
    memset(agent, 0, sizeof(struct cog_agent));
    
    agent->agent_id = next_agent_id++;
    agent->type = type;
    agent->state = COG_STATE_INIT;
    
    if (name)
        strlcpy(agent->name, name, sizeof(agent->name));
    else
        snprintf(agent->name, sizeof(agent->name), "agent_%llu",
                (unsigned long long)agent->agent_id);
    
    /* Set capabilities based on type */
    switch (type) {
    case COG_AGENT_ZERO:
        agent->capabilities = COG_CAP_ORCHESTRATE | COG_CAP_BUILD_CONFIG;
        break;
    case COG_AGENT_SYNC:
        agent->capabilities = COG_CAP_SYNC;
        break;
    case COG_AGENT_MONITOR:
        agent->capabilities = COG_CAP_MONITOR;
        break;
    case COG_AGENT_AUTH:
        agent->capabilities = COG_CAP_AUTH;
        break;
    case COG_AGENT_SWARM:
        agent->capabilities = COG_CAP_SWARM | COG_CAP_SYNC;
        break;
    case COG_AGENT_HYPERGRAPH:
        agent->capabilities = COG_CAP_HYPERGRAPH;
        break;
    }
    
    return agent;
}

/**
 * cog_agent_destroy - Free agent resources
 * @agent: Agent to destroy
 */
void cog_agent_destroy(struct cog_agent *agent)
{
    struct cog_message *msg, *next_msg;
    
    if (!agent)
        return;
    
    /* Free message queue */
    msg = agent->msg_queue;
    while (msg) {
        next_msg = msg->next;
        cog_message_destroy(msg);
        msg = next_msg;
    }
    
    /* Free agent-specific data */
    if (agent->agent_data)
        free(agent->agent_data);
    
    free(agent);
}

/**
 * cog_agent_init - Initialize agent with AtomSpace
 * @agent: Agent to initialize
 * @atomspace: Shared AtomSpace instance
 *
 * Returns: 0 on success, -1 on failure
 */
int cog_agent_init(struct cog_agent *agent, struct atom_space *atomspace)
{
    if (!agent || !atomspace)
        return -1;
    
    agent->atomspace = atomspace;
    agent->state = COG_STATE_IDLE;
    agent->last_active = time(NULL);
    
    return 0;
}

/**
 * cog_agent_start - Start agent operations
 * @agent: Agent to start
 *
 * Returns: 0 on success, -1 on failure
 */
int cog_agent_start(struct cog_agent *agent)
{
    if (!agent)
        return -1;
    
    if (agent->state != COG_STATE_IDLE && agent->state != COG_STATE_INIT)
        return -1;
    
    agent->state = COG_STATE_ACTIVE;
    agent->last_active = time(NULL);
    
    return 0;
}

/**
 * cog_agent_stop - Stop agent operations
 * @agent: Agent to stop
 *
 * Returns: 0 on success, -1 on failure
 */
int cog_agent_stop(struct cog_agent *agent)
{
    if (!agent)
        return -1;
    
    agent->state = COG_STATE_SHUTDOWN;
    
    return 0;
}

/**
 * cog_agent_send_message - Send message from one agent to another
 * @from: Sending agent
 * @to: Receiving agent
 * @type: Message type
 * @payload: Message payload data
 * @size: Size of payload
 *
 * Returns: 0 on success, -1 on failure
 */
int cog_agent_send_message(struct cog_agent *from, struct cog_agent *to,
                           cog_msg_type type, void *payload, size_t size)
{
    struct cog_message *msg;
    
    if (!from || !to)
        return -1;
    
    msg = malloc(sizeof(struct cog_message));
    if (!msg)
        return -1;
    
    memset(msg, 0, sizeof(struct cog_message));
    
    msg->type = type;
    msg->msg_id = next_msg_id++;
    msg->sender_id = from->agent_id;
    msg->receiver_id = to->agent_id;
    msg->timestamp = time(NULL);
    
    if (payload && size > 0) {
        msg->payload = malloc(size);
        if (!msg->payload) {
            free(msg);
            return -1;
        }
        memcpy(msg->payload, payload, size);
        msg->payload_size = size;
    }
    
    /* Add to receiver's queue */
    msg->next = to->msg_queue;
    to->msg_queue = msg;
    to->msg_queue_size++;
    
    return 0;
}

/**
 * cog_agent_receive_message - Receive next message from queue
 * @agent: Agent receiving message
 *
 * Returns: Message or NULL if queue is empty
 */
struct cog_message *cog_agent_receive_message(struct cog_agent *agent)
{
    struct cog_message *msg;
    
    if (!agent || !agent->msg_queue)
        return NULL;
    
    msg = agent->msg_queue;
    agent->msg_queue = msg->next;
    agent->msg_queue_size--;
    msg->next = NULL;
    
    return msg;
}

/**
 * cog_message_destroy - Free message resources
 * @msg: Message to destroy
 */
void cog_message_destroy(struct cog_message *msg)
{
    if (!msg)
        return;
    
    if (msg->payload)
        free(msg->payload);
    
    free(msg);
}

/**
 * cog_agent_register - Register agent in global registry
 * @agent: Agent to register
 *
 * Returns: 0 on success, -1 on failure
 */
int cog_agent_register(struct cog_agent *agent)
{
    if (!agent)
        return -1;
    
    agent->next = agent_registry;
    agent_registry = agent;
    
    return 0;
}

/**
 * cog_agent_find - Find agent by ID
 * @agent_id: Agent identifier
 *
 * Returns: Agent or NULL if not found
 */
struct cog_agent *cog_agent_find(uint64_t agent_id)
{
    struct cog_agent *agent;
    
    for (agent = agent_registry; agent; agent = agent->next) {
        if (agent->agent_id == agent_id)
            return agent;
    }
    
    return NULL;
}

/**
 * cog_agent_find_by_type - Find first agent of given type
 * @type: Agent type to find
 *
 * Returns: Agent or NULL if not found
 */
struct cog_agent *cog_agent_find_by_type(cog_agent_type type)
{
    struct cog_agent *agent;
    
    for (agent = agent_registry; agent; agent = agent->next) {
        if (agent->type == type)
            return agent;
    }
    
    return NULL;
}

/**
 * cog_agent_broadcast - Broadcast message to all agents
 * @from: Sending agent
 * @type: Message type
 * @payload: Message payload
 * @size: Payload size
 *
 * Returns: Number of agents messaged, -1 on failure
 */
int cog_agent_broadcast(struct cog_agent *from, cog_msg_type type,
                       void *payload, size_t size)
{
    struct cog_agent *agent;
    int count = 0;
    
    if (!from)
        return -1;
    
    for (agent = agent_registry; agent; agent = agent->next) {
        if (agent != from) {
            if (cog_agent_send_message(from, agent, type, payload, size) == 0)
                count++;
        }
    }
    
    return count;
}

/**
 * cog_agent_process_tasks - Process agent tasks with handler
 * @agent: Agent to process tasks for
 * @handler: Task handler function
 *
 * Returns: Number of tasks processed, -1 on failure
 */
int cog_agent_process_tasks(struct cog_agent *agent, cog_task_handler handler)
{
    struct cog_message *msg;
    int count = 0;
    
    if (!agent || !handler)
        return -1;
    
    agent->state = COG_STATE_ACTIVE;
    agent->last_active = time(NULL);
    
    while ((msg = cog_agent_receive_message(agent)) != NULL) {
        if (msg->type == COG_MSG_TASK) {
            if (handler(agent, msg->payload) == 0) {
                agent->tasks_processed++;
                count++;
            } else {
                agent->tasks_failed++;
            }
        }
        cog_message_destroy(msg);
    }
    
    return count;
}
