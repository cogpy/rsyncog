/*
 * AtomSpace Hypergraph Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "atomspace.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ATOM_TABLE_SIZE 1024
#define LINK_TABLE_SIZE 512
#define ATTENTION_QUEUE_SIZE 256

/* Hash function for atoms */
static uint32_t atom_hash(const char *name)
{
    uint32_t hash = 5381;
    int c;
    
    while ((c = *name++))
        hash = ((hash << 5) + hash) + c;
    
    return hash;
}

/**
 * atomspace_create - Create new AtomSpace instance
 *
 * Returns: New AtomSpace or NULL on failure
 */
struct atom_space *atomspace_create(void)
{
    struct atom_space *as;
    
    as = malloc(sizeof(struct atom_space));
    if (!as)
        return NULL;
    
    memset(as, 0, sizeof(struct atom_space));
    
    /* Initialize hash tables */
    as->atom_table_size = ATOM_TABLE_SIZE;
    as->atom_table = calloc(ATOM_TABLE_SIZE, sizeof(struct atom *));
    if (!as->atom_table) {
        free(as);
        return NULL;
    }
    
    as->link_table_size = LINK_TABLE_SIZE;
    as->link_table = calloc(LINK_TABLE_SIZE, sizeof(struct atom_link *));
    if (!as->link_table) {
        free(as->atom_table);
        free(as);
        return NULL;
    }
    
    /* Initialize attention queue */
    as->attention_queue_size = ATTENTION_QUEUE_SIZE;
    as->attention_queue = calloc(ATTENTION_QUEUE_SIZE, sizeof(struct atom *));
    if (!as->attention_queue) {
        free(as->link_table);
        free(as->atom_table);
        free(as);
        return NULL;
    }
    
    as->next_handle = 1;
    
    return as;
}

/**
 * atomspace_destroy - Free AtomSpace resources
 * @as: AtomSpace to destroy
 */
void atomspace_destroy(struct atom_space *as)
{
    size_t i;
    struct atom *atom, *next_atom;
    struct atom_link *link, *next_link;
    
    if (!as)
        return;
    
    /* Free all atoms */
    for (i = 0; i < as->atom_table_size; i++) {
        atom = as->atom_table[i];
        while (atom) {
            next_atom = atom->hash_next;
            if (atom->name)
                free(atom->name);
            if (atom->rsync_data)
                free(atom->rsync_data);
            free(atom);
            atom = next_atom;
        }
    }
    
    /* Free all links */
    for (i = 0; i < as->link_table_size; i++) {
        link = as->link_table[i];
        while (link) {
            next_link = link->hash_next;
            if (link->outgoing)
                free(link->outgoing);
            free(link);
            link = next_link;
        }
    }
    
    if (as->swarm_nodes)
        free(as->swarm_nodes);
    
    free(as->attention_queue);
    free(as->link_table);
    free(as->atom_table);
    free(as);
}

/**
 * atomspace_add_node - Add node to AtomSpace
 * @as: AtomSpace instance
 * @type: Node type
 * @name: Node name
 *
 * Returns: New atom or NULL on failure
 */
struct atom *atomspace_add_node(struct atom_space *as, atom_type type,
                               const char *name)
{
    struct atom *atom;
    uint32_t hash;
    size_t bucket;
    
    if (!as || !name)
        return NULL;
    
    /* Check if atom already exists */
    atom = atomspace_find_node(as, type, name);
    if (atom)
        return atom;
    
    /* Create new atom */
    atom = malloc(sizeof(struct atom));
    if (!atom)
        return NULL;
    
    memset(atom, 0, sizeof(struct atom));
    
    atom->handle = as->next_handle++;
    atom->type = type;
    atom->name = strdup(name);
    
    if (!atom->name) {
        free(atom);
        return NULL;
    }
    
    /* Initialize truth value */
    atom->tv.strength = 1.0;
    atom->tv.confidence = 0.0;
    
    /* Initialize attention value */
    atom->av.sti = 0;
    atom->av.lti = 0;
    atom->av.vlti = 0;
    
    atom->created = time(NULL);
    atom->last_accessed = atom->created;
    atom->access_count = 0;
    
    /* Add to hash table */
    hash = atom_hash(name);
    bucket = hash % as->atom_table_size;
    atom->hash_next = as->atom_table[bucket];
    as->atom_table[bucket] = atom;
    
    as->atom_count++;
    
    return atom;
}

/**
 * atomspace_get_atom - Get atom by handle
 * @as: AtomSpace instance
 * @handle: Atom handle
 *
 * Returns: Atom or NULL if not found
 */
struct atom *atomspace_get_atom(struct atom_space *as, uint64_t handle)
{
    size_t i;
    struct atom *atom;
    
    if (!as)
        return NULL;
    
    /* Linear search through hash table */
    for (i = 0; i < as->atom_table_size; i++) {
        for (atom = as->atom_table[i]; atom; atom = atom->hash_next) {
            if (atom->handle == handle) {
                atom->last_accessed = time(NULL);
                atom->access_count++;
                return atom;
            }
        }
    }
    
    return NULL;
}

/**
 * atomspace_find_node - Find node by type and name
 * @as: AtomSpace instance
 * @type: Node type
 * @name: Node name
 *
 * Returns: Atom or NULL if not found
 */
