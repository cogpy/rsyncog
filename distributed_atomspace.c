/*
 * Distributed AtomSpace Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Implements network protocol for distributed AtomSpace synchronization.
 */

#include "distributed_atomspace.h"
#include "pln_inference.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/* Protocol constants */
#define ATOMSPACE_PROTO_VERSION 1
#define ATOMSPACE_MAGIC 0x41544F4D  /* "ATOM" */
#define MAX_ATOM_NAME_LEN 1024
#define SYNC_BUFFER_SIZE 65536

/* Message types */
enum {
    MSG_SYNC_REQUEST = 1,
    MSG_SYNC_ATOM,
    MSG_SYNC_LINK,
    MSG_SYNC_COMPLETE,
    MSG_SYNC_ACK,
    MSG_CONFLICT_NOTIFY
};

/**
 * Atom serialization header
 */
struct atom_header {
    uint32_t magic;
    uint32_t version;
    uint8_t msg_type;
    uint64_t atom_handle;
    uint8_t atom_type;
    uint32_t name_len;
    struct truth_value tv;
    struct attention_value av;
    time_t timestamp;
} __attribute__((packed));

/**
 * Link serialization header
 */
struct link_header {
    uint32_t magic;
    uint32_t version;
    uint8_t msg_type;
    uint64_t link_handle;
    uint8_t link_type;
    uint32_t arity;
    struct truth_value tv;
    struct attention_value av;
    time_t timestamp;
} __attribute__((packed));

/**
 * Helper: find node by ID
 */
static struct atomspace_node *find_node(struct distributed_atomspace *das, uint64_t node_id)
{
    struct atomspace_node *node;
    for (node = das->nodes; node; node = node->next) {
        if (node->node_id == node_id)
            return node;
    }
    return NULL;
}

/**
 * Serialize atom to buffer
 */
static int serialize_atom(struct atom *atom, char *buffer, size_t bufsize, size_t *out_len)
{
    struct atom_header hdr;
    size_t name_len, total_len;
    char *ptr;
    
    if (!atom || !buffer || !out_len)
        return -1;
    
    name_len = atom->name ? strlen(atom->name) : 0;
    total_len = sizeof(hdr) + name_len;
    
    if (total_len > bufsize)
        return -1;
    
    /* Build header */
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = ATOMSPACE_MAGIC;
    hdr.version = ATOMSPACE_PROTO_VERSION;
    hdr.msg_type = MSG_SYNC_ATOM;
    hdr.atom_handle = atom->handle;
    hdr.atom_type = atom->type;
    hdr.name_len = name_len;
    hdr.tv = atom->tv;
    hdr.av = atom->av;
    hdr.timestamp = time(NULL);
    
    /* Copy to buffer */
    ptr = buffer;
    memcpy(ptr, &hdr, sizeof(hdr));
    ptr += sizeof(hdr);
    
    if (name_len > 0) {
        memcpy(ptr, atom->name, name_len);
    }
    
    *out_len = total_len;
    return 0;
}

/**
 * Deserialize atom from buffer
 */
static struct atom *deserialize_atom(const char *buffer, size_t bufsize)
{
    const struct atom_header *hdr;
    struct atom *atom;
    const char *name_ptr;
    char *name_copy = NULL;
    
    if (!buffer || bufsize < sizeof(struct atom_header))
        return NULL;
    
    hdr = (const struct atom_header *)buffer;
    
    /* Validate header */
    if (hdr->magic != ATOMSPACE_MAGIC || hdr->version != ATOMSPACE_PROTO_VERSION)
        return NULL;
    
    if (sizeof(struct atom_header) + hdr->name_len > bufsize)
        return NULL;
    
    /* Extract name */
    if (hdr->name_len > 0) {
        name_ptr = buffer + sizeof(struct atom_header);
        name_copy = malloc(hdr->name_len + 1);
        if (!name_copy)
            return NULL;
        memcpy(name_copy, name_ptr, hdr->name_len);
        name_copy[hdr->name_len] = '\0';
    }
    
    /* Create atom */
    atom = malloc(sizeof(struct atom));
    if (!atom) {
        free(name_copy);
        return NULL;
    }
    
    memset(atom, 0, sizeof(struct atom));
    atom->handle = hdr->atom_handle;
    atom->type = hdr->atom_type;
    atom->name = name_copy;
    atom->tv = hdr->tv;
    atom->av = hdr->av;
    
    return atom;
}

/**
 * Connect to remote node over TCP
 */
