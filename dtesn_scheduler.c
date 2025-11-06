/*
 * Echo.Kern DTESN Scheduler Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Implements the Differential Tensor Echo State Network (DTESN) scheduler
 * with reservoir dynamics for cognitive task scheduling.
 */

#include "echo_kern.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* GGML stub - matches echo_kern.c */
#ifndef HAVE_GGML
extern struct ggml_tensor *ggml_new_tensor_1d(void *ctx, int type, size_t ne0);
extern struct ggml_tensor *ggml_new_tensor_2d(void *ctx, int type, size_t ne0, size_t ne1);
#define GGML_TYPE_F32 0
#endif

/* ============================================================================
 * ESN Reservoir Utilities
 * ============================================================================ */

/**
 * random_uniform - Generate random float in [0, 1]
 */
static float random_uniform(void)
{
    return (float)rand() / (float)RAND_MAX;
}

/**
 * random_normal - Generate random float with normal distribution
 */
static float random_normal(void)
{
    /* Box-Muller transform */
    float u1 = random_uniform();
    float u2 = random_uniform();
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * M_PI * u2);
}

/**
 * init_reservoir_weights - Initialize ESN reservoir weight matrix
 * @W: Weight matrix (size x size)
 * @size: Reservoir size
 * @spectral_radius: Target spectral radius
 * @sparsity: Connection sparsity (0-1)
 *
 * Initializes a sparse random reservoir matrix and scales to desired
 * spectral radius for echo state property.
 */
static void init_reservoir_weights(float *W, size_t size,
                                   float spectral_radius, float sparsity)
{
    size_t i, j;
    float scale;
    
    /* Initialize with sparse random connections */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (random_uniform() > sparsity) {
                W[i * size + j] = (random_uniform() * 2.0f - 1.0f);
            } else {
                W[i * size + j] = 0.0f;
            }
        }
    }
    
    /* Scale to desired spectral radius */
    /* Note: This is approximate - full spectral radius calculation requires
     * eigenvalue computation. For production, use LAPACK or similar. */
    scale = spectral_radius / 1.5f;  /* Empirical scaling factor */
    
    for (i = 0; i < size * size; i++) {
        W[i] *= scale;
    }
}

/**
 * init_input_weights - Initialize ESN input weight matrix
 * @W_in: Input weight matrix (size x input_dim)
 * @size: Reservoir size
 * @input_dim: Input dimension
 */
static void init_input_weights(float *W_in, size_t size, size_t input_dim)
{
    size_t i;
    
    for (i = 0; i < size * input_dim; i++) {
        W_in[i] = (random_uniform() * 2.0f - 1.0f) * 0.5f;
    }
}

/**
 * init_output_weights - Initialize ESN output weight matrix
 * @W_out: Output weight matrix (output_dim x size)
 * @output_dim: Output dimension
 * @size: Reservoir size
 */
static void init_output_weights(float *W_out, size_t output_dim, size_t size)
{
    size_t i;
    
    for (i = 0; i < output_dim * size; i++) {
        W_out[i] = (random_uniform() * 2.0f - 1.0f) * 0.1f;
    }
}

/**
 * tanh_activation - Hyperbolic tangent activation function
 */
static inline float tanh_activation(float x)
{
    return tanhf(x);
}

/**
 * reservoir_update - Update ESN reservoir state
 * @state: Current state vector (size)
 * @input: Input vector (input_dim)
 * @W_res: Reservoir weights (size x size)
 * @W_in: Input weights (size x input_dim)
 * @size: Reservoir size
 * @input_dim: Input dimension
 * @leak_rate: Leak rate (0-1)
 *
 * Updates reservoir state: state = (1-α)*state + α*tanh(W_res*state + W_in*input)
 */
static void reservoir_update(float *state, const float *input,
                            const float *W_res, const float *W_in,
                            size_t size, size_t input_dim, float leak_rate)
{
    float *new_state = alloca(size * sizeof(float));
    size_t i, j;
    
    /* Compute W_res * state + W_in * input */
    for (i = 0; i < size; i++) {
        float activation = 0.0f;
        
        /* Reservoir recurrent connections */
        for (j = 0; j < size; j++) {
            activation += W_res[i * size + j] * state[j];
        }
        
        /* Input connections */
        for (j = 0; j < input_dim; j++) {
            activation += W_in[i * input_dim + j] * input[j];
        }
        
        /* Apply activation function */
        new_state[i] = tanh_activation(activation);
    }
    
    /* Leaky integration: state = (1-α)*state + α*new_state */
    for (i = 0; i < size; i++) {
        state[i] = (1.0f - leak_rate) * state[i] + leak_rate * new_state[i];
    }
}

/**
 * compute_output - Compute output from reservoir state
 * @output: Output vector (output_dim)
 * @state: Reservoir state (size)
 * @W_out: Output weights (output_dim x size)
 * @output_dim: Output dimension
 * @size: Reservoir size
 */
