/*
 * Dynamic Reconfiguration Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "dynamic_reconfig.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PERFORMANCE_THRESHOLD 0.7
#define DEFAULT_MIN_RECONFIG_INTERVAL 3600  /* 1 hour */
#define DEFAULT_MAX_EVENT_HISTORY 100

/**
 * reconfig_context_create - Create reconfiguration context
 */
struct reconfig_context *reconfig_context_create(struct cog_agent *agent_zero,
                                                 struct atom_space *atomspace,
                                                 struct learning_context *learning_ctx)
{
    struct reconfig_context *ctx;
    
    if (!agent_zero || !atomspace)
        return NULL;
    
    ctx = malloc(sizeof(struct reconfig_context));
    if (!ctx)
        return NULL;
    
    memset(ctx, 0, sizeof(struct reconfig_context));
    
    ctx->agent_zero = agent_zero;
    ctx->atomspace = atomspace;
    ctx->learning_ctx = learning_ctx;
    
    ctx->auto_reconfig_enabled = 0;
    ctx->performance_threshold = DEFAULT_PERFORMANCE_THRESHOLD;
    ctx->min_reconfig_interval = DEFAULT_MIN_RECONFIG_INTERVAL;
    ctx->max_event_history = DEFAULT_MAX_EVENT_HISTORY;
    
    ctx->last_reconfig = time(NULL);
    
    return ctx;
}

/**
 * reconfig_context_destroy - Free reconfiguration context
 */
void reconfig_context_destroy(struct reconfig_context *ctx)
{
    struct reconfig_event *event, *next;
    
    if (!ctx)
        return;
    
    /* Free event history */
    event = ctx->event_history;
    while (event) {
        next = event->next;
        if (event->reason)
            free(event->reason);
        free(event);
        event = next;
    }
    
    if (ctx->current_config_path)
        free(ctx->current_config_path);
    
    free(ctx);
}

/**
 * Helper: Add reconfiguration event to history
 */
static void reconfig_add_event(struct reconfig_context *ctx,
                              reconfig_trigger_type trigger,
                              const char *reason)
{
    struct reconfig_event *event;
    
    if (!ctx)
        return;
    
    event = malloc(sizeof(struct reconfig_event));
    if (!event)
        return;
    
    memset(event, 0, sizeof(struct reconfig_event));
    
    event->event_id = ctx->reconfig_count;
    event->timestamp = time(NULL);
    event->trigger = trigger;
    if (reason)
        event->reason = strdup(reason);
    
    event->atoms_before = ctx->atomspace->atom_count;
    
    /* Add to history */
    event->next = ctx->event_history;
    ctx->event_history = event;
}

/**
 * reconfig_enable_auto - Enable automatic reconfiguration
 */
int reconfig_enable_auto(struct reconfig_context *ctx,
                        int enabled,
                        float threshold,
                        time_t min_interval)
{
    if (!ctx)
        return -1;
    
    ctx->auto_reconfig_enabled = enabled;
    
    if (threshold >= 0.0 && threshold <= 1.0)
        ctx->performance_threshold = threshold;
    
    if (min_interval > 0)
        ctx->min_reconfig_interval = min_interval;
    
    return 0;
}

/**
 * reconfig_check_triggers - Check if reconfiguration should occur
 */
int reconfig_check_triggers(struct reconfig_context *ctx)
{
    time_t now;
    time_t time_since_reconfig;
    
    if (!ctx || !ctx->auto_reconfig_enabled)
        return -1;
    
    now = time(NULL);
    time_since_reconfig = now - ctx->last_reconfig;
    
    /* Check minimum interval */
    if (time_since_reconfig < ctx->min_reconfig_interval)
        return -1;
    
    /* Check performance threshold */
    if (ctx->learning_ctx) {
        /* Calculate average success rate across all modules */
        struct atom *atom;
        size_t i;
        float total_strength = 0.0;
        int module_count = 0;
        
        for (i = 0; i < ctx->atomspace->atom_table_size; i++) {
            for (atom = ctx->atomspace->atom_table[i]; atom; atom = atom->hash_next) {
                if (atom->type == ATOM_MODULE) {
                    total_strength += atom->tv.strength;
                    module_count++;
                }
            }
        }
        
        if (module_count > 0) {
            ctx->current_success_rate = total_strength / module_count;
            
            if (ctx->current_success_rate < ctx->performance_threshold)
                return RECONFIG_TRIGGER_PERFORMANCE;
        }
    }
    
    /* Check topology changes */
    if (ctx->syncs_since_reconfig > 100) {
        return RECONFIG_TRIGGER_SCHEDULED;
    }
    
    return -1;
}

