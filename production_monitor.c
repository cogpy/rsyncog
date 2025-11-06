/*
 * Production Monitoring and Hardening Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "production_monitor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/**
 * error_recovery_create - Create error recovery context
 */
struct error_recovery_context *error_recovery_create(
    struct error_recovery_config *config)
{
    struct error_recovery_context *ctx;
    
    if (!config)
        return NULL;
    
    ctx = malloc(sizeof(struct error_recovery_context));
    if (!ctx)
        return NULL;
    
    memset(ctx, 0, sizeof(struct error_recovery_context));
    ctx->config = *config;
    
    return ctx;
}

/**
 * error_recovery_destroy - Free error recovery context
 */
void error_recovery_destroy(struct error_recovery_context *ctx)
{
    free(ctx);
}

/**
 * error_recovery_should_retry - Check if should retry
 */
int error_recovery_should_retry(struct error_recovery_context *ctx,
                                int error_code)
{
    time_t now;
    
    if (!ctx)
        return 0;
    
    /* Check circuit breaker */
    if (ctx->circuit_open) {
        now = time(NULL);
        if (now - ctx->circuit_open_time < ctx->config.circuit_breaker_timeout)
            return 0;  /* Circuit still open */
        
        /* Try to close circuit */
        ctx->circuit_open = 0;
    }
    
    /* Check retry limit */
    if (ctx->consecutive_failures >= ctx->config.max_retries)
        return 0;
    
    /* Retry for transient errors */
    if (error_code == -1 || error_code == -2)  /* Network or timeout errors */
        return 1;
    
    return 0;
}

/**
 * error_recovery_record_failure - Record failure
 */
void error_recovery_record_failure(struct error_recovery_context *ctx)
{
    if (!ctx)
        return;
    
    ctx->consecutive_failures++;
    ctx->total_errors++;
    ctx->last_failure_time = time(NULL);
    
    /* Check circuit breaker threshold */
    if (ctx->consecutive_failures >= ctx->config.circuit_breaker_threshold) {
        ctx->circuit_open = 1;
        ctx->circuit_open_time = time(NULL);
        ctx->circuit_breaker_trips++;
    }
}

/**
 * error_recovery_record_success - Record success
 */
void error_recovery_record_success(struct error_recovery_context *ctx)
{
    if (!ctx)
        return;
    
    if (ctx->consecutive_failures > 0)
        ctx->total_recoveries++;
    
    ctx->consecutive_failures = 0;
    ctx->circuit_open = 0;
}

/**
 * error_recovery_get_retry_delay - Get retry delay with backoff
 */
time_t error_recovery_get_retry_delay(struct error_recovery_context *ctx,
                                     int attempt)
{
    time_t delay;
    
    if (!ctx)
        return 0;
    
    /* Exponential backoff */
    delay = ctx->config.retry_delay;
    if (attempt > 1) {
        delay *= pow(ctx->config.backoff_multiplier, attempt - 1);
    }
    
    return delay;
}

/**
 * rate_limiter_create - Create rate limiter
 */
struct rate_limiter *rate_limiter_create(struct rate_limit_config *config)
{
    struct rate_limiter *limiter;
    
    if (!config)
        return NULL;
    
    limiter = malloc(sizeof(struct rate_limiter));
    if (!limiter)
        return NULL;
    
    memset(limiter, 0, sizeof(struct rate_limiter));
    limiter->config = *config;
    limiter->tokens = config->max_requests;
    limiter->last_refill = time(NULL);
    
    return limiter;
}

/**
 * rate_limiter_destroy - Free rate limiter
 */
void rate_limiter_destroy(struct rate_limiter *limiter)
{
    free(limiter);
}

/**
 * rate_limiter_refill - Refill token bucket
 */
static void rate_limiter_refill(struct rate_limiter *limiter)
{
    time_t now, elapsed;
    int tokens_to_add;
    
    now = time(NULL);
    elapsed = now - limiter->last_refill;
    
    if (elapsed >= limiter->config.time_window) {
        /* Refill full bucket */
        limiter->tokens = limiter->config.max_requests;
        limiter->last_refill = now;
    } else if (elapsed > 0) {
        /* Partial refill */
        tokens_to_add = (limiter->config.max_requests * elapsed) / 
                        limiter->config.time_window;
        limiter->tokens += tokens_to_add;
        if (limiter->tokens > limiter->config.max_requests)
            limiter->tokens = limiter->config.max_requests;
        limiter->last_refill = now;
    }
}

/**
 * rate_limiter_allow - Check if request allowed
 */
