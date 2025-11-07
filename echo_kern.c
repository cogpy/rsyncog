/*
 * Echo.Kern - Cognitive Kernel Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Implements the Echo.Kern cognitive kernel with GGML tensor operations.
 */

#include "echo_kern.h"
#include "atomspace.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

/* GGML integration - stub until GGML is linked */
#ifdef HAVE_GGML
#include <ggml/ggml.h>
#else
/* Stub GGML types when GGML is not available */
struct ggml_context {
    void *data;
    size_t size;
};

struct ggml_tensor {
    void *data;
    size_t ne[4];  /* dimensions */
    size_t nb[4];  /* strides */
    int type;
};

/* Stub GGML functions */
struct ggml_context *ggml_init(size_t size) {
    struct ggml_context *ctx = malloc(sizeof(struct ggml_context));
    if (ctx) {
        ctx->data = malloc(size);
        ctx->size = size;
    }
    return ctx;
}

void ggml_free(struct ggml_context *ctx) {
    if (ctx) {
        free(ctx->data);
        free(ctx);
    }
}

struct ggml_tensor *ggml_new_tensor_1d(struct ggml_context *ctx,
                                              int type, size_t ne0) {
    struct ggml_tensor *t = malloc(sizeof(struct ggml_tensor));
    if (t) {
        size_t elem_size = 4;  /* Assume F32 */
        t->data = calloc(ne0, elem_size);
        t->ne[0] = ne0;
        t->ne[1] = t->ne[2] = t->ne[3] = 1;
        t->type = type;
    }
    (void)ctx;  /* Unused in stub */
    return t;
}

struct ggml_tensor *ggml_new_tensor_2d(struct ggml_context *ctx,
                                              int type, size_t ne0, size_t ne1) {
    struct ggml_tensor *t = malloc(sizeof(struct ggml_tensor));
    if (t) {
        size_t elem_size = 4;  /* Assume F32 */
        t->data = calloc(ne0 * ne1, elem_size);
        t->ne[0] = ne0;
        t->ne[1] = ne1;
        t->ne[2] = t->ne[3] = 1;
        t->type = type;
    }
    (void)ctx;  /* Unused in stub */
    return t;
}

#define GGML_TYPE_F32 0
#endif

/* ============================================================================
 * Global Kernel State
 * ============================================================================ */

struct echo_kernel *g_kernel = NULL;

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * kern_get_time_ns - Get current time in nanoseconds
 */
uint64_t kern_get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * kern_log - Kernel logging
 */
void kern_log(const char *fmt, ...)
{
    va_list args;
    char buf[512];
    
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    printf("[KERN] %s\n", buf);
}

/**
 * ggml_tensor_data - Get tensor data pointer
 * @tensor: GGML tensor
 *
 * Returns: Data pointer
 */
void *ggml_tensor_data(struct ggml_tensor *tensor)
{
    return tensor ? tensor->data : NULL;
}

/* ============================================================================
 * Stage 0: Bootstrap & Initialization
 * ============================================================================ */

/**
 * stage0_init_kernel - Initialize the Echo.Kern cognitive kernel
 * @config: Kernel configuration (NULL for defaults)
 *
 * Returns: 0 on success, -1 on failure
 */