/**
 * reconfig_trigger_manual - Manually trigger reconfiguration
 */
int reconfig_trigger_manual(struct reconfig_context *ctx, const char *reason)
{
    if (!ctx)
        return -1;
    
    reconfig_add_event(ctx, RECONFIG_TRIGGER_MANUAL, reason);
    
    return reconfig_generate(ctx, ctx->current_config_path);
}

/**
 * reconfig_generate - Generate new configuration
 */
int reconfig_generate(struct reconfig_context *ctx, const char *output_path)
{
    int result;
    
    if (!ctx || !ctx->agent_zero)
        return -1;
    
    /* Initialize Agent Zero with new path if provided */
    if (output_path && strcmp(output_path, "") != 0) {
        if (ctx->current_config_path)
            free(ctx->current_config_path);
        ctx->current_config_path = strdup(output_path);
        
        agent_zero_init(ctx->agent_zero, ctx->atomspace, output_path);
    }
    
    /* Optimize before generating */
    reconfig_optimize_topology(ctx);
    reconfig_adapt_modules(ctx);
    reconfig_update_priorities(ctx);
    
    /* Generate configuration */
    result = agent_zero_generate_config(ctx->agent_zero);
    
    if (result == 0) {
        ctx->last_reconfig = time(NULL);
        ctx->reconfig_count++;
        ctx->syncs_since_reconfig = 0;
        
        /* Update event with after-metrics */
        if (ctx->event_history) {
            ctx->event_history->atoms_after = ctx->atomspace->atom_count;
            ctx->event_history->avg_success_after = ctx->current_success_rate;
        }
    }
    
    return result;
}

/**
 * reconfig_optimize_topology - Optimize AtomSpace topology
 */
int reconfig_optimize_topology(struct reconfig_context *ctx)
{
    struct atom *atom;
    size_t i;
    int optimizations = 0;
    
    if (!ctx)
        return 0;
    
    /* Remove low-importance atoms that haven't been accessed recently */
    for (i = 0; i < ctx->atomspace->atom_table_size; i++) {
        for (atom = ctx->atomspace->atom_table[i]; atom; atom = atom->hash_next) {
            time_t age = time(NULL) - atom->last_accessed;
            
            /* Remove stale, unimportant atoms */
            if (atom->av.sti < -50 && atom->av.lti < 10 && age > 86400) {
                /* TODO: Implement atom removal safely */
                /* atomspace_remove_atom(ctx->atomspace, atom->handle); */
                optimizations++;
            }
        }
    }
    
    return optimizations;
}

/**
 * reconfig_adapt_modules - Adapt module configurations
 */
int reconfig_adapt_modules(struct reconfig_context *ctx)
{
    struct atom *atom;
    size_t i;
    int adapted = 0;
    
    if (!ctx || !ctx->learning_ctx)
        return 0;
    
    /* Update truth values from learning */
    adapted = learning_update_truth_values(ctx->learning_ctx);
    
    /* Adapt attention values */
    adapted += learning_adapt_attention(ctx->learning_ctx);
    
    /* Adjust module properties based on performance */
    for (i = 0; i < ctx->atomspace->atom_table_size; i++) {
        for (atom = ctx->atomspace->atom_table[i]; atom; atom = atom->hash_next) {
            if (atom->type != ATOM_MODULE)
                continue;
            
            /* Low-performing modules might need special handling */
            if (atom->tv.strength < 0.5) {
                /* Increase STI to give them more attention */
                atom->av.sti += 20;
            }
        }
    }
    
    return adapted;
}

/**
 * reconfig_update_priorities - Update priorities based on importance
 */
int reconfig_update_priorities(struct reconfig_context *ctx)
{
    struct atom *atom;
    size_t i;
    int updated = 0;
    
    if (!ctx)
        return 0;
    
    /* Update VLTI based on LTI (promote consistently important atoms) */
    for (i = 0; i < ctx->atomspace->atom_table_size; i++) {
        for (atom = ctx->atomspace->atom_table[i]; atom; atom = atom->hash_next) {
            if (atom->av.lti > 100) {
                atom->av.vlti = (uint16_t)(atom->av.lti / 10);
                updated++;
            }
        }
    }
    
    return updated;
}

/**
 * reconfig_get_events - Get reconfiguration event history
 */
int reconfig_get_events(struct reconfig_context *ctx,
                       struct reconfig_event **events,
                       size_t max_events)
{
    struct reconfig_event *event;
    int count = 0;
    
    if (!ctx || !events || max_events == 0)
        return 0;
    
    for (event = ctx->event_history; event && count < (int)max_events; 
         event = event->next) {
        events[count++] = event;
    }
    
    return count;
}
