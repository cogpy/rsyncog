/*
 * OpenCog Autonomous Agent Infrastructure for rsync
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef COGAGENT_H
#define COGAGENT_H

#include <stdint.h>
#include <time.h>

/* Forward declarations */
struct cog_agent;
struct atom_space;
struct cog_message;

/**
 * Cognitive Agent Types - Specialized agent roles
 */
typedef enum {
    COG_AGENT_ZERO,        /* Master builder/orchestrator */
    COG_AGENT_SYNC,        /* Sync coordination agent */
    COG_AGENT_MONITOR,     /* Monitoring and feedback agent */
    COG_AGENT_AUTH,        /* Authentication and security agent */
    COG_AGENT_SWARM,       /* HyperGNN swarm coordination */
    COG_AGENT_HYPERGRAPH   /* AtomSpace hypergraph manager */
} cog_agent_type;

/**
 * Agent State - Current operational state
 */
typedef enum {
    COG_STATE_INIT,        /* Initializing */
    COG_STATE_IDLE,        /* Waiting for tasks */
    COG_STATE_ACTIVE,      /* Actively processing */
    COG_STATE_COORDINATING,/* Coordinating with other agents */
    COG_STATE_ERROR,       /* Error state */
    COG_STATE_SHUTDOWN     /* Shutting down */
} cog_agent_state;

/**
 * Cognitive Message Types - Inter-agent communication
 */
typedef enum {
    COG_MSG_TASK,          /* Task assignment */
    COG_MSG_QUERY,         /* Query request */
    COG_MSG_RESPONSE,      /* Response to query */
    COG_MSG_STATUS,        /* Status update */
    COG_MSG_SYNC_REQ,      /* Sync request */
    COG_MSG_SYNC_ACK,      /* Sync acknowledgment */
    COG_MSG_SWARM_FORM,    /* Swarm formation command */
    COG_MSG_SWARM_UPDATE   /* Swarm state update */
} cog_msg_type;

/**
 * Cognitive Message Structure
 */
struct cog_message {
    cog_msg_type type;
    uint64_t msg_id;
    uint64_t sender_id;
    uint64_t receiver_id;
    time_t timestamp;
    void *payload;
    size_t payload_size;
    struct cog_message *next;
};

/**
 * Cognitive Agent Structure
 */
struct cog_agent {
    uint64_t agent_id;
    cog_agent_type type;
    cog_agent_state state;
    char name[256];
    
    /* Agent capabilities */
    uint32_t capabilities;
    
    /* Performance metrics */
    uint64_t tasks_processed;
    uint64_t tasks_failed;
    time_t last_active;
    
    /* Communication */
    struct cog_message *msg_queue;
    int msg_queue_size;
    
    /* Reference to shared AtomSpace */
    struct atom_space *atomspace;
    
    /* Agent-specific data */
    void *agent_data;
    
    /* Linked list for multi-agent coordination */
    struct cog_agent *next;
};

/* Agent capability flags */
#define COG_CAP_ORCHESTRATE   (1 << 0)  /* Can orchestrate other agents */
#define COG_CAP_SYNC          (1 << 1)  /* Can perform sync operations */
#define COG_CAP_AUTH          (1 << 2)  /* Can handle authentication */
#define COG_CAP_MONITOR       (1 << 3)  /* Can monitor operations */
#define COG_CAP_SWARM         (1 << 4)  /* Can participate in swarms */
#define COG_CAP_HYPERGRAPH    (1 << 5)  /* Can manage hypergraph structures */
#define COG_CAP_BUILD_CONFIG  (1 << 6)  /* Can build configurations */

/**
 * Agent Initialization and Lifecycle
 */
struct cog_agent *cog_agent_create(cog_agent_type type, const char *name);
void cog_agent_destroy(struct cog_agent *agent);
int cog_agent_init(struct cog_agent *agent, struct atom_space *atomspace);
int cog_agent_start(struct cog_agent *agent);
int cog_agent_stop(struct cog_agent *agent);

/**
 * Agent Communication
 */
int cog_agent_send_message(struct cog_agent *from, struct cog_agent *to,
                           cog_msg_type type, void *payload, size_t size);
struct cog_message *cog_agent_receive_message(struct cog_agent *agent);
void cog_message_destroy(struct cog_message *msg);

/**
 * Agent Coordination
 */
int cog_agent_register(struct cog_agent *agent);
struct cog_agent *cog_agent_find(uint64_t agent_id);
struct cog_agent *cog_agent_find_by_type(cog_agent_type type);
int cog_agent_broadcast(struct cog_agent *from, cog_msg_type type,
                       void *payload, size_t size);

/**
 * Task Processing
 */
typedef int (*cog_task_handler)(struct cog_agent *agent, void *task_data);
int cog_agent_process_tasks(struct cog_agent *agent, cog_task_handler handler);

#endif /* COGAGENT_H */
