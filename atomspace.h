/*
 * AtomSpace Hypergraph Data Structures for Distributed Sync Topology
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ATOMSPACE_H
#define ATOMSPACE_H

#include <stdint.h>
#include <time.h>

/* Forward declarations */
struct atom;
struct atom_link;
struct atom_space;

/**
 * Atom Types - Core hypergraph node types
 */
typedef enum {
    ATOM_NODE,             /* Basic node */
    ATOM_CONCEPT,          /* Concept node */
    ATOM_RSYNC_DAEMON,     /* rsync daemon node */
    ATOM_SYNC_PATH,        /* Sync path node */
    ATOM_HOST,             /* Host/server node */
    ATOM_MODULE,           /* rsync module node */
    ATOM_SWARM             /* Swarm formation node */
} atom_type;

/**
 * Link Types - Hypergraph edge types
 */
typedef enum {
    LINK_INHERITANCE,      /* Inheritance relationship */
    LINK_SIMILARITY,       /* Similarity relationship */
    LINK_SYNC_TOPOLOGY,    /* Sync topology edge */
    LINK_SWARM_MEMBER,     /* Swarm membership */
    LINK_AUTH_TRUST,       /* Authentication trust link */
    LINK_DEPENDENCY        /* Dependency relationship */
} link_type;

/**
 * Truth Value - Probabilistic logic representation
 */
struct truth_value {
    float strength;        /* Probability strength [0.0, 1.0] */
    float confidence;      /* Confidence in the value [0.0, 1.0] */
};

/**
 * Attention Value - Economic attention allocation
 */
struct attention_value {
    int16_t sti;           /* Short-term importance */
    int16_t lti;           /* Long-term importance */
    uint16_t vlti;         /* Very long-term importance */
};

/**
 * Atom - Fundamental hypergraph node
 */
struct atom {
    uint64_t handle;       /* Unique identifier */
    atom_type type;
    char *name;
    
    /* Cognitive values */
    struct truth_value tv;
    struct attention_value av;
    
    /* Metadata */
    time_t created;
    time_t last_accessed;
    uint32_t access_count;
    
    /* rsync-specific data */
    void *rsync_data;
    size_t rsync_data_size;
    
    /* Hash table bucket for lookups */
    struct atom *hash_next;
};

/**
 * Atom Link - Hypergraph edge connecting atoms
 */
struct atom_link {
    uint64_t handle;
    link_type type;
    
    /* Connected atoms (outgoing set) */
    struct atom **outgoing;
    size_t outgoing_size;
    
    /* Cognitive values */
    struct truth_value tv;
    struct attention_value av;
    
    /* Metadata */
    time_t created;
    
    /* Hash table bucket */
    struct atom_link *hash_next;
};

/**
 * AtomSpace - Hypergraph knowledge base
 */
struct atom_space {
    /* Hash tables for fast lookup */
    struct atom **atom_table;
    size_t atom_table_size;
    struct atom_link **link_table;
    size_t link_table_size;
    
    /* Statistics */
    uint64_t atom_count;
    uint64_t link_count;
    uint64_t next_handle;
    
    /* Attention allocation */
    struct atom **attention_queue;
    size_t attention_queue_size;
    
    /* Sync topology graph */
    struct atom *sync_topology_root;
    
    /* Swarm formations */
    struct atom **swarm_nodes;
    size_t swarm_count;
};

/**
 * AtomSpace Operations
 */
struct atom_space *atomspace_create(void);
void atomspace_destroy(struct atom_space *as);

/**
 * Atom Operations
 */
struct atom *atomspace_add_node(struct atom_space *as, atom_type type,
                               const char *name);
struct atom *atomspace_get_atom(struct atom_space *as, uint64_t handle);
struct atom *atomspace_find_node(struct atom_space *as, atom_type type,
                                const char *name);
int atomspace_remove_atom(struct atom_space *as, uint64_t handle);

/**
 * Link Operations
 */
struct atom_link *atomspace_add_link(struct atom_space *as, link_type type,
                                    struct atom **outgoing, size_t size);
struct atom_link *atomspace_get_link(struct atom_space *as, uint64_t handle);
int atomspace_remove_link(struct atom_space *as, uint64_t handle);

/**
 * Truth Value Operations
 */
void atom_set_tv(struct atom *atom, float strength, float confidence);
struct truth_value atom_get_tv(struct atom *atom);

/**
 * Attention Allocation (ECAN)
 */
void atom_set_sti(struct atom *atom, int16_t sti);
void atom_set_lti(struct atom *atom, int16_t lti);
int atomspace_update_attention(struct atom_space *as);

/**
 * Sync Topology Management
 */
int atomspace_build_sync_topology(struct atom_space *as,
                                 const char *config_file);
struct atom *atomspace_get_daemon_node(struct atom_space *as,
                                      const char *daemon_name);
struct atom **atomspace_get_sync_paths(struct atom_space *as,
                                      struct atom *daemon,
                                      size_t *count);

/**
 * Swarm Formation Management
 */
int atomspace_create_swarm(struct atom_space *as, const char *swarm_name,
                          struct atom **members, size_t member_count);
struct atom **atomspace_get_swarm_members(struct atom_space *as,
                                         const char *swarm_name,
                                         size_t *count);
int atomspace_update_swarm(struct atom_space *as, const char *swarm_name);

/**
 * Serialization for Distributed Deployment
 */
int atomspace_serialize(struct atom_space *as, const char *filename);
struct atom_space *atomspace_deserialize(const char *filename);

#endif /* ATOMSPACE_H */
