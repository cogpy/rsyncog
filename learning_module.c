/*
 * Learning Module Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "learning_module.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEFAULT_MAX_HISTORY 10000
#define DEFAULT_LEARNING_RATE 0.1
#define DEFAULT_DECAY_FACTOR 0.95

/**
 * learning_context_create - Create learning context
 */
struct learning_context *learning_context_create(struct atom_space *atomspace,
                                                struct pln_inference_context *pln_ctx)
{
    struct learning_context *ctx;
    
    if (!atomspace)
        return NULL;
    
    ctx = malloc(sizeof(struct learning_context));
    if (!ctx)
        return NULL;
    
    memset(ctx, 0, sizeof(struct learning_context));
    
    ctx->atomspace = atomspace;
    ctx->pln_ctx = pln_ctx;
    ctx->max_history_size = DEFAULT_MAX_HISTORY;
    ctx->learning_rate = DEFAULT_LEARNING_RATE;
    ctx->decay_factor = DEFAULT_DECAY_FACTOR;
    ctx->enable_temporal_patterns = 1;
    
    return ctx;
}

/**
 * learning_context_destroy - Free learning context
 */
void learning_context_destroy(struct learning_context *ctx)
{
    struct sync_history_entry *entry, *next;
    
    if (!ctx)
        return;
    
    /* Free history entries */
    entry = ctx->history_head;
    while (entry) {
        next = entry->next;
        if (entry->module_name)
            free(entry->module_name);
        if (entry->host_name)
            free(entry->host_name);
        free(entry);
        entry = next;
    }
    
    free(ctx);
}

/**
 * learning_record_sync - Record sync event
 */
int learning_record_sync(struct learning_context *ctx,
                        const char *module_name,
                        const char *host_name,
                        int success,
                        uint64_t bytes,
                        time_t duration)
{
    struct sync_history_entry *entry;
    struct atom *module_atom;
    time_t now;
    struct tm *tm_info;
    
    if (!ctx || !module_name)
        return -1;
    
    /* Create history entry */
    entry = malloc(sizeof(struct sync_history_entry));
    if (!entry)
        return -1;
    
    memset(entry, 0, sizeof(struct sync_history_entry));
    
    entry->entry_id = ctx->history_count++;
    entry->module_name = strdup(module_name);
    if (host_name)
        entry->host_name = strdup(host_name);
    
    now = time(NULL);
    entry->timestamp = now;
    entry->success = success;
    entry->bytes_transferred = bytes;
    entry->duration = duration;
    
    /* Extract temporal context */
    tm_info = localtime(&now);
    if (tm_info) {
        entry->hour_of_day = tm_info->tm_hour;
        entry->day_of_week = tm_info->tm_wday;
    }
    
    /* Add to history list */
    if (ctx->history_tail) {
        ctx->history_tail->next = entry;
        ctx->history_tail = entry;
    } else {
        ctx->history_head = entry;
        ctx->history_tail = entry;
    }
    
    /* Enforce max history size (FIFO) */
    while (ctx->history_count > ctx->max_history_size && ctx->history_head) {
        struct sync_history_entry *old = ctx->history_head;
        ctx->history_head = old->next;
        if (old->module_name)
            free(old->module_name);
        if (old->host_name)
            free(old->host_name);
        free(old);
        ctx->history_count--;
    }
    
    /* Update atom with observation */
    module_atom = atomspace_find_node(ctx->atomspace, ATOM_MODULE, module_name);
    if (module_atom && ctx->pln_ctx) {
        pln_update_from_observation(ctx->pln_ctx, module_atom,
                                   success, duration, bytes);
        ctx->stats.truth_value_updates++;
    }
    
    ctx->stats.total_observations++;
    
    return 0;
}

/**
 * learning_get_success_rate - Calculate historical success rate
 */
float learning_get_success_rate(struct learning_context *ctx,
                               const char *module_name)
{
    struct sync_history_entry *entry;
    uint64_t total = 0;
    uint64_t successes = 0;
    
    if (!ctx || !module_name)
        return -1.0;
    
    /* Count successes and total syncs for this module */
    for (entry = ctx->history_head; entry; entry = entry->next) {
        if (strcmp(entry->module_name, module_name) == 0) {
            total++;
            if (entry->success)
                successes++;
        }
    }
    
    if (total == 0)
        return -1.0;
    
    return (float)successes / (float)total;
}

/**
 * learning_update_truth_values - Update all truth values from history
 */
int learning_update_truth_values(struct learning_context *ctx)
{
    struct atom *atom;
    size_t i;
    int updated = 0;
    float success_rate;
    
    if (!ctx)
        return 0;
    
    /* Iterate through all module atoms */
    for (i = 0; i < ctx->atomspace->atom_table_size; i++) {
        for (atom = ctx->atomspace->atom_table[i]; atom; atom = atom->hash_next) {
            if (atom->type != ATOM_MODULE)
                continue;
            
            /* Get historical success rate */
            success_rate = learning_get_success_rate(ctx, atom->name);
            if (success_rate >= 0.0) {
                /* Update truth value using learning rate */
                float new_strength = atom->tv.strength * (1.0 - ctx->learning_rate) +
                                   success_rate * ctx->learning_rate;
                
                /* Increase confidence as we learn more */
                float new_confidence = fmin(0.99, 
                    atom->tv.confidence + ctx->learning_rate * 0.1);
                
                atom->tv.strength = new_strength;
                atom->tv.confidence = new_confidence;
                
                updated++;
            }
        }
    }
    
    return updated;
}

