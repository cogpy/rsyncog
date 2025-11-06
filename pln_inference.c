/*
 * PLN (Probabilistic Logic Networks) Inference Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "pln_inference.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEFAULT_CONFIDENCE_THRESHOLD 0.1
#define DEFAULT_STRENGTH_THRESHOLD 0.1
#define DEFAULT_MAX_INFERENCE_DEPTH 5

/**
 * pln_context_create - Create PLN inference context
 */
struct pln_inference_context *pln_context_create(struct atom_space *atomspace)
{
    struct pln_inference_context *ctx;
    
    if (!atomspace)
        return NULL;
    
    ctx = malloc(sizeof(struct pln_inference_context));
    if (!ctx)
        return NULL;
    
    memset(ctx, 0, sizeof(struct pln_inference_context));
    
    ctx->atomspace = atomspace;
    ctx->confidence_threshold = DEFAULT_CONFIDENCE_THRESHOLD;
    ctx->strength_threshold = DEFAULT_STRENGTH_THRESHOLD;
    ctx->max_inference_depth = DEFAULT_MAX_INFERENCE_DEPTH;
    
    return ctx;
}

/**
 * pln_context_destroy - Free PLN context
 */
void pln_context_destroy(struct pln_inference_context *ctx)
{
    if (!ctx)
        return;
    
    if (ctx->sync_history)
        free(ctx->sync_history);
    
    free(ctx);
}

/**
 * Helper: Calculate confidence from count (count-to-confidence formula)
 */
static float count_to_confidence(uint64_t count)
{
    /* Simple formula: confidence = count / (count + k) where k=10 */
    return (float)count / (count + 10.0);
}

/**
 * pln_apply_deduction - Apply deduction rule
 *
 * Deduction formula:
 * If A->B with TV(sAB, cAB) and B->C with TV(sBC, cBC)
 * Then A->C with TV(sAB * sBC, cAB * cBC * sBC)
 */
struct atom_link *pln_apply_deduction(struct pln_inference_context *ctx,
                                      struct atom_link *link_ab,
                                      struct atom_link *link_bc)
{
    struct atom_link *link_ac;
    struct atom *outgoing[2];
    struct truth_value tv;
    
    if (!ctx || !link_ab || !link_bc)
        return NULL;
    
    /* Verify that B is common to both links */
    if (link_ab->outgoing_size < 2 || link_bc->outgoing_size < 2)
        return NULL;
    
    if (link_ab->outgoing[1] != link_bc->outgoing[0])
        return NULL;
    
    /* Create A->C link */
    outgoing[0] = link_ab->outgoing[0];
    outgoing[1] = link_bc->outgoing[1];
    
    /* Calculate deduction truth value */
    tv.strength = link_ab->tv.strength * link_bc->tv.strength;
    tv.confidence = link_ab->tv.confidence * link_bc->tv.confidence * 
                   link_bc->tv.strength;
    
    /* Only create link if confidence is above threshold */
    if (tv.confidence < ctx->confidence_threshold)
        return NULL;
    
    link_ac = atomspace_add_link(ctx->atomspace, link_ab->type,
                                outgoing, 2);
    if (link_ac) {
        link_ac->tv = tv;
        ctx->inferences_performed++;
        ctx->rules_applied++;
    }
    
    return link_ac;
}

/**
 * pln_apply_revision - Merge conflicting truth values
 *
 * Revision formula combines two independent observations:
 * s = (s1*c1 + s2*c2) / (c1 + c2)
 * c = (c1 + c2) / (1 + c1*c2)
 */
struct truth_value pln_apply_revision(struct pln_inference_context *ctx,
                                     struct truth_value tv1,
                                     struct truth_value tv2)
{
    struct truth_value result;
    float c_sum;
    
    if (!ctx)
        return (struct truth_value){0.0, 0.0};
    
    c_sum = tv1.confidence + tv2.confidence;
    
    if (c_sum > 0.0) {
        result.strength = (tv1.strength * tv1.confidence + 
                          tv2.strength * tv2.confidence) / c_sum;
        result.confidence = c_sum / (1.0 + tv1.confidence * tv2.confidence);
    } else {
        result.strength = 0.5;
        result.confidence = 0.0;
    }
    
    return result;
}

/**
 * pln_predict_sync_success - Predict sync success using historical data
 */
struct truth_value pln_predict_sync_success(struct pln_inference_context *ctx,
                                           struct atom *module,
                                           struct atom *host)
{
    struct truth_value prediction;
    
    if (!ctx || !module)
        return (struct truth_value){0.5, 0.0};
    
    /* Use current truth value as baseline prediction */
    prediction = module->tv;
    
    /* Factor in access patterns */
    if (module->access_count > 0) {
        float access_factor = count_to_confidence(module->access_count);
        prediction.confidence *= access_factor;
    }
    
    /* Factor in attention value (STI indicates recent importance) */
    if (module->av.sti > 0) {
        prediction.strength = fmin(1.0, prediction.strength * 
                                  (1.0 + module->av.sti / 100.0));
    }
    
    ctx->predictions_made++;
    
    return prediction;
}