int stage0_init_kernel(struct kernel_config *config)
{
    int ret;
    
    kern_log("Initializing Echo.Kern cognitive kernel...");
    
    /* Allocate global kernel state */
    if (g_kernel) {
        kern_log("ERROR: Kernel already initialized");
        return -1;
    }
    
    g_kernel = calloc(1, sizeof(struct echo_kernel));
    if (!g_kernel) {
        kern_log("ERROR: Failed to allocate kernel state");
        return -1;
    }
    
    /* Set configuration (use defaults if not provided) */
    if (config) {
        memcpy(&g_kernel->config, config, sizeof(struct kernel_config));
    } else {
        /* Default configuration */
        g_kernel->config.memory_pool_size = KERN_DEFAULT_MEM_POOL_SIZE;
        g_kernel->config.tensor_mem_size = KERN_DEFAULT_TENSOR_MEM;
        g_kernel->config.scheduler_freq_hz = KERN_DEFAULT_SCHED_FREQ;
        g_kernel->config.max_tasks = KERN_DEFAULT_MAX_TASKS;
        g_kernel->config.reservoir_size = KERN_DEFAULT_RESERVOIR_SIZE;
        g_kernel->config.spectral_radius = KERN_DEFAULT_SPECTRAL_RADIUS;
        g_kernel->config.reservoir_sparsity = KERN_DEFAULT_SPARSITY;
        g_kernel->config.max_atoms = KERN_DEFAULT_MAX_ATOMS;
        g_kernel->config.max_membrane_depth = KERN_DEFAULT_MAX_DEPTH;
        g_kernel->config.max_tick_ns = KERN_DEFAULT_MAX_TICK_NS;
        g_kernel->config.max_alloc_ns = KERN_DEFAULT_MAX_ALLOC_NS;
        g_kernel->config.enable_ggml = 1;
        g_kernel->config.enable_realtime = 0;
        g_kernel->config.enable_debug = 0;
    }
    
    kern_log("Configuration: mem=%zu MB, tensor=%zu MB, sched=%u Hz",
             g_kernel->config.memory_pool_size / (1024*1024),
             g_kernel->config.tensor_mem_size / (1024*1024),
             g_kernel->config.scheduler_freq_hz);
    
    /* Initialize GGML context */
    if (g_kernel->config.enable_ggml) {
        g_kernel->ggml_ctx = ggml_init(g_kernel->config.tensor_mem_size);
        if (!g_kernel->ggml_ctx) {
            kern_log("ERROR: Failed to initialize GGML context");
            free(g_kernel);
            g_kernel = NULL;
            return -1;
        }
        kern_log("GGML context initialized (%zu MB)",
                 g_kernel->config.tensor_mem_size / (1024*1024));
    }
    
    /* Initialize memory subsystem */
    ret = kmem_init(g_kernel->config.memory_pool_size);
    if (ret < 0) {
        kern_log("ERROR: Failed to initialize memory subsystem");
        if (g_kernel->ggml_ctx)
            ggml_free(g_kernel->ggml_ctx);
        free(g_kernel);
        g_kernel = NULL;
        return -1;
    }
    
    /* Initialize hypergraph filesystem */
    g_kernel->hgfs = calloc(1, sizeof(struct hgfs_context));
    if (!g_kernel->hgfs) {
        kern_log("ERROR: Failed to allocate HGFS context");
        goto error_cleanup;
    }
    
    g_kernel->hgfs->ggml_ctx = g_kernel->ggml_ctx;
    g_kernel->hgfs->next_handle = 1;
    g_kernel->hgfs->next_edge_id = 1;
    
    /* Create adjacency matrix for hypergraph */
    if (g_kernel->config.enable_ggml && g_kernel->ggml_ctx) {
        size_t max_atoms = g_kernel->config.max_atoms;
        g_kernel->hgfs->adjacency = ggml_new_tensor_2d(
            g_kernel->ggml_ctx, GGML_TYPE_F32, max_atoms, max_atoms);
        
        if (g_kernel->hgfs->adjacency) {
            /* Initialize to zero */
            memset(g_kernel->hgfs->adjacency->data, 0,
                   max_atoms * max_atoms * sizeof(float));
        }
    }
    
    kern_log("HGFS initialized (max_atoms=%u)", g_kernel->config.max_atoms);
    
    /* Mark kernel as initialized */
    g_kernel->initialized = 1;
    g_kernel->running = 0;
    g_kernel->shutdown_requested = 0;
    
    kern_log("Echo.Kern initialization complete");
    return 0;

error_cleanup:
    if (g_kernel->hgfs)
        free(g_kernel->hgfs);
    if (g_kernel->kmem)
        free(g_kernel->kmem);
    if (g_kernel->ggml_ctx)
        ggml_free(g_kernel->ggml_ctx);
    free(g_kernel);
    g_kernel = NULL;
    return -1;
}

/**
 * stage0_shutdown_kernel - Clean shutdown of kernel resources
 */