/**
 * learning_predict_success - Predict success using learned patterns
 */
struct truth_value learning_predict_success(struct learning_context *ctx,
                                           const char *module_name,
                                           const char *host_name)
{
    struct atom *module_atom;
    struct truth_value prediction;
    float historical_rate;
    
    if (!ctx || !module_name)
        return (struct truth_value){0.5, 0.0};
    
    /* Get module atom */
    module_atom = atomspace_find_node(ctx->atomspace, ATOM_MODULE, module_name);
    if (!module_atom)
        return (struct truth_value){0.5, 0.0};
    
    /* Use PLN for base prediction */
    if (ctx->pln_ctx) {
        prediction = pln_predict_sync_success(ctx->pln_ctx, module_atom, NULL);
    } else {
        prediction = module_atom->tv;
    }
    
    /* Refine with historical success rate */
    historical_rate = learning_get_success_rate(ctx, module_name);
    if (historical_rate >= 0.0) {
        /* Blend PLN prediction with historical rate */
        prediction.strength = prediction.strength * 0.5 + historical_rate * 0.5;
        prediction.confidence = fmin(0.99, prediction.confidence + 0.1);
    }
    
    return prediction;
}

/**
 * learning_discover_temporal_patterns - Find time-based patterns
 */
int learning_discover_temporal_patterns(struct learning_context *ctx,
                                       struct temporal_pattern **patterns,
                                       size_t max_patterns)
{
    struct sync_history_entry *entry;
    struct temporal_pattern *pattern;
    int pattern_count = 0;
    
    /* Simple implementation: group by hour of day */
    int hour_stats[24][2]; /* [hour][successes, total] */
    size_t i;
    
    if (!ctx || !patterns || max_patterns == 0 || !ctx->enable_temporal_patterns)
        return 0;
    
    memset(hour_stats, 0, sizeof(hour_stats));
    
    /* Collect statistics by hour */
    for (entry = ctx->history_head; entry; entry = entry->next) {
        if (entry->hour_of_day >= 0 && entry->hour_of_day < 24) {
            hour_stats[entry->hour_of_day][1]++; /* total */
            if (entry->success)
                hour_stats[entry->hour_of_day][0]++; /* successes */
        }
    }
    
    /* Create patterns for hours with significant data */
    for (i = 0; i < 24 && pattern_count < (int)max_patterns; i++) {
        if (hour_stats[i][1] >= 10) { /* At least 10 observations */
            pattern = malloc(sizeof(struct temporal_pattern));
            if (!pattern)
                continue;
            
            memset(pattern, 0, sizeof(struct temporal_pattern));
            
            pattern->module_name = strdup("all_modules");
            pattern->hour_of_day = i;
            pattern->day_of_week = -1; /* All days */
            pattern->observation_count = hour_stats[i][1];
            pattern->success_probability = 
                (float)hour_stats[i][0] / (float)hour_stats[i][1];
            
            patterns[pattern_count++] = pattern;
        }
    }
    
    ctx->stats.patterns_learned = pattern_count;
    
    return pattern_count;
}

/**
 * learning_adapt_attention - Adapt attention based on learning
 */
int learning_adapt_attention(struct learning_context *ctx)
{
    struct atom *atom;
    size_t i;
    int adapted = 0;
    float success_rate;
    
    if (!ctx)
        return 0;
    
    /* Adapt attention values based on performance */
    for (i = 0; i < ctx->atomspace->atom_table_size; i++) {
        for (atom = ctx->atomspace->atom_table[i]; atom; atom = atom->hash_next) {
            if (atom->type != ATOM_MODULE)
                continue;
            
            success_rate = learning_get_success_rate(ctx, atom->name);
            if (success_rate >= 0.0) {
                /* High success rate increases LTI (long-term reliability) */
                if (success_rate > 0.8) {
                    atom->av.lti = (int16_t)fmin(1000, atom->av.lti + 5);
                } else if (success_rate < 0.5) {
                    /* Low success rate needs attention (high STI) */
                    atom->av.sti = (int16_t)fmin(1000, atom->av.sti + 10);
                }
                adapted++;
            }
        }
    }
    
    return adapted;
}

/**
 * learning_get_statistics - Return learning statistics
 */
struct learning_stats learning_get_statistics(struct learning_context *ctx)
{
    struct learning_stats stats = {0};
    
    if (ctx)
        stats = ctx->stats;
    
    /* Calculate prediction accuracy if we have data */
    if (ctx && ctx->stats.predictions_correct + ctx->stats.predictions_incorrect > 0) {
        stats.prediction_accuracy = 
            (float)ctx->stats.predictions_correct / 
            (float)(ctx->stats.predictions_correct + ctx->stats.predictions_incorrect);
    }
    
    return stats;
}

/**
 * learning_save_history - Persist history to file (stub implementation)
 */
int learning_save_history(struct learning_context *ctx, const char *filename)
{
    /* TODO: Implement binary or JSON serialization */
    if (!ctx || !filename)
        return -1;
    
    return 0;
}

/**
 * learning_load_history - Load history from file (stub implementation)
 */
int learning_load_history(struct learning_context *ctx, const char *filename)
{
    /* TODO: Implement binary or JSON deserialization */
    if (!ctx || !filename)
        return -1;
    
    return 0;
}