int rate_limiter_allow(struct rate_limiter *limiter)
{
    if (!limiter)
        return 1;
    
    rate_limiter_refill(limiter);
    
    limiter->total_requests++;
    
    if (limiter->tokens > 0) {
        limiter->tokens--;
        limiter->accepted_requests++;
        return 1;
    }
    
    limiter->rejected_requests++;
    return 0;
}

/**
 * rate_limiter_wait_time - Get wait time
 */
time_t rate_limiter_wait_time(struct rate_limiter *limiter)
{
    time_t now, elapsed, remaining;
    
    if (!limiter || limiter->tokens > 0)
        return 0;
    
    now = time(NULL);
    elapsed = now - limiter->last_refill;
    remaining = limiter->config.time_window - elapsed;
    
    return remaining > 0 ? remaining : 0;
}

/**
 * rate_limiter_reset - Reset rate limiter
 */
void rate_limiter_reset(struct rate_limiter *limiter)
{
    if (!limiter)
        return;
    
    limiter->tokens = limiter->config.max_requests;
    limiter->last_refill = time(NULL);
}

/**
 * monitor_stats_create - Create monitoring stats
 */
struct monitor_stats *monitor_stats_create(void)
{
    struct monitor_stats *stats;
    
    stats = malloc(sizeof(struct monitor_stats));
    if (!stats)
        return NULL;
    
    memset(stats, 0, sizeof(struct monitor_stats));
    stats->uptime_start = time(NULL);
    stats->min_operation_time = 999999.0;
    stats->health_score = 1.0;
    
    return stats;
}

/**
 * monitor_stats_destroy - Free monitoring stats
 */
void monitor_stats_destroy(struct monitor_stats *stats)
{
    free(stats);
}

/**
 * monitor_record_operation - Record operation
 */
void monitor_record_operation(struct monitor_stats *stats,
                              double duration,
                              int success)
{
    if (!stats)
        return;
    
    stats->total_operations++;
    
    if (success) {
        stats->successful_operations++;
    } else {
        stats->failed_operations++;
    }
    
    /* Update timing statistics */
    if (duration > stats->max_operation_time)
        stats->max_operation_time = duration;
    if (duration < stats->min_operation_time)
        stats->min_operation_time = duration;
    
    /* Update running average */
    stats->avg_operation_time = 
        (stats->avg_operation_time * (stats->total_operations - 1) + duration) /
        stats->total_operations;
}

/**
 * monitor_record_memory - Record memory usage
 */
void monitor_record_memory(struct monitor_stats *stats, int64_t bytes)
{
    if (!stats)
        return;
    
    if (bytes > 0) {
        stats->memory_allocated += bytes;
        if (stats->memory_allocated - stats->memory_freed > stats->peak_memory_usage)
            stats->peak_memory_usage = stats->memory_allocated - stats->memory_freed;
    } else {
        stats->memory_freed += (-bytes);
    }
}

/**
 * monitor_record_network - Record network activity
 */
void monitor_record_network(struct monitor_stats *stats,
                           uint64_t bytes_sent,
                           uint64_t bytes_received,
                           int had_error)
{
    if (!stats)
        return;
    
    stats->bytes_sent += bytes_sent;
    stats->bytes_received += bytes_received;
    
    if (had_error)
        stats->network_errors++;
}

/**
 * monitor_calculate_health - Calculate health score
 */
float monitor_calculate_health(struct monitor_stats *stats)
{
    float success_rate, error_rate, health;
    
    if (!stats)
        return 0.0;
    
    /* Calculate success rate */
    if (stats->total_operations > 0) {
        success_rate = (float)stats->successful_operations / 
                      (float)stats->total_operations;
    } else {
        success_rate = 1.0;
    }
    
    /* Calculate error rate penalty */
    if (stats->total_operations > 0) {
        error_rate = (float)stats->failed_operations / 
                    (float)stats->total_operations;
    } else {
        error_rate = 0.0;
    }
    
    /* Combine metrics */
    health = success_rate * (1.0 - error_rate * 0.5);
    
    /* Network errors reduce health */
    if (stats->network_errors > 100) {
        health *= 0.9;
    }
    
    stats->health_score = health;
    stats->last_health_check = time(NULL);
    
    return health;
}

/**
 * monitor_print_stats - Print statistics
 */
