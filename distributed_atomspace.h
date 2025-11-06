/*
 * Distributed AtomSpace - Cross-Node Sync for True Distributed Cognition
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Enables synchronization of AtomSpace itself across distributed nodes,
 * allowing true distributed cognitive processing.
 */

#ifndef DISTRIBUTED_ATOMSPACE_H
#define DISTRIBUTED_ATOMSPACE_H

#include "atomspace.h"
#include "swarm_sync.h"
#include <stdint.h>

/* Forward declarations */
struct distributed_atomspace;
struct atomspace_node;
struct atomspace_sync_state;

/**
 * AtomSpace Node - Represents a node in distributed network
 */
struct atomspace_node {
    uint64_t node_id;
    char *hostname;
    int port;
    
    /* Node state */
    int connected;
    time_t last_sync;
    uint64_t atoms_synced;
    
    /* Local AtomSpace mirror */
    struct atom_space *remote_mirror;
    
    struct atomspace_node *next;
};

/**
 * Sync Conflict Resolution Strategy
 */
typedef enum {
    CONFLICT_LATEST_WINS,      /* Most recent update wins */
    CONFLICT_HIGHEST_CONFIDENCE, /* Highest confidence wins */
    CONFLICT_MERGE_TV,         /* Merge truth values using PLN */
    CONFLICT_MANUAL            /* Manual resolution required */
} conflict_strategy;

/**
 * AtomSpace Sync State - Tracks synchronization status
 */
struct atomspace_sync_state {
    uint64_t atoms_sent;
    uint64_t atoms_received;
    uint64_t links_sent;
    uint64_t links_received;
    uint64_t conflicts_resolved;
    time_t last_full_sync;
    time_t last_incremental_sync;
};

/**
 * Distributed AtomSpace Context
 */
struct distributed_atomspace {
    struct atom_space *local_atomspace;
    
    /* Network of nodes */
    struct atomspace_node *nodes;
    size_t node_count;
    
    /* Sync configuration */
    conflict_strategy conflict_resolution;
    int enable_incremental_sync;
    int enable_bidirectional_sync;
    time_t sync_interval;
    
    /* Sync state */
    struct atomspace_sync_state sync_state;
    
    /* Swarm integration */
    struct swarm_formation *atomspace_swarm;
};

/**
 * Distributed AtomSpace Functions
 */

/**
 * distributed_atomspace_create - Create distributed AtomSpace context
 * @local_atomspace: Local AtomSpace instance
 *
 * Returns: New distributed context or NULL on failure
 */
struct distributed_atomspace *distributed_atomspace_create(
    struct atom_space *local_atomspace);

/**
 * distributed_atomspace_destroy - Free distributed AtomSpace
 * @das: Distributed AtomSpace to destroy
 */
void distributed_atomspace_destroy(struct distributed_atomspace *das);

/**
 * distributed_atomspace_add_node - Add node to distributed network
 * @das: Distributed AtomSpace
 * @hostname: Node hostname
 * @port: Node port
 *
 * Returns: Node ID or 0 on failure
 */
uint64_t distributed_atomspace_add_node(struct distributed_atomspace *das,
                                       const char *hostname,
                                       int port);

/**
 * distributed_atomspace_connect - Connect to remote node
 * @das: Distributed AtomSpace
 * @node_id: Node to connect to
 *
 * Returns: 0 on success, -1 on failure
 */
int distributed_atomspace_connect(struct distributed_atomspace *das,
                                 uint64_t node_id);

/**
 * distributed_atomspace_sync_full - Perform full synchronization
 * @das: Distributed AtomSpace
 * @node_id: Target node (0 for all nodes)
 *
 * Returns: 0 on success, -1 on failure
 */
int distributed_atomspace_sync_full(struct distributed_atomspace *das,
                                   uint64_t node_id);

/**
 * distributed_atomspace_sync_incremental - Sync only changes since last sync
 * @das: Distributed AtomSpace
 * @node_id: Target node (0 for all nodes)
 *
 * Returns: Number of atoms synchronized
 */
int distributed_atomspace_sync_incremental(struct distributed_atomspace *das,
                                          uint64_t node_id);

/**
 * distributed_atomspace_sync_atom - Sync specific atom to network
 * @das: Distributed AtomSpace
 * @atom: Atom to synchronize
 *
 * Returns: Number of nodes synced to
 */
int distributed_atomspace_sync_atom(struct distributed_atomspace *das,
                                   struct atom *atom);

/**
 * distributed_atomspace_resolve_conflict - Resolve sync conflict
 * @das: Distributed AtomSpace
 * @local_atom: Local atom version
 * @remote_atom: Remote atom version
 *
 * Returns: Resolved atom
 */
struct atom *distributed_atomspace_resolve_conflict(
    struct distributed_atomspace *das,
    struct atom *local_atom,
    struct atom *remote_atom);

/**
 * distributed_atomspace_subscribe - Subscribe to remote atom updates
 * @das: Distributed AtomSpace
 * @node_id: Node to subscribe to
 * @atom_type: Type of atoms to watch (0 for all)
 *
 * Returns: 0 on success, -1 on failure
 */
int distributed_atomspace_subscribe(struct distributed_atomspace *das,
                                   uint64_t node_id,
                                   atom_type type);

/**
 * distributed_atomspace_get_sync_state - Get synchronization statistics
 * @das: Distributed AtomSpace
 *
 * Returns: Sync state structure
 */
struct atomspace_sync_state distributed_atomspace_get_sync_state(
    struct distributed_atomspace *das);

/**
 * distributed_atomspace_set_conflict_strategy - Set conflict resolution
 * @das: Distributed AtomSpace
 * @strategy: Conflict resolution strategy
 *
 * Returns: 0 on success, -1 on failure
 */
int distributed_atomspace_set_conflict_strategy(
    struct distributed_atomspace *das,
    conflict_strategy strategy);

#endif /* DISTRIBUTED_ATOMSPACE_H */