void stage0_shutdown_kernel(void)
{
    if (!g_kernel) {
        kern_log("WARNING: Kernel not initialized");
        return;
    }
    
    kern_log("Shutting down Echo.Kern...");
    
    g_kernel->shutdown_requested = 1;
    g_kernel->running = 0;
    
    /* Print statistics */
    kern_log("Kernel Statistics:");
    kern_log("  Total ticks: %lu", g_kernel->stats.total_ticks);
    kern_log("  Total allocations: %lu", g_kernel->stats.total_allocations);
    kern_log("  Total frees: %lu", g_kernel->stats.total_frees);
    kern_log("  Peak memory: %lu bytes", g_kernel->stats.memory_peak);
    kern_log("  Avg tick time: %lu ns", g_kernel->stats.avg_tick_ns);
    kern_log("  Max tick time: %lu ns", g_kernel->stats.max_tick_ns);
    
    /* Free cognitive loop */
    if (g_kernel->cogloop) {
        free(g_kernel->cogloop);
        g_kernel->cogloop = NULL;
    }
    
    /* Free scheduler */
    if (g_kernel->sched) {
        /* Free task queues */
        struct task *t, *next;
        for (t = g_kernel->sched->ready_queue; t; t = next) {
            next = t->next;
            free(t);
        }
        for (t = g_kernel->sched->waiting_queue; t; t = next) {
            next = t->next;
            free(t);
        }
        
        /* Note: GGML tensors are freed with the GGML context, not individually */
        
        free(g_kernel->sched);
        g_kernel->sched = NULL;
    }
    
    /* Free HGFS */
    if (g_kernel->hgfs) {
        /* Free all nodes */
        struct hgfs_node *n, *next_n;
        for (n = g_kernel->hgfs->nodes; n; n = next_n) {
            next_n = n->next;
            /* Only free data if it's not part of a tensor */
            if (n->data && !n->tensor)
                free(n->data);
            /* Tensors are freed with GGML context, not individually */
            if (n->tensor && !g_kernel->config.enable_ggml)
                free(n->tensor);
            free(n);
        }
        
        /* Free all edges */
        struct hgfs_edge *e, *next_e;
        for (e = g_kernel->hgfs->edges; e; e = next_e) {
            next_e = e->next;
            free(e);
        }
        
        free(g_kernel->hgfs);
        g_kernel->hgfs = NULL;
    }
    
    /* Free memory pool */
    if (g_kernel->kmem) {
        if (g_kernel->kmem->base)
            free(g_kernel->kmem->base);
        free(g_kernel->kmem);
        g_kernel->kmem = NULL;
    }
    
    /* Free GGML context */
    if (g_kernel->ggml_ctx) {
        ggml_free(g_kernel->ggml_ctx);
        g_kernel->ggml_ctx = NULL;
    }
    
    /* Free kernel state */
    free(g_kernel);
    g_kernel = NULL;
    
    kern_log("Echo.Kern shutdown complete");
}

/* ============================================================================
 * Memory Subsystem
 * ============================================================================ */

/**
 * kmem_init - Initialize kernel memory subsystem
 * @pool_size: Size of memory pool in bytes
 *
 * Returns: 0 on success, -1 on failure
 */
int kmem_init(size_t pool_size)
{
    if (!g_kernel) {
        kern_log("ERROR: Kernel not initialized");
        return -1;
    }
    
    g_kernel->kmem = calloc(1, sizeof(struct kmem_pool));
    if (!g_kernel->kmem) {
        kern_log("ERROR: Failed to allocate memory pool structure");
        return -1;
    }
    
    g_kernel->kmem->size = pool_size;
    g_kernel->kmem->base = malloc(pool_size);
    if (!g_kernel->kmem->base) {
        kern_log("ERROR: Failed to allocate memory pool");
        free(g_kernel->kmem);
        g_kernel->kmem = NULL;
        return -1;
    }
    
    g_kernel->kmem->used = 0;
    g_kernel->kmem->peak = 0;
    
    /* Initialize slab allocator for common sizes */
    /* Sizes: 16, 32, 64, 128, 256, 512, 1024, 2048, ... */
    for (int i = 0; i < 16; i++) {
        g_kernel->kmem->slab_sizes[i] = 16 << i;
        g_kernel->kmem->slabs[i] = NULL;
    }
    
    kern_log("Memory pool initialized: %zu MB", pool_size / (1024*1024));
    return 0;
}

/**
 * kmem_tensor_alloc - Fast tensor memory allocation
 * @size: Size in bytes
 *
 * Returns: Pointer to allocated memory or NULL
 */