static int connect_to_node(const char *hostname, int port)
{
    int sock;
    struct addrinfo hints, *result, *rp;
    char port_str[16];
    int ret;
    
    /* Setup hints for getaddrinfo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    /* Convert port to string */
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    /* Resolve hostname */
    ret = getaddrinfo(hostname, port_str, &hints, &result);
    if (ret != 0)
        return -1;
    
    /* Try each address until we successfully connect */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0)
            continue;
        
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;  /* Success */
        
        close(sock);
    }
    
    freeaddrinfo(result);
    
    if (rp == NULL)
        return -1;  /* No address succeeded */
    
    /* Set non-blocking mode */
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    return sock;
}

/**
 * Send atom over network
 */
static int send_atom(int sock, struct atom *atom)
{
    char buffer[SYNC_BUFFER_SIZE];
    size_t len;
    ssize_t sent;
    
    if (serialize_atom(atom, buffer, sizeof(buffer), &len) < 0)
        return -1;
    
    sent = send(sock, buffer, len, 0);
    if (sent < 0 || (size_t)sent != len)
        return -1;
    
    return 0;
}

/**
 * Receive atom from network
 */
static struct atom *receive_atom(int sock)
{
    char buffer[SYNC_BUFFER_SIZE];
    ssize_t received;
    struct atom *atom;
    
    received = recv(sock, buffer, sizeof(buffer), 0);
    if (received <= 0)
        return NULL;
    
    atom = deserialize_atom(buffer, received);
    return atom;
}

/**
 * distributed_atomspace_create - Create distributed AtomSpace
 */
struct distributed_atomspace *distributed_atomspace_create(
    struct atom_space *local_atomspace)
{
    struct distributed_atomspace *das;
    
    if (!local_atomspace)
        return NULL;
    
    das = malloc(sizeof(struct distributed_atomspace));
    if (!das)
        return NULL;
    
    memset(das, 0, sizeof(struct distributed_atomspace));
    
    das->local_atomspace = local_atomspace;
    das->conflict_resolution = CONFLICT_MERGE_TV;
    das->enable_incremental_sync = 1;
    das->enable_bidirectional_sync = 1;
    das->sync_interval = 60; /* 1 minute default */
    
    return das;
}

/**
 * distributed_atomspace_destroy - Free distributed AtomSpace
 */
void distributed_atomspace_destroy(struct distributed_atomspace *das)
{
    struct atomspace_node *node, *next;
    
    if (!das)
        return;
    
    /* Free all nodes */
    node = das->nodes;
    while (node) {
        next = node->next;
        if (node->hostname)
            free(node->hostname);
        if (node->remote_mirror)
            atomspace_destroy(node->remote_mirror);
        free(node);
        node = next;
    }
    
    free(das);
}

/**
 * distributed_atomspace_add_node - Add node to network
 */
uint64_t distributed_atomspace_add_node(struct distributed_atomspace *das,
                                       const char *hostname,
                                       int port)
{
    struct atomspace_node *node;
    static uint64_t next_id = 1;
    
    if (!das || !hostname)
        return 0;
    
    node = malloc(sizeof(struct atomspace_node));
    if (!node)
        return 0;
    
    memset(node, 0, sizeof(struct atomspace_node));
    
    node->node_id = next_id++;
    node->hostname = strdup(hostname);
    node->port = port;
    node->connected = 0;
    
    /* Create mirror AtomSpace for this node */
    node->remote_mirror = atomspace_create();
    
    /* Add to list */
    node->next = das->nodes;
    das->nodes = node;
    das->node_count++;
    
    return node->node_id;
}

/**
 * distributed_atomspace_connect - Connect to remote node
 */
int distributed_atomspace_connect(struct distributed_atomspace *das,
                                 uint64_t node_id)
{
    struct atomspace_node *node;
    int sock;
    
    if (!das)
        return -1;
    
    /* Find node */
    node = find_node(das, node_id);
    if (!node)
        return -1;
    
    /* Connect to remote */
    sock = connect_to_node(node->hostname, node->port);
    if (sock < 0)
        return -1;
    
    /* Store socket (reusing atoms_synced field as socket fd) */
    node->atoms_synced = sock;
    node->connected = 1;
    
    return 0;
}

/**
 * distributed_atomspace_sync_full - Full synchronization
 */