void monitor_print_stats(struct monitor_stats *stats, FILE *fp)
{
    time_t uptime;
    float success_rate;
    
    if (!stats || !fp)
        return;
    
    uptime = time(NULL) - stats->uptime_start;
    success_rate = stats->total_operations > 0 ?
                   (float)stats->successful_operations / stats->total_operations * 100.0 :
                   100.0;
    
    fprintf(fp, "\n=== Production Monitoring Statistics ===\n");
    fprintf(fp, "Uptime: %ld seconds\n", uptime);
    fprintf(fp, "\nOperations:\n");
    fprintf(fp, "  Total: %lu\n", stats->total_operations);
    fprintf(fp, "  Successful: %lu (%.2f%%)\n", stats->successful_operations, success_rate);
    fprintf(fp, "  Failed: %lu\n", stats->failed_operations);
    fprintf(fp, "\nPerformance:\n");
    fprintf(fp, "  Avg Time: %.3f ms\n", stats->avg_operation_time);
    fprintf(fp, "  Max Time: %.3f ms\n", stats->max_operation_time);
    fprintf(fp, "  Min Time: %.3f ms\n", stats->min_operation_time);
    fprintf(fp, "\nMemory:\n");
    fprintf(fp, "  Allocated: %lu bytes\n", stats->memory_allocated);
    fprintf(fp, "  Freed: %lu bytes\n", stats->memory_freed);
    fprintf(fp, "  Peak Usage: %lu bytes\n", stats->peak_memory_usage);
    fprintf(fp, "\nNetwork:\n");
    fprintf(fp, "  Bytes Sent: %lu\n", stats->bytes_sent);
    fprintf(fp, "  Bytes Received: %lu\n", stats->bytes_received);
    fprintf(fp, "  Errors: %lu\n", stats->network_errors);
    fprintf(fp, "\nHealth Score: %.2f%%\n", stats->health_score * 100.0);
    fprintf(fp, "=====================================\n\n");
}

/**
 * monitor_export_metrics - Export in Prometheus format
 */
int monitor_export_metrics(struct monitor_stats *stats, const char *filename)
{
    FILE *fp;
    time_t uptime;
    
    if (!stats || !filename)
        return -1;
    
    fp = fopen(filename, "w");
    if (!fp)
        return -1;
    
    uptime = time(NULL) - stats->uptime_start;
    
    /* Export metrics in Prometheus format */
    fprintf(fp, "# HELP opencog_uptime_seconds System uptime\n");
    fprintf(fp, "# TYPE opencog_uptime_seconds counter\n");
    fprintf(fp, "opencog_uptime_seconds %ld\n", uptime);
    
    fprintf(fp, "# HELP opencog_operations_total Total operations\n");
    fprintf(fp, "# TYPE opencog_operations_total counter\n");
    fprintf(fp, "opencog_operations_total %lu\n", stats->total_operations);
    
    fprintf(fp, "# HELP opencog_operations_successful Successful operations\n");
    fprintf(fp, "# TYPE opencog_operations_successful counter\n");
    fprintf(fp, "opencog_operations_successful %lu\n", stats->successful_operations);
    
    fprintf(fp, "# HELP opencog_operations_failed Failed operations\n");
    fprintf(fp, "# TYPE opencog_operations_failed counter\n");
    fprintf(fp, "opencog_operations_failed %lu\n", stats->failed_operations);
    
    fprintf(fp, "# HELP opencog_operation_duration_avg Average operation duration\n");
    fprintf(fp, "# TYPE opencog_operation_duration_avg gauge\n");
    fprintf(fp, "opencog_operation_duration_avg %.6f\n", stats->avg_operation_time);
    
    fprintf(fp, "# HELP opencog_memory_allocated_bytes Total memory allocated\n");
    fprintf(fp, "# TYPE opencog_memory_allocated_bytes counter\n");
    fprintf(fp, "opencog_memory_allocated_bytes %lu\n", stats->memory_allocated);
    
    fprintf(fp, "# HELP opencog_memory_peak_bytes Peak memory usage\n");
    fprintf(fp, "# TYPE opencog_memory_peak_bytes gauge\n");
    fprintf(fp, "opencog_memory_peak_bytes %lu\n", stats->peak_memory_usage);
    
    fprintf(fp, "# HELP opencog_network_bytes_sent Total bytes sent\n");
    fprintf(fp, "# TYPE opencog_network_bytes_sent counter\n");
    fprintf(fp, "opencog_network_bytes_sent %lu\n", stats->bytes_sent);
    
    fprintf(fp, "# HELP opencog_network_bytes_received Total bytes received\n");
    fprintf(fp, "# TYPE opencog_network_bytes_received counter\n");
    fprintf(fp, "opencog_network_bytes_received %lu\n", stats->bytes_received);
    
    fprintf(fp, "# HELP opencog_network_errors_total Network errors\n");
    fprintf(fp, "# TYPE opencog_network_errors_total counter\n");
    fprintf(fp, "opencog_network_errors_total %lu\n", stats->network_errors);
    
    fprintf(fp, "# HELP opencog_health_score System health score\n");
    fprintf(fp, "# TYPE opencog_health_score gauge\n");
    fprintf(fp, "opencog_health_score %.6f\n", stats->health_score);
    
    fclose(fp);
    return 0;
}
