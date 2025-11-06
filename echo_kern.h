/*
 * Echo.Kern - Cognitive Kernel Core Definitions
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Defines the core kernel structures and interfaces for the Echo.Kern
 * cognitive kernel implementation.
 */

#ifndef ECHO_KERN_H
#define ECHO_KERN_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* Forward declarations */
struct ggml_context;
struct ggml_tensor;

/* ============================================================================
 * Kernel Configuration
 * ============================================================================ */

/**
 * struct kernel_config - Kernel initialization configuration
 */
struct kernel_config {
    /* Memory configuration */
    size_t memory_pool_size;        /* Total memory pool size in bytes */
    size_t tensor_mem_size;         /* GGML tensor memory size */
    
    /* Scheduler configuration */
    uint32_t scheduler_freq_hz;     /* Scheduler tick frequency */
    uint32_t max_tasks;             /* Maximum concurrent tasks */
    
    /* ESN Reservoir configuration */
    uint32_t reservoir_size;        /* ESN reservoir neurons */
    float spectral_radius;          /* ESN spectral radius */
    float reservoir_sparsity;       /* ESN connection sparsity */
    
    /* Hypergraph configuration */
    uint32_t max_atoms;             /* Maximum atoms in hypergraph */
    uint32_t max_membrane_depth;    /* Maximum P-system depth */
    
    /* Real-time constraints */
    uint64_t max_tick_ns;           /* Max scheduler tick (nanoseconds) */
    uint64_t max_alloc_ns;          /* Max allocation time (nanoseconds) */
    
    /* Flags */
    uint32_t enable_ggml : 1;       /* Enable GGML tensor operations */
    uint32_t enable_realtime : 1;   /* Enable real-time mode */
    uint32_t enable_debug : 1;      /* Enable debug mode */
    uint32_t reserved : 29;
};

/* Default configuration values */
#define KERN_DEFAULT_MEM_POOL_SIZE     (256 * 1024 * 1024)  /* 256 MB */
#define KERN_DEFAULT_TENSOR_MEM        (128 * 1024 * 1024)  /* 128 MB */
#define KERN_DEFAULT_SCHED_FREQ        1000                  /* 1 kHz */
#define KERN_DEFAULT_MAX_TASKS         256
#define KERN_DEFAULT_RESERVOIR_SIZE    1024
#define KERN_DEFAULT_SPECTRAL_RADIUS   0.95f
#define KERN_DEFAULT_SPARSITY          0.1f
#define KERN_DEFAULT_MAX_ATOMS         10000
#define KERN_DEFAULT_MAX_DEPTH         16
#define KERN_DEFAULT_MAX_TICK_NS       5000                  /* 5 µs */
#define KERN_DEFAULT_MAX_ALLOC_NS      100                   /* 100 ns */

/**
 * struct kernel_stats - Kernel runtime statistics
 */
struct kernel_stats {
    uint64_t total_ticks;           /* Total scheduler ticks */
    uint64_t total_allocations;     /* Total memory allocations */
    uint64_t total_frees;           /* Total memory frees */
    uint64_t total_edges;           /* Total hypergraph edges */
    
    uint64_t avg_tick_ns;           /* Average tick time (ns) */
    uint64_t max_tick_ns;           /* Maximum tick time (ns) */
    uint64_t avg_alloc_ns;          /* Average allocation time (ns) */
    uint64_t max_alloc_ns;          /* Maximum allocation time (ns) */
    
    uint64_t memory_used;           /* Current memory usage */
    uint64_t memory_peak;           /* Peak memory usage */
    
    uint32_t active_tasks;          /* Currently active tasks */
    uint32_t peak_tasks;            /* Peak concurrent tasks */
};

/* ============================================================================
 * Memory Subsystem
 * ============================================================================ */

/**
 * struct kmem_pool - Kernel memory pool
 */
struct kmem_pool {
    void *base;                     /* Base address */
    size_t size;                    /* Total size */
    size_t used;                    /* Used bytes */
    size_t peak;                    /* Peak usage */
    
    /* Slab allocator for common sizes */
    void *slabs[16];                /* Size-specific allocators */
    size_t slab_sizes[16];          /* Slab sizes */
};

/* ============================================================================
 * Hypergraph Filesystem (HGFS)
 * ============================================================================ */

/**
 * enum hgfs_edge_type - Hypergraph edge types
 */
