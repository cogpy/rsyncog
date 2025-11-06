/*
 * PLN (Probabilistic Logic Networks) Inference Engine for rsync
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Implements probabilistic inference for reasoning about sync patterns,
 * predicting sync success, and optimizing sync strategies.
 */

#ifndef PLN_INFERENCE_H
#define PLN_INFERENCE_H

#include "atomspace.h"
#include <stdint.h>

/* Forward declarations */
struct pln_rule;
struct pln_inference_context;

/**
 * PLN Inference Rule Types
 */
typedef enum {
    PLN_RULE_DEDUCTION,           /* A->B, B->C implies A->C */
    PLN_RULE_INDUCTION,           /* Multiple A->B observations strengthen rule */
    PLN_RULE_ABDUCTION,           /* B observed, A->B known, infer A */
    PLN_RULE_REVISION,            /* Merge conflicting truth values */
    PLN_RULE_SIMILARITY,          /* Infer similarity from common properties */
    PLN_RULE_INHERITANCE,         /* Inheritance relationship inference */
    PLN_RULE_IMPLICATION          /* Temporal/causal implication */
} pln_rule_type;

/**
 * PLN Inference Context - Manages inference state
 */
struct pln_inference_context {
    struct atom_space *atomspace;
    
    /* Inference statistics */
    uint64_t inferences_performed;
    uint64_t rules_applied;
    uint64_t predictions_made;
    
    /* Inference parameters */
    float confidence_threshold;    /* Minimum confidence for inference */
    float strength_threshold;      /* Minimum strength for inference */
    int max_inference_depth;       /* Maximum chain depth */
    
    /* Historical data for learning */
    void *sync_history;
    size_t history_size;
};

/**
 * Sync Pattern - Represents observed sync behavior
 */
struct sync_pattern {
    char *module_name;
    char *host_name;
    
    /* Historical metrics */
    uint64_t total_syncs;
    uint64_t successful_syncs;
    uint64_t failed_syncs;
    uint64_t total_bytes;
    
    /* Time-based patterns */
    time_t last_sync;
    time_t avg_duration;
    float success_rate;
    
    /* Predicted values */
    struct truth_value predicted_success;
    float predicted_duration;
};

/**
 * PLN Inference Functions
 */

/**
 * pln_context_create - Create PLN inference context
 * @atomspace: AtomSpace for inference
 *
 * Returns: New context or NULL on failure
 */
struct pln_inference_context *pln_context_create(struct atom_space *atomspace);

/**
 * pln_context_destroy - Free PLN context resources
 * @ctx: Context to destroy
 */
void pln_context_destroy(struct pln_inference_context *ctx);

/**
 * pln_apply_deduction - Apply deduction rule (A->B, B->C => A->C)
 * @ctx: Inference context
 * @link_ab: Link from A to B
 * @link_bc: Link from B to C
 *
 * Returns: Inferred link A->C or NULL
 */
struct atom_link *pln_apply_deduction(struct pln_inference_context *ctx,
                                      struct atom_link *link_ab,
                                      struct atom_link *link_bc);

/**
 * pln_apply_revision - Merge conflicting truth values
 * @ctx: Inference context
 * @tv1: First truth value
 * @tv2: Second truth value
 *
 * Returns: Revised truth value
 */
struct truth_value pln_apply_revision(struct pln_inference_context *ctx,
                                     struct truth_value tv1,
                                     struct truth_value tv2);

/**
 * pln_predict_sync_success - Predict sync success probability
 * @ctx: Inference context
 * @module: Module atom
 * @host: Host atom
 *
 * Returns: Predicted truth value for sync success
 */
struct truth_value pln_predict_sync_success(struct pln_inference_context *ctx,
                                           struct atom *module,
                                           struct atom *host);

/**
 * pln_infer_sync_patterns - Discover sync patterns from history
 * @ctx: Inference context
 * @patterns: Output array of discovered patterns
 * @max_patterns: Maximum patterns to return
 *
 * Returns: Number of patterns discovered
 */
int pln_infer_sync_patterns(struct pln_inference_context *ctx,
                           struct sync_pattern **patterns,
                           size_t max_patterns);

/**
 * pln_update_from_observation - Update truth values from sync observation
 * @ctx: Inference context
 * @module: Module that was synced
 * @success: Whether sync succeeded
 * @duration: Sync duration in seconds
 * @bytes: Bytes transferred
 *
 * Returns: 0 on success, -1 on failure
 */
int pln_update_from_observation(struct pln_inference_context *ctx,
                               struct atom *module,
                               int success,
                               time_t duration,
                               uint64_t bytes);

/**
 * pln_compute_similarity - Compute similarity between atoms
 * @ctx: Inference context
 * @atom1: First atom
 * @atom2: Second atom
 *
 * Returns: Similarity truth value
 */
struct truth_value pln_compute_similarity(struct pln_inference_context *ctx,
                                         struct atom *atom1,
                                         struct atom *atom2);

/**
 * pln_infer_optimal_schedule - Infer optimal sync schedule
 * @ctx: Inference context
 * @module: Module to schedule
 *
 * Returns: Recommended time interval in seconds
 */
time_t pln_infer_optimal_schedule(struct pln_inference_context *ctx,
                                 struct atom *module);

#endif /* PLN_INFERENCE_H */
