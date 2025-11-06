/*
 * llama.cpp Integration - Neural Pattern Recognition Backend
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Integrates llama.cpp for neural network-based pattern recognition
 * on sync patterns and AtomSpace structures.
 */

#ifndef LLAMA_PATTERN_H
#define LLAMA_PATTERN_H

#include "atomspace.h"
#include "learning_module.h"
#include <stdint.h>

/* Forward declarations */
struct llama_model;
struct llama_context;
struct llama_pattern_context;

/**
 * Pattern Recognition Task Types
 */
typedef enum {
    PATTERN_TASK_PREDICT_SUCCESS,     /* Predict sync success */
    PATTERN_TASK_CLASSIFY_FAILURE,    /* Classify failure type */
    PATTERN_TASK_GENERATE_SCHEDULE,   /* Generate optimal schedule */
    PATTERN_TASK_DETECT_ANOMALY       /* Detect anomalous patterns */
} pattern_task_type;

/**
 * llama.cpp Pattern Recognition Context
 */
struct llama_pattern_context {
    struct atom_space *atomspace;
    struct learning_context *learning_ctx;
    
    /* llama.cpp model */
    struct llama_model *model;
    struct llama_context *llama_ctx;
    
    /* Model configuration */
    char *model_path;
    int context_size;
    int n_threads;
    
    /* Pattern recognition stats */
    uint64_t predictions_made;
    uint64_t correct_predictions;
    float accuracy;
};

/**
 * Pattern Recognition Result
 */
struct pattern_result {
    pattern_task_type task;
    float confidence;
    
    union {
        float success_probability;
        int failure_class;
        time_t recommended_schedule;
        int is_anomaly;
    } result;
    
    char *explanation;
};

/**
 * llama.cpp Pattern Recognition Functions
 */

/**
 * llama_pattern_create - Create pattern recognition context
 * @atomspace: AtomSpace for analysis
 * @learning_ctx: Learning context for historical data
 * @model_path: Path to llama model file
 *
 * Returns: New context or NULL on failure
 */
struct llama_pattern_context *llama_pattern_create(
    struct atom_space *atomspace,
    struct learning_context *learning_ctx,
    const char *model_path);

/**
 * llama_pattern_destroy - Free pattern recognition context
 * @ctx: Context to destroy
 */
void llama_pattern_destroy(struct llama_pattern_context *ctx);

/**
 * llama_pattern_predict_success - Predict sync success using neural network
 * @ctx: Pattern context
 * @module_name: Module to predict for
 * @host_name: Target host
 *
 * Returns: Prediction result
 */
struct pattern_result llama_pattern_predict_success(
    struct llama_pattern_context *ctx,
    const char *module_name,
    const char *host_name);

/**
 * llama_pattern_classify_failure - Classify type of sync failure
 * @ctx: Pattern context
 * @module_name: Module that failed
 * @error_msg: Error message
 *
 * Returns: Failure classification
 */
struct pattern_result llama_pattern_classify_failure(
    struct llama_pattern_context *ctx,
    const char *module_name,
    const char *error_msg);

/**
 * llama_pattern_generate_schedule - Generate optimal sync schedule
 * @ctx: Pattern context
 * @module_name: Module to schedule
 *
 * Returns: Schedule recommendation
 */
struct pattern_result llama_pattern_generate_schedule(
    struct llama_pattern_context *ctx,
    const char *module_name);

/**
 * llama_pattern_detect_anomaly - Detect anomalous sync patterns
 * @ctx: Pattern context
 * @module_name: Module to check
 *
 * Returns: Anomaly detection result
 */
struct pattern_result llama_pattern_detect_anomaly(
    struct llama_pattern_context *ctx,
    const char *module_name);

/**
 * llama_pattern_train - Train model on historical data
 * @ctx: Pattern context
 * @epochs: Number of training epochs
 *
 * Returns: 0 on success, -1 on failure
 */
int llama_pattern_train(struct llama_pattern_context *ctx, int epochs);

/**
 * llama_pattern_get_accuracy - Get model prediction accuracy
 * @ctx: Pattern context
 *
 * Returns: Accuracy [0.0-1.0]
 */
float llama_pattern_get_accuracy(struct llama_pattern_context *ctx);

#endif /* LLAMA_PATTERN_H */