void *kmem_tensor_alloc(size_t size)
{
    uint64_t start_ns, end_ns, duration_ns;
    void *ptr;
    
    if (!g_kernel || !g_kernel->kmem) {
        kern_log("ERROR: Memory subsystem not initialized");
        return NULL;
    }
    
    start_ns = kern_get_time_ns();
    
    /* Simple bump allocator for now */
    /* TODO: Implement proper slab allocator */
    
    if (g_kernel->kmem->used + size > g_kernel->kmem->size) {
        kern_log("ERROR: Out of memory (requested=%zu, available=%zu)",
                 size, g_kernel->kmem->size - g_kernel->kmem->used);
        return NULL;
    }
    
    ptr = (char *)g_kernel->kmem->base + g_kernel->kmem->used;
    g_kernel->kmem->used += size;
    
    /* Update statistics */
    if (g_kernel->kmem->used > g_kernel->kmem->peak)
        g_kernel->kmem->peak = g_kernel->kmem->used;
    
    g_kernel->stats.total_allocations++;
    g_kernel->stats.memory_used = g_kernel->kmem->used;
    g_kernel->stats.memory_peak = g_kernel->kmem->peak;
    
    end_ns = kern_get_time_ns();
    duration_ns = end_ns - start_ns;
    
    /* Update allocation timing stats */
    if (duration_ns > g_kernel->stats.max_alloc_ns)
        g_kernel->stats.max_alloc_ns = duration_ns;
    
    /* Running average */
    if (g_kernel->stats.total_allocations > 1) {
        g_kernel->stats.avg_alloc_ns =
            (g_kernel->stats.avg_alloc_ns * (g_kernel->stats.total_allocations - 1) +
             duration_ns) / g_kernel->stats.total_allocations;
    } else {
        g_kernel->stats.avg_alloc_ns = duration_ns;
    }
    
    return ptr;
}

/**
 * kmem_tensor_free - Free tensor memory
 * @ptr: Pointer to free
 *
 * Note: Current implementation uses bump allocator, so free is a no-op
 * TODO: Implement proper slab allocator with free support
 */
void kmem_tensor_free(void *ptr)
{
    if (!g_kernel || !g_kernel->kmem)
        return;
    
    /* Bump allocator doesn't support free */
    /* Mark it in stats anyway */
    g_kernel->stats.total_frees++;
    
    (void)ptr;  /* Unused */
}

/* ============================================================================
 * Hypergraph Filesystem (HGFS)
 * ============================================================================ */

/**
 * hgfs_alloc - Allocate memory as GGML tensor node in hypergraph
 * @size: Size in bytes
 * @depth: Membrane depth (OEIS A000081)
 *
 * Returns: Pointer to allocated memory or NULL
 */
void *hgfs_alloc(size_t size, uint32_t depth)
{
    struct hgfs_node *node;
    struct ggml_tensor *tensor;
    
    if (!g_kernel || !g_kernel->hgfs) {
        kern_log("ERROR: HGFS not initialized");
        return NULL;
    }
    
    /* Allocate node structure */
    node = calloc(1, sizeof(struct hgfs_node));
    if (!node)
        return NULL;
    
    /* Create GGML tensor if enabled */
    if (g_kernel->config.enable_ggml && g_kernel->ggml_ctx) {
        /* Create 1D tensor with appropriate size */
        size_t elem_count = (size + 3) / 4;  /* Round up to F32 elements */
        tensor = ggml_new_tensor_1d(g_kernel->ggml_ctx, GGML_TYPE_F32, elem_count);
        
        if (!tensor) {
            free(node);
            return NULL;
        }
        
        node->tensor = tensor;
        node->data = tensor->data;
    } else {
        /* Fallback to regular allocation */
        node->data = malloc(size);
        if (!node->data) {
            free(node);
            return NULL;
        }
        node->tensor = NULL;
    }
    
    /* Initialize node */
    node->handle = g_kernel->hgfs->next_handle++;
    node->depth = depth;
    node->size = size;
    
    /* Add to node registry */
    node->next = g_kernel->hgfs->nodes;
    g_kernel->hgfs->nodes = node;
    g_kernel->hgfs->node_count++;
    
    return node->data;
}

/**
 * hgfs_free - Free hypergraph tensor node
 * @ptr: Pointer to free
 */
void hgfs_free(void *ptr)
{
    struct hgfs_node *node, *prev = NULL;
    
    if (!g_kernel || !g_kernel->hgfs || !ptr)
        return;
    
    /* Find node by data pointer */
    for (node = g_kernel->hgfs->nodes; node; prev = node, node = node->next) {
        if (node->data == ptr) {
            /* Remove from list */
            if (prev)
                prev->next = node->next;
            else
                g_kernel->hgfs->nodes = node->next;
            
            /* Free resources */
            if (node->data && node->data != node->tensor->data)
                free(node->data);
            if (node->tensor)
                free(node->tensor);
            free(node);
            
            g_kernel->hgfs->node_count--;
            return;
        }
    }
}

/**
 * hgfs_edge - Create hypergraph edge between tensor nodes
 * @src: Source node pointer
 * @dst: Destination node pointer
 * @type: Edge type
 *
 * Returns: 0 on success, -1 on failure
 */