enum hgfs_edge_type {
    HGFS_EDGE_INHERITANCE = 0,      /* Inheritance relationship */
    HGFS_EDGE_SIMILARITY,           /* Similarity relationship */
    HGFS_EDGE_SYNC_TOPOLOGY,        /* Sync topology link */
    HGFS_EDGE_SWARM_MEMBER,         /* Swarm membership */
    HGFS_EDGE_AUTH_TRUST,           /* Authentication trust */
    HGFS_EDGE_DEPENDENCY,           /* Dependency relationship */
    HGFS_EDGE_TEMPORAL,             /* Temporal relationship */
    HGFS_EDGE_CAUSAL,               /* Causal relationship */
};

/**
 * struct hgfs_node - Hypergraph node (tensor-backed)
 */
struct hgfs_node {
    uint64_t handle;                /* Unique node handle */
    uint32_t depth;                 /* Membrane depth (OEIS A000081) */
    size_t size;                    /* Allocation size */
    
    struct ggml_tensor *tensor;     /* Backing GGML tensor */
    void *data;                     /* Data pointer */
    
    struct hgfs_node *next;         /* Linked list */
};

/**
 * struct hgfs_edge - Hypergraph edge
 */
struct hgfs_edge {
    uint64_t id;                    /* Edge ID */
    enum hgfs_edge_type type;       /* Edge type */
    
    struct hgfs_node *src;          /* Source node */
    struct hgfs_node *dst;          /* Destination node */
    
    float weight;                   /* Edge weight */
    
    struct hgfs_edge *next;         /* Linked list */
};

/**
 * struct hgfs_context - HGFS global context
 */
struct hgfs_context {
    struct ggml_context *ggml_ctx;  /* GGML context for tensors */
    
    struct hgfs_node *nodes;        /* Node registry */
    struct hgfs_edge *edges;        /* Edge registry */
    
    uint64_t next_handle;           /* Next node handle */
    uint64_t next_edge_id;          /* Next edge ID */
    
    uint32_t node_count;            /* Current node count */
    uint32_t edge_count;            /* Current edge count */
    
    struct ggml_tensor *adjacency;  /* Adjacency matrix */
};

/* ============================================================================
 * DTESN Scheduler
 * ============================================================================ */

/**
 * enum task_state - Task states
 */
enum task_state {
    TASK_READY = 0,                 /* Ready to run */
    TASK_RUNNING,                   /* Currently running */
    TASK_WAITING,                   /* Waiting for event */
    TASK_SLEEPING,                  /* Sleeping */
    TASK_ZOMBIE,                    /* Terminated */
};

/**
 * struct task - Kernel task
 */
struct task {
    uint64_t tid;                   /* Task ID */
    enum task_state state;          /* Current state */
    
    /* Attention values (ECAN) */
    int32_t sti;                    /* Short-term importance */
    int32_t lti;                    /* Long-term importance */
    
    /* Execution context */
    void (*entry)(void *arg);       /* Entry point */
    void *arg;                      /* Argument */
    
    /* Timing */
    uint64_t wake_time_ns;          /* Wake time for sleeping tasks */
    
    struct task *next;              /* Linked list */
};

/**
 * struct dtesn_config - DTESN scheduler configuration
 */
struct dtesn_config {
    uint32_t reservoir_size;        /* ESN reservoir neurons */
    float spectral_radius;          /* Spectral radius */
    float sparsity;                 /* Connection sparsity */
    
    uint32_t input_dim;             /* Input dimension */
    uint32_t output_dim;            /* Output dimension */
};

/**
 * struct dtesn_scheduler - DTESN scheduler state
 */
struct dtesn_scheduler {
    struct dtesn_config config;     /* Configuration */
    
    /* ESN Reservoir (GGML tensors) */
    struct ggml_context *ggml_ctx;  /* GGML context */
    struct ggml_tensor *W_reservoir; /* Reservoir weight matrix */
    struct ggml_tensor *W_input;    /* Input weight matrix */
    struct ggml_tensor *W_output;   /* Output weight matrix */
    struct ggml_tensor *state;      /* Current reservoir state */
    
    /* Task queues */
    struct task *ready_queue;       /* Ready tasks */
    struct task *waiting_queue;     /* Waiting tasks */
    
    /* Current task */
    struct task *current;           /* Currently running task */
    
    /* Statistics */
    uint64_t tick_count;            /* Total ticks */
    uint64_t context_switches;      /* Total context switches */
};

/* ============================================================================
 * P-System Membranes
 * ============================================================================ */

/**
 * struct membrane - P-system membrane
 */
struct membrane {
    uint32_t id;                    /* Membrane ID */
    uint32_t depth;                 /* Depth in hierarchy */
    