static void compute_output(float *output, const float *state,
                          const float *W_out, size_t output_dim, size_t size)
{
    size_t i, j;
    
    for (i = 0; i < output_dim; i++) {
        output[i] = 0.0f;
        for (j = 0; j < size; j++) {
            output[i] += W_out[i * size + j] * state[j];
        }
    }
}

/* ============================================================================
 * DTESN Scheduler Implementation
 * ============================================================================ */

/**
 * dtesn_sched_init - Initialize tensorized ESN reservoir scheduler
 * @config: Scheduler configuration
 *
 * Returns: 0 on success, -1 on failure
 */
int dtesn_sched_init(struct dtesn_config *config)
{
    struct dtesn_scheduler *sched;
    struct dtesn_config default_config;
    size_t res_size, in_size, out_size;
    
    if (!g_kernel) {
        kern_log("ERROR: Kernel not initialized");
        return -1;
    }
    
    if (g_kernel->sched) {
        kern_log("ERROR: Scheduler already initialized");
        return -1;
    }
    
    kern_log("Initializing DTESN scheduler...");
    
    /* Use provided config or defaults */
    if (config) {
        memcpy(&default_config, config, sizeof(struct dtesn_config));
    } else {
        default_config.reservoir_size = KERN_DEFAULT_RESERVOIR_SIZE;
        default_config.spectral_radius = KERN_DEFAULT_SPECTRAL_RADIUS;
        default_config.sparsity = KERN_DEFAULT_SPARSITY;
        default_config.input_dim = 64;
        default_config.output_dim = 32;
    }
    
    /* Allocate scheduler structure */
    sched = calloc(1, sizeof(struct dtesn_scheduler));
    if (!sched) {
        kern_log("ERROR: Failed to allocate scheduler");
        return -1;
    }
    
    memcpy(&sched->config, &default_config, sizeof(struct dtesn_config));
    
    /* Use kernel's GGML context or create new one */
    sched->ggml_ctx = g_kernel->ggml_ctx;
    
    /* Allocate reservoir matrices as GGML tensors */
    res_size = sched->config.reservoir_size;
    in_size = sched->config.input_dim;
    out_size = sched->config.output_dim;
    
    kern_log("Allocating ESN tensors (reservoir=%zu, input=%zu, output=%zu)",
             res_size, in_size, out_size);
    
    /* Reservoir weight matrix (res_size x res_size) */
    sched->W_reservoir = ggml_new_tensor_2d(sched->ggml_ctx, GGML_TYPE_F32,
                                            res_size, res_size);
    if (!sched->W_reservoir) {
        kern_log("ERROR: Failed to allocate reservoir weights");
        free(sched);
        return -1;
    }
    
    /* Input weight matrix (res_size x in_size) */
    sched->W_input = ggml_new_tensor_2d(sched->ggml_ctx, GGML_TYPE_F32,
                                        in_size, res_size);
    if (!sched->W_input) {
        kern_log("ERROR: Failed to allocate input weights");
        free(sched);
        return -1;
    }
    
    /* Output weight matrix (out_size x res_size) */
    sched->W_output = ggml_new_tensor_2d(sched->ggml_ctx, GGML_TYPE_F32,
                                         res_size, out_size);
    if (!sched->W_output) {
        kern_log("ERROR: Failed to allocate output weights");
        free(sched);
        return -1;
    }
    
    /* Reservoir state vector (res_size) */
    sched->state = ggml_new_tensor_1d(sched->ggml_ctx, GGML_TYPE_F32, res_size);
    if (!sched->state) {
        kern_log("ERROR: Failed to allocate reservoir state");
        free(sched);
        return -1;
    }
    
    /* Initialize reservoir weights */
    kern_log("Initializing ESN weights (spectral_radius=%.2f, sparsity=%.2f)",
             sched->config.spectral_radius, sched->config.sparsity);
    
    init_reservoir_weights((float *)ggml_tensor_data(sched->W_reservoir), res_size,
                          sched->config.spectral_radius,
                          sched->config.sparsity);
    
    init_input_weights((float *)ggml_tensor_data(sched->W_input), res_size, in_size);
    init_output_weights((float *)ggml_tensor_data(sched->W_output), out_size, res_size);
    
    /* Initialize state to zero */
    memset(ggml_tensor_data(sched->state), 0, res_size * sizeof(float));
    
    /* Initialize task queues */
    sched->ready_queue = NULL;
    sched->waiting_queue = NULL;
    sched->current = NULL;
    
    sched->tick_count = 0;
    sched->context_switches = 0;
    
    g_kernel->sched = sched;
    
    kern_log("DTESN scheduler initialized successfully");
    return 0;
}

/**
 * dtesn_sched_tick - Single scheduler tick with reservoir dynamics
 *
 * Returns: 0 on success, -1 on failure
 *
 * Performance target: ≤5µs per tick
 */