int hgfs_edge(void *src, void *dst, enum hgfs_edge_type type)
{
    struct hgfs_node *src_node = NULL, *dst_node = NULL, *n;
    struct hgfs_edge *edge;
    
    if (!g_kernel || !g_kernel->hgfs || !src || !dst)
        return -1;
    
    /* Find source and destination nodes */
    for (n = g_kernel->hgfs->nodes; n; n = n->next) {
        if (n->data == src)
            src_node = n;
        if (n->data == dst)
            dst_node = n;
        if (src_node && dst_node)
            break;
    }
    
    if (!src_node || !dst_node) {
        kern_log("ERROR: Invalid node pointers for edge");
        return -1;
    }
    
    /* Create edge */
    edge = calloc(1, sizeof(struct hgfs_edge));
    if (!edge)
        return -1;
    
    edge->id = g_kernel->hgfs->next_edge_id++;
    edge->type = type;
    edge->src = src_node;
    edge->dst = dst_node;
    edge->weight = 1.0f;
    
    /* Add to edge registry */
    edge->next = g_kernel->hgfs->edges;
    g_kernel->hgfs->edges = edge;
    g_kernel->hgfs->edge_count++;
    
    /* Update adjacency matrix if available */
    if (g_kernel->hgfs->adjacency && src_node->handle < g_kernel->config.max_atoms &&
        dst_node->handle < g_kernel->config.max_atoms) {
        float *adj_data = (float *)g_kernel->hgfs->adjacency->data;
        size_t max_atoms = g_kernel->config.max_atoms;
        adj_data[src_node->handle * max_atoms + dst_node->handle] = 1.0f;
    }
    
    g_kernel->stats.total_edges++;
    
    return 0;
}

/* ============================================================================
 * Stub Implementations (To be completed in future phases)
 * ============================================================================ */

int cogloop_init(struct cogloop_config *config)
{
    kern_log("cogloop_init: Not yet implemented");
    (void)config;
    return -1;
}

int cogloop_step(void)
{
    kern_log("cogloop_step: Not yet implemented");
    return -1;
}

struct truth_value pln_eval_tensor(struct atom *atom)
{
    struct truth_value tv = {0};
    kern_log("pln_eval_tensor: Not yet implemented");
    (void)atom;
    return tv;
}

float pln_unify_graph(struct atom *pattern, struct atom *target)
{
    kern_log("pln_unify_graph: Not yet implemented");
    (void)pattern;
    (void)target;
    return 0.0f;
}

int pln_inference_step(void *pln_ctx)
{
    kern_log("pln_inference_step: Not yet implemented");
    (void)pln_ctx;
    return -1;
}

int kirq_register(int irq, irq_handler handler)
{
    kern_log("kirq_register: Not yet implemented");
    (void)irq;
    (void)handler;
    return -1;
}

int ksyscall_register(int num, syscall_handler handler)
{
    kern_log("ksyscall_register: Not yet implemented");
    (void)num;
    (void)handler;
    return -1;
}

int kio_init(void)
{
    kern_log("kio_init: Not yet implemented");
    return -1;
}

int ksync_mutex_init(struct kmutex *mutex)
{
    if (!mutex)
        return -1;
    mutex->locked = 0;
    mutex->owner = 0;
    return 0;
}

int ksync_mutex_lock(struct kmutex *mutex)
{
    if (!mutex)
        return -1;
    /* Simple spinlock (non-production) */
    while (__sync_lock_test_and_set(&mutex->locked, 1))
        ;
    mutex->owner = (uint64_t)pthread_self();
    return 0;
}

int ksync_mutex_unlock(struct kmutex *mutex)
{
    if (!mutex)
        return -1;
    mutex->owner = 0;
    __sync_lock_release(&mutex->locked);
    return 0;
}

int ktimer_init(void)
{
    kern_log("ktimer_init: Not yet implemented");
    return -1;
}

int ktimer_schedule(uint64_t nsec, timer_callback cb, void *arg)
{
    kern_log("ktimer_schedule: Not yet implemented");
    (void)nsec;
    (void)cb;
    (void)arg;
    return -1;
}

int kprot_set_domain(struct domain *domain)
{
    kern_log("kprot_set_domain: Not yet implemented");
    (void)domain;
    return -1;
}

int kabi_validate(struct abi_version *version)
{
    kern_log("kabi_validate: Not yet implemented");
    (void)version;
    return -1;
}