struct atom *atomspace_find_node(struct atom_space *as, atom_type type,
                                const char *name)
{
    uint32_t hash;
    size_t bucket;
    struct atom *atom;
    
    if (!as || !name)
        return NULL;
    
    hash = atom_hash(name);
    bucket = hash % as->atom_table_size;
    
    for (atom = as->atom_table[bucket]; atom; atom = atom->hash_next) {
        if (atom->type == type && strcmp(atom->name, name) == 0) {
            atom->last_accessed = time(NULL);
            atom->access_count++;
            return atom;
        }
    }
    
    return NULL;
}

/**
 * atomspace_add_link - Add link between atoms
 * @as: AtomSpace instance
 * @type: Link type
 * @outgoing: Array of atoms to link
 * @size: Number of atoms in outgoing set
 *
 * Returns: New link or NULL on failure
 */
struct atom_link *atomspace_add_link(struct atom_space *as, link_type type,
                                    struct atom **outgoing, size_t size)
{
    struct atom_link *link;
    size_t bucket;
    
    if (!as || !outgoing || size == 0)
        return NULL;
    
    link = malloc(sizeof(struct atom_link));
    if (!link)
        return NULL;
    
    memset(link, 0, sizeof(struct atom_link));
    
    link->handle = as->next_handle++;
    link->type = type;
    link->outgoing_size = size;
    
    link->outgoing = malloc(sizeof(struct atom *) * size);
    if (!link->outgoing) {
        free(link);
        return NULL;
    }
    
    memcpy(link->outgoing, outgoing, sizeof(struct atom *) * size);
    
    /* Initialize truth value */
    link->tv.strength = 1.0;
    link->tv.confidence = 0.0;
    
    link->created = time(NULL);
    
    /* Add to hash table */
    bucket = link->handle % as->link_table_size;
    link->hash_next = as->link_table[bucket];
    as->link_table[bucket] = link;
    
    as->link_count++;
    
    return link;
}

/**
 * atom_set_tv - Set truth value for atom
 * @atom: Atom to update
 * @strength: Truth strength [0.0, 1.0]
 * @confidence: Confidence [0.0, 1.0]
 */
void atom_set_tv(struct atom *atom, float strength, float confidence)
{
    if (!atom)
        return;
    
    atom->tv.strength = strength;
    atom->tv.confidence = confidence;
}

/**
 * atom_get_tv - Get truth value from atom
 * @atom: Atom to query
 *
 * Returns: Truth value
 */
struct truth_value atom_get_tv(struct atom *atom)
{
    struct truth_value tv = {0.0, 0.0};
    
    if (atom)
        tv = atom->tv;
    
    return tv;
}

/**
 * atom_set_sti - Set short-term importance
 * @atom: Atom to update
 * @sti: Short-term importance value
 */
void atom_set_sti(struct atom *atom, int16_t sti)
{
    if (!atom)
        return;
    
    atom->av.sti = sti;
}

/**
 * atom_set_lti - Set long-term importance
 * @atom: Atom to update
 * @lti: Long-term importance value
 */
void atom_set_lti(struct atom *atom, int16_t lti)
{
    if (!atom)
        return;
    
    atom->av.lti = lti;
}

/**
 * atomspace_build_sync_topology - Build sync topology from config
 * @as: AtomSpace instance
 * @config_file: Path to rsyncd.conf file
 *
 * Returns: 0 on success, -1 on failure
 */
int atomspace_build_sync_topology(struct atom_space *as,
                                 const char *config_file)
{
    struct atom *root;
    
    if (!as || !config_file)
        return -1;
    
    /* Create root topology node */
    root = atomspace_add_node(as, ATOM_CONCEPT, "sync_topology_root");
    if (!root)
        return -1;
    
    as->sync_topology_root = root;
    
    /* TODO: Parse config_file and build topology */
    
    return 0;
}

/**
 * atomspace_get_daemon_node - Get daemon node by name
 * @as: AtomSpace instance
 * @daemon_name: Daemon name
 *
 * Returns: Daemon atom or NULL if not found
 */
struct atom *atomspace_get_daemon_node(struct atom_space *as,
                                      const char *daemon_name)
{
    if (!as || !daemon_name)
        return NULL;
    
    return atomspace_find_node(as, ATOM_RSYNC_DAEMON, daemon_name);
}

/**
 * atomspace_create_swarm - Create swarm formation
 * @as: AtomSpace instance
 * @swarm_name: Swarm identifier
 * @members: Array of member atoms
 * @member_count: Number of members
 *
 * Returns: 0 on success, -1 on failure
 */
int atomspace_create_swarm(struct atom_space *as, const char *swarm_name,
                          struct atom **members, size_t member_count)
{
    struct atom *swarm_node;
    struct atom_link *membership_link;
    size_t i;
    
    if (!as || !swarm_name || !members || member_count == 0)
        return -1;
    
    /* Create swarm node */
    swarm_node = atomspace_add_node(as, ATOM_SWARM, swarm_name);
    if (!swarm_node)
        return -1;
    
    /* Create membership links */
    for (i = 0; i < member_count; i++) {
        struct atom *link_atoms[2];
        link_atoms[0] = swarm_node;
        link_atoms[1] = members[i];
        
        membership_link = atomspace_add_link(as, LINK_SWARM_MEMBER,
                                             link_atoms, 2);
        if (!membership_link)
            return -1;
    }
    
    return 0;
}