int dtesn_sched_tick(void)
{
    struct dtesn_scheduler *sched;
    uint64_t start_ns, end_ns, duration_ns;
    float input[64];  /* Input vector */
    float output[32]; /* Output vector */
    struct task *selected_task = NULL;
    size_t i;
    float max_priority = -1.0f;
    
    if (!g_kernel || !g_kernel->sched) {
        kern_log("ERROR: Scheduler not initialized");
        return -1;
    }
    
    sched = g_kernel->sched;
    start_ns = kern_get_time_ns();
    
    /* Prepare input vector from current system state */
    /* Input encoding:
     * - Task queue lengths
     * - Current attention values
     * - Time of day
     * - etc.
     */
    memset(input, 0, sizeof(input));
    
    /* Count tasks in ready queue */
    int ready_count = 0;
    struct task *t;
    for (t = sched->ready_queue; t; t = t->next) {
        ready_count++;
    }
    
    input[0] = (float)ready_count / (float)g_kernel->config.max_tasks;
    input[1] = (float)sched->tick_count / 1000.0f;  /* Normalized tick count */
    
    /* Add current task attention if exists */
    if (sched->current) {
        input[2] = (float)sched->current->sti / 1000.0f;
        input[3] = (float)sched->current->lti / 1000.0f;
    }
    
    /* Update reservoir state */
    reservoir_update((float *)ggml_tensor_data(sched->state), input,
                    (float *)ggml_tensor_data(sched->W_reservoir),
                    (float *)ggml_tensor_data(sched->W_input),
                    sched->config.reservoir_size,
                    sched->config.input_dim,
                    0.3f);  /* Leak rate */
    
    /* Compute output (priority scores) */
    compute_output(output, (float *)ggml_tensor_data(sched->state),
                  (float *)ggml_tensor_data(sched->W_output),
                  sched->config.output_dim,
                  sched->config.reservoir_size);
    
    /* Select task with highest priority (reservoir output + attention) */
    i = 0;
    for (t = sched->ready_queue; t && i < sched->config.output_dim; t = t->next, i++) {
        float priority = output[i] + (float)t->sti / 1000.0f;
        if (priority > max_priority) {
            max_priority = priority;
            selected_task = t;
        }
    }
    
    /* Context switch if needed */
    if (selected_task && selected_task != sched->current) {
        sched->current = selected_task;
        sched->context_switches++;
    }
    
    /* Update statistics */
    sched->tick_count++;
    g_kernel->stats.total_ticks++;
    
    end_ns = kern_get_time_ns();
    duration_ns = end_ns - start_ns;
    
    /* Update tick timing stats */
    if (duration_ns > g_kernel->stats.max_tick_ns)
        g_kernel->stats.max_tick_ns = duration_ns;
    
    if (g_kernel->stats.total_ticks > 1) {
        g_kernel->stats.avg_tick_ns =
            (g_kernel->stats.avg_tick_ns * (g_kernel->stats.total_ticks - 1) +
             duration_ns) / g_kernel->stats.total_ticks;
    } else {
        g_kernel->stats.avg_tick_ns = duration_ns;
    }
    
    return 0;
}

/**
 * dtesn_sched_enqueue - Enqueue task with attention-based priority
 * @task: Task to enqueue
 *
 * Returns: 0 on success, -1 on failure
 */
int dtesn_sched_enqueue(struct task *task)
{
    struct dtesn_scheduler *sched;
    
    if (!g_kernel || !g_kernel->sched || !task) {
        return -1;
    }
    
    sched = g_kernel->sched;
    
    /* Set task state */
    task->state = TASK_READY;
    
    /* Add to ready queue (simple FIFO for now) */
    task->next = sched->ready_queue;
    sched->ready_queue = task;
    
    g_kernel->stats.active_tasks++;
    if (g_kernel->stats.active_tasks > g_kernel->stats.peak_tasks)
        g_kernel->stats.peak_tasks = g_kernel->stats.active_tasks;
    
    return 0;
}

/**
 * dtesn_mem_init_regions - Initialize P-system membrane regions
 * @max_depth: Maximum membrane depth
 *
 * Returns: 0 on success, -1 on failure
 */
int dtesn_mem_init_regions(uint32_t max_depth)
{
    kern_log("Initializing P-system membrane regions (depth=%u)", max_depth);
    
    if (!g_kernel) {
        kern_log("ERROR: Kernel not initialized");
        return -1;
    }
    
    /* TODO: Implement full P-system membrane hierarchy
     * For now, just validate and store config
     */
    
    if (max_depth > g_kernel->config.max_membrane_depth) {
        kern_log("WARNING: Requested depth %u exceeds max %u",
                 max_depth, g_kernel->config.max_membrane_depth);
        max_depth = g_kernel->config.max_membrane_depth;
    }
    
    kern_log("P-system membrane regions initialized");
    return 0;
}