/**
 * pln_update_from_observation - Update beliefs from sync observation
 */
int pln_update_from_observation(struct pln_inference_context *ctx,
                               struct atom *module,
                               int success,
                               time_t duration,
                               uint64_t bytes)
{
    struct truth_value observation, revised;
    
    if (!ctx || !module)
        return -1;
    
    /* Create observation truth value */
    observation.strength = success ? 1.0 : 0.0;
    observation.confidence = 0.9; /* High confidence in direct observation */
    
    /* Revise existing belief with new observation */
    revised = pln_apply_revision(ctx, module->tv, observation);
    
    /* Update module truth value */
    module->tv = revised;
    
    /* Update attention based on observation */
    if (success) {
        /* Successful sync increases both STI and LTI */
        module->av.sti = (int16_t)fmin(1000, module->av.sti + 5);
        module->av.lti = (int16_t)fmin(1000, module->av.lti + 1);
    } else {
        /* Failed sync decreases STI but can increase LTI (needs attention) */
        module->av.sti = (int16_t)fmax(-1000, module->av.sti - 10);
        module->av.lti = (int16_t)fmin(1000, module->av.lti + 2);
    }
    
    /* Update access count */
    module->access_count++;
    module->last_accessed = time(NULL);
    
    return 0;
}

/**
 * pln_compute_similarity - Compute similarity between atoms
 *
 * Uses Jaccard similarity of properties and connections
 */
struct truth_value pln_compute_similarity(struct pln_inference_context *ctx,
                                         struct atom *atom1,
                                         struct atom *atom2)
{
    struct truth_value similarity;
    float type_match, tv_similarity;
    
    if (!ctx || !atom1 || !atom2)
        return (struct truth_value){0.0, 0.0};
    
    /* Same atom has perfect similarity */
    if (atom1 == atom2)
        return (struct truth_value){1.0, 1.0};
    
    /* Type similarity */
    type_match = (atom1->type == atom2->type) ? 1.0 : 0.0;
    
    /* Truth value similarity (how close are the strengths) */
    tv_similarity = 1.0 - fabs(atom1->tv.strength - atom2->tv.strength);
    
    /* Combined similarity */
    similarity.strength = (type_match + tv_similarity) / 2.0;
    
    /* Confidence based on how much we know about each atom */
    similarity.confidence = (atom1->tv.confidence + atom2->tv.confidence) / 2.0;
    
    return similarity;
}

/**
 * pln_infer_optimal_schedule - Infer optimal sync schedule
 *
 * Uses historical access patterns and importance to determine schedule
 */
time_t pln_infer_optimal_schedule(struct pln_inference_context *ctx,
                                 struct atom *module)
{
    time_t interval;
    float importance_factor;
    
    if (!ctx || !module)
        return 3600; /* Default: 1 hour */
    
    /* Base interval on importance */
    importance_factor = (module->av.sti + 100.0) / 200.0; /* Normalize to [0,1] */
    
    /* More important modules sync more frequently */
    if (importance_factor > 0.8)
        interval = 300;      /* 5 minutes for high importance */
    else if (importance_factor > 0.5)
        interval = 900;      /* 15 minutes for medium importance */
    else if (importance_factor > 0.2)
        interval = 3600;     /* 1 hour for low importance */
    else
        interval = 14400;    /* 4 hours for very low importance */
    
    /* Adjust based on reliability (truth value strength) */
    if (module->tv.strength < 0.5) {
        /* Unreliable modules need more frequent checking */
        interval = interval / 2;
    }
    
    return interval;
}

/**
 * pln_infer_sync_patterns - Discover patterns from AtomSpace
 */
int pln_infer_sync_patterns(struct pln_inference_context *ctx,
                           struct sync_pattern **patterns,
                           size_t max_patterns)
{
    struct atom *atom;
    struct sync_pattern *pattern;
    size_t pattern_count = 0;
    size_t i;
    
    if (!ctx || !patterns || max_patterns == 0)
        return 0;
    
    /* Iterate through AtomSpace looking for module atoms */
    for (i = 0; i < ctx->atomspace->atom_table_size && pattern_count < max_patterns; i++) {
        for (atom = ctx->atomspace->atom_table[i]; atom && pattern_count < max_patterns; 
             atom = atom->hash_next) {
            
            if (atom->type != ATOM_MODULE)
                continue;
            
            /* Create pattern for this module */
            pattern = malloc(sizeof(struct sync_pattern));
            if (!pattern)
                continue;
            
            memset(pattern, 0, sizeof(struct sync_pattern));
            
            pattern->module_name = strdup(atom->name);
            pattern->total_syncs = atom->access_count;
            
            /* Estimate success rate from truth value */
            pattern->success_rate = atom->tv.strength;
            pattern->successful_syncs = (uint64_t)(atom->access_count * atom->tv.strength);
            pattern->failed_syncs = atom->access_count - pattern->successful_syncs;
            
            pattern->last_sync = atom->last_accessed;
            
            /* Predict future success */
            pattern->predicted_success = pln_predict_sync_success(ctx, atom, NULL);
            
            patterns[pattern_count++] = pattern;
        }
    }
    
    return pattern_count;
}