    struct membrane *parent;        /* Parent membrane */
    struct membrane *children;      /* Child membranes */
    struct membrane *sibling;       /* Sibling membranes */
    
    /* Evolution rules */
    void (*evolve)(struct membrane *m);
    
    /* Contents */
    void *data;                     /* Membrane data */
    size_t data_size;               /* Data size */
};

/* ============================================================================
 * Cognitive Loop
 * ============================================================================ */

/**
 * struct cogloop_config - Cognitive loop configuration
 */
struct cogloop_config {
    uint32_t cycle_freq_hz;         /* Cognitive cycle frequency */
    uint32_t perception_steps;      /* Perception steps per cycle */
    uint32_t reasoning_steps;       /* Reasoning steps per cycle */
    uint32_t action_steps;          /* Action steps per cycle */
};

/**
 * struct cogloop_context - Cognitive loop state
 */
struct cogloop_context {
    struct cogloop_config config;   /* Configuration */
    
    struct dtesn_scheduler *sched;  /* Scheduler */
    void *atomspace;                /* AtomSpace pointer */
    void *pln_ctx;                  /* PLN context */
    
    uint64_t cycle_count;           /* Total cycles */
    uint64_t avg_cycle_ns;          /* Average cycle time */
};

/* ============================================================================
 * Kernel Global State
 * ============================================================================ */

/**
 * struct echo_kernel - Global kernel state
 */
struct echo_kernel {
    struct kernel_config config;    /* Configuration */
    struct kernel_stats stats;      /* Statistics */
    
    /* Subsystems */
    struct kmem_pool *kmem;         /* Memory pool */
    struct hgfs_context *hgfs;      /* Hypergraph filesystem */
    struct dtesn_scheduler *sched;  /* Scheduler */
    struct cogloop_context *cogloop; /* Cognitive loop */
    
    /* GGML global context */
    struct ggml_context *ggml_ctx;  /* Master GGML context */
    
    /* State flags */
    uint32_t initialized : 1;
    uint32_t running : 1;
    uint32_t shutdown_requested : 1;
    uint32_t reserved : 29;
};

/* Global kernel instance (defined in echo_kern.c) */
extern struct echo_kernel *g_kernel;

/* ============================================================================
 * Kernel API Functions
 * ============================================================================ */

/* Bootstrap */
int stage0_init_kernel(struct kernel_config *config);
void stage0_shutdown_kernel(void);

/* Memory */
int kmem_init(size_t pool_size);
void *kmem_tensor_alloc(size_t size);
void kmem_tensor_free(void *ptr);

/* HGFS */
void *hgfs_alloc(size_t size, uint32_t depth);
void hgfs_free(void *ptr);
int hgfs_edge(void *src, void *dst, enum hgfs_edge_type type);

/* Scheduler */
int dtesn_sched_init(struct dtesn_config *config);
int dtesn_sched_tick(void);
int dtesn_sched_enqueue(struct task *task);
int dtesn_mem_init_regions(uint32_t max_depth);

/* Cognitive Loop */
int cogloop_init(struct cogloop_config *config);
int cogloop_step(void);

/* PLN Tensor Operations */
struct truth_value;  /* Forward declaration */
struct atom;         /* Forward declaration */

struct truth_value pln_eval_tensor(struct atom *atom);
float pln_unify_graph(struct atom *pattern, struct atom *target);
int pln_inference_step(void *pln_ctx);

/* Interrupts */
typedef void (*irq_handler)(int irq);
typedef int (*syscall_handler)(int num, void *args);

int kirq_register(int irq, irq_handler handler);
int ksyscall_register(int num, syscall_handler handler);

/* I/O & Sync */
int kio_init(void);

struct kmutex {
    uint32_t locked;
    uint64_t owner;
};

int ksync_mutex_init(struct kmutex *mutex);
int ksync_mutex_lock(struct kmutex *mutex);
int ksync_mutex_unlock(struct kmutex *mutex);

/* Timers */
typedef void (*timer_callback)(void *arg);

int ktimer_init(void);
int ktimer_schedule(uint64_t nsec, timer_callback cb, void *arg);

/* Protection & ABI */
struct domain {
    uint32_t id;
    uint32_t capabilities;
};

struct abi_version {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
};

int kprot_set_domain(struct domain *domain);
int kabi_validate(struct abi_version *version);

/* Utility functions */
uint64_t kern_get_time_ns(void);  /* Get current time in nanoseconds */
void kern_log(const char *fmt, ...);  /* Kernel logging */

#endif /* ECHO_KERN_H */