int distributed_atomspace_sync_full(struct distributed_atomspace *das,
                                   uint64_t node_id)
{
    struct atomspace_node *node;
    struct atom *atom;
    size_t i, j;
    int synced = 0;
    
    if (!das)
        return -1;
    
    /* Sync to all nodes if node_id is 0 */
    for (node = das->nodes; node; node = node->next) {
        if (node_id == 0 || node->node_id == node_id) {
            if (!node->connected)
                continue;
            
            /* Send all atoms from local AtomSpace hash table */
            for (i = 0; i < das->local_atomspace->atom_table_size; i++) {
                atom = das->local_atomspace->atom_table[i];
                while (atom) {
                    /* Send atom over network */
                    if (send_atom(node->atoms_synced, atom) == 0) {
                        synced++;
                    }
                    atom = atom->hash_next;
                }
            }
            
            /* Also send attention queue atoms */
            for (j = 0; j < das->local_atomspace->attention_queue_size; j++) {
                atom = das->local_atomspace->attention_queue[j];
                if (atom) {
                    send_atom(node->atoms_synced, atom);
                }
            }
            
            node->last_sync = time(NULL);
            das->sync_state.atoms_sent += synced;
            das->sync_state.last_full_sync = time(NULL);
            
            if (node_id != 0)
                break;
        }
    }
    
    return synced;
}

/**
 * distributed_atomspace_sync_incremental - Incremental sync (stub)
 */
int distributed_atomspace_sync_incremental(struct distributed_atomspace *das,
                                          uint64_t node_id)
{
    if (!das)
        return -1;
    
    /* TODO: Implement incremental sync based on timestamps */
    das->sync_state.last_incremental_sync = time(NULL);
    
    return 0;
}

/**
 * distributed_atomspace_sync_atom - Sync single atom (stub)
 */
int distributed_atomspace_sync_atom(struct distributed_atomspace *das,
                                   struct atom *atom)
{
    struct atomspace_node *node;
    int synced = 0;
    
    if (!das || !atom)
        return 0;
    
    /* Sync to all connected nodes */
    for (node = das->nodes; node; node = node->next) {
        if (node->connected) {
            /* TODO: Send atom to remote node */
            node->atoms_synced++;
            synced++;
        }
    }
    
    das->sync_state.atoms_sent++;
    
    return synced;
}

/**
 * distributed_atomspace_resolve_conflict - Resolve conflicts
 */
struct atom *distributed_atomspace_resolve_conflict(
    struct distributed_atomspace *das,
    struct atom *local_atom,
    struct atom *remote_atom)
{
    struct pln_inference_context *pln_ctx;
    struct truth_value merged_tv;
    
    if (!das || !local_atom || !remote_atom)
        return local_atom;
    
    switch (das->conflict_resolution) {
    case CONFLICT_LATEST_WINS:
        /* Use most recently modified */
        return (local_atom->last_accessed > remote_atom->last_accessed) ?
               local_atom : remote_atom;
        
    case CONFLICT_HIGHEST_CONFIDENCE:
        /* Use atom with highest confidence */
        return (local_atom->tv.confidence > remote_atom->tv.confidence) ?
               local_atom : remote_atom;
        
    case CONFLICT_MERGE_TV:
        /* Merge truth values using PLN revision */
        pln_ctx = pln_context_create(das->local_atomspace);
        if (pln_ctx) {
            merged_tv = pln_apply_revision(pln_ctx, local_atom->tv, remote_atom->tv);
            local_atom->tv = merged_tv;
            pln_context_destroy(pln_ctx);
        }
        das->sync_state.conflicts_resolved++;
        return local_atom;
        
    case CONFLICT_MANUAL:
    default:
        /* Keep local version, log conflict */
        return local_atom;
    }
}

/**
 * distributed_atomspace_subscribe - Subscribe to updates (stub)
 */
int distributed_atomspace_subscribe(struct distributed_atomspace *das,
                                   uint64_t node_id,
                                   atom_type type)
{
    if (!das)
        return -1;
    
    /* TODO: Implement subscription mechanism */
    return 0;
}

/**
 * distributed_atomspace_get_sync_state - Get sync statistics
 */
struct atomspace_sync_state distributed_atomspace_get_sync_state(
    struct distributed_atomspace *das)
{
    struct atomspace_sync_state state = {0};
    
    if (das)
        state = das->sync_state;
    
    return state;
}

/**
 * distributed_atomspace_set_conflict_strategy - Set conflict resolution
 */
int distributed_atomspace_set_conflict_strategy(
    struct distributed_atomspace *das,
    conflict_strategy strategy)
{
    if (!das)
        return -1;
    
    das->conflict_resolution = strategy;
    return 0;
}
