/*
 * Learning Module - Historical Sync Success Rate Tracking
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Implements experience-based learning by tracking sync success rates,
 * identifying patterns, and adapting behavior based on historical performance.
 */

#ifndef LEARNING_MODULE_H
#define LEARNING_MODULE_H

#include "atomspace.h"
#include "pln_inference.h"
#include <stdint.h>
#include <time.h>

/* Forward declarations */
struct learning_context;
struct sync_history_entry;

/**
 * Sync History Entry - Records individual sync event
 */
struct sync_history_entry {
    uint64_t entry_id;
    char *module_name;
    char *host_name;
    time_t timestamp;
    
    /* Sync metrics */
    int success;
    uint64_t bytes_transferred;
    time_t duration;
    int error_code;
    
    /* Context */
    int hour_of_day;       /* 0-23 */
    int day_of_week;       /* 0-6, 0=Sunday */
    
    struct sync_history_entry *next;
};

/**
 * Learning Statistics - Aggregate learning metrics
 */
struct learning_stats {
    uint64_t total_observations;
    uint64_t truth_value_updates;
    uint64_t patterns_learned;
    uint64_t predictions_correct;
    uint64_t predictions_incorrect;
    float prediction_accuracy;
};

/**
 * Learning Context - Manages learning state and history
 */
struct learning_context {
    struct atom_space *atomspace;
    struct pln_inference_context *pln_ctx;
    
    /* History storage */
    struct sync_history_entry *history_head;
    struct sync_history_entry *history_tail;
    uint64_t history_count;
    uint64_t max_history_size;
    
    /* Learning parameters */
    float learning_rate;           /* How fast to adapt (0.0-1.0) */
    float decay_factor;            /* Historical decay over time */
    int enable_temporal_patterns;  /* Learn time-based patterns */
    
    /* Statistics */
    struct learning_stats stats;
};

/**
 * Temporal Pattern - Time-based sync pattern
 */
struct temporal_pattern {
    char *module_name;
    int hour_of_day;
    int day_of_week;
    float success_probability;
    uint32_t observation_count;
};

/**
 * Learning Module Functions
 */

/**
 * learning_context_create - Create learning context
 * @atomspace: AtomSpace for learning
 * @pln_ctx: PLN context for inference
 *
 * Returns: New learning context or NULL on failure
 */
struct learning_context *learning_context_create(struct atom_space *atomspace,
                                                struct pln_inference_context *pln_ctx);

/**
 * learning_context_destroy - Free learning context
 * @ctx: Context to destroy
 */
void learning_context_destroy(struct learning_context *ctx);

/**
 * learning_record_sync - Record sync event for learning
 * @ctx: Learning context
 * @module_name: Module that was synced
 * @host_name: Host involved in sync
 * @success: Whether sync succeeded
 * @bytes: Bytes transferred
 * @duration: Sync duration
 *
 * Returns: 0 on success, -1 on failure
 */
int learning_record_sync(struct learning_context *ctx,
                        const char *module_name,
                        const char *host_name,
                        int success,
                        uint64_t bytes,
                        time_t duration);

/**
 * learning_update_truth_values - Update all atom truth values from history
 * @ctx: Learning context
 *
 * Returns: Number of atoms updated
 */
int learning_update_truth_values(struct learning_context *ctx);

/**
 * learning_get_success_rate - Get historical success rate for module
 * @ctx: Learning context
 * @module_name: Module to query
 *
 * Returns: Success rate [0.0-1.0] or -1.0 if no data
 */
float learning_get_success_rate(struct learning_context *ctx,
                               const char *module_name);

/**
 * learning_predict_success - Predict sync success with learned knowledge
 * @ctx: Learning context
 * @module_name: Module to predict for
 * @host_name: Target host
 *
 * Returns: Predicted truth value
 */
struct truth_value learning_predict_success(struct learning_context *ctx,
                                           const char *module_name,
                                           const char *host_name);

/**
 * learning_discover_temporal_patterns - Find time-based patterns
 * @ctx: Learning context
 * @patterns: Output array of patterns
 * @max_patterns: Maximum patterns to return
 *
 * Returns: Number of patterns discovered
 */
int learning_discover_temporal_patterns(struct learning_context *ctx,
                                       struct temporal_pattern **patterns,
                                       size_t max_patterns);

/**
 * learning_adapt_attention - Adapt attention values based on performance
 * @ctx: Learning context
 *
 * Returns: Number of atoms adapted
 */
int learning_adapt_attention(struct learning_context *ctx);

/**
 * learning_get_statistics - Get learning statistics
 * @ctx: Learning context
 *
 * Returns: Learning statistics structure
 */
struct learning_stats learning_get_statistics(struct learning_context *ctx);

/**
 * learning_save_history - Persist learning history to file
 * @ctx: Learning context
 * @filename: Path to save file
 *
 * Returns: 0 on success, -1 on failure
 */
int learning_save_history(struct learning_context *ctx, const char *filename);

/**
 * learning_load_history - Load learning history from file
 * @ctx: Learning context
 * @filename: Path to load file
 *
 * Returns: Number of entries loaded or -1 on failure
 */
int learning_load_history(struct learning_context *ctx, const char *filename);

#endif /* LEARNING_MODULE_H */
