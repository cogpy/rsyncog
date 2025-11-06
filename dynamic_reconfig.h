/*
 * Dynamic Reconfiguration - Agent Zero Adaptive Config Generation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Enables Agent Zero to regenerate rsyncd.conf based on changing conditions,
 * learned patterns, and cognitive state.
 */

#ifndef DYNAMIC_RECONFIG_H
#define DYNAMIC_RECONFIG_H

#include "agent_zero.h"
#include "atomspace.h"
#include "learning_module.h"
#include <stdint.h>

/* Forward declarations */
struct reconfig_context;
struct reconfig_trigger;

/**
 * Reconfiguration Trigger Types
 */
typedef enum {
    RECONFIG_TRIGGER_MANUAL,          /* Manual trigger */
    RECONFIG_TRIGGER_PERFORMANCE,     /* Performance degradation */
    RECONFIG_TRIGGER_PATTERN_CHANGE,  /* New pattern learned */
    RECONFIG_TRIGGER_TOPOLOGY_CHANGE, /* AtomSpace topology changed */
    RECONFIG_TRIGGER_SCHEDULED,       /* Scheduled reconfiguration */
    RECONFIG_TRIGGER_THRESHOLD        /* Metric threshold exceeded */
} reconfig_trigger_type;

/**
 * Reconfiguration Event
 */
struct reconfig_event {
    uint64_t event_id;
    time_t timestamp;
    reconfig_trigger_type trigger;
    char *reason;
    
    /* Before/after metrics */
    uint64_t atoms_before;
    uint64_t atoms_after;
    float avg_success_before;
    float avg_success_after;
    
    struct reconfig_event *next;
};

/**
 * Reconfiguration Context - Manages adaptive reconfiguration
 */
struct reconfig_context {
    struct cog_agent *agent_zero;
    struct atom_space *atomspace;
    struct learning_context *learning_ctx;
    
    /* Configuration state */
    char *current_config_path;
    time_t last_reconfig;
    uint64_t reconfig_count;
    
    /* Reconfiguration policy */
    int auto_reconfig_enabled;
    float performance_threshold;      /* Trigger if success rate drops below */
    time_t min_reconfig_interval;     /* Minimum time between reconfigs */
    
    /* Event history */
    struct reconfig_event *event_history;
    size_t max_event_history;
    
    /* Monitoring metrics */
    float current_success_rate;
    uint64_t syncs_since_reconfig;
};

/**
 * Dynamic Reconfiguration Functions
 */

/**
 * reconfig_context_create - Create reconfiguration context
 * @agent_zero: Agent Zero instance
 * @atomspace: AtomSpace
 * @learning_ctx: Learning context
 *
 * Returns: New context or NULL on failure
 */
struct reconfig_context *reconfig_context_create(struct cog_agent *agent_zero,
                                                 struct atom_space *atomspace,
                                                 struct learning_context *learning_ctx);

/**
 * reconfig_context_destroy - Free reconfiguration context
 * @ctx: Context to destroy
 */
void reconfig_context_destroy(struct reconfig_context *ctx);

/**
 * reconfig_enable_auto - Enable automatic reconfiguration
 * @ctx: Reconfiguration context
 * @enabled: Whether to enable auto-reconfig
 * @threshold: Performance threshold to trigger (0.0-1.0)
 * @min_interval: Minimum seconds between reconfigurations
 *
 * Returns: 0 on success, -1 on failure
 */
int reconfig_enable_auto(struct reconfig_context *ctx,
                        int enabled,
                        float threshold,
                        time_t min_interval);

/**
 * reconfig_check_triggers - Check if reconfiguration should occur
 * @ctx: Reconfiguration context
 *
 * Returns: Trigger type if should reconfigure, or -1 if not
 */
int reconfig_check_triggers(struct reconfig_context *ctx);

/**
 * reconfig_trigger_manual - Manually trigger reconfiguration
 * @ctx: Reconfiguration context
 * @reason: Reason for reconfiguration
 *
 * Returns: 0 on success, -1 on failure
 */
int reconfig_trigger_manual(struct reconfig_context *ctx, const char *reason);

/**
 * reconfig_generate - Generate new configuration based on current state
 * @ctx: Reconfiguration context
 * @output_path: Path for new configuration
 *
 * Returns: 0 on success, -1 on failure
 */
int reconfig_generate(struct reconfig_context *ctx, const char *output_path);

/**
 * reconfig_optimize_topology - Optimize AtomSpace topology for performance
 * @ctx: Reconfiguration context
 *
 * Returns: Number of optimizations applied
 */
int reconfig_optimize_topology(struct reconfig_context *ctx);

/**
 * reconfig_adapt_modules - Adapt module configurations based on learning
 * @ctx: Reconfiguration context
 *
 * Returns: Number of modules adapted
 */
int reconfig_adapt_modules(struct reconfig_context *ctx);

/**
 * reconfig_update_priorities - Update module priorities based on importance
 * @ctx: Reconfiguration context
 *
 * Returns: Number of priorities updated
 */
int reconfig_update_priorities(struct reconfig_context *ctx);

/**
 * reconfig_get_events - Get reconfiguration event history
 * @ctx: Reconfiguration context
 * @events: Output array of events
 * @max_events: Maximum events to return
 *
 * Returns: Number of events returned
 */
int reconfig_get_events(struct reconfig_context *ctx,
                       struct reconfig_event **events,
                       size_t max_events);

#endif /* DYNAMIC_RECONFIG_H */
