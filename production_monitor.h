/*
 * Production Monitoring and Hardening
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Provides error recovery, rate limiting, and production monitoring.
 */

#ifndef PRODUCTION_MONITOR_H
#define PRODUCTION_MONITOR_H

#include <stdint.h>
#include <time.h>
#include <stdio.h>

/* Forward declarations */
struct error_recovery_context;
struct rate_limiter;
struct monitor_stats;

/**
 * Error recovery configuration
 */
struct error_recovery_config {
    int max_retries;                /* Maximum retry attempts */
    time_t retry_delay;             /* Delay between retries (seconds) */
    time_t backoff_multiplier;      /* Exponential backoff multiplier */
    int circuit_breaker_threshold;  /* Failures before circuit opens */
    time_t circuit_breaker_timeout; /* Time before retry after circuit opens */
};

/**
 * Error recovery context
 */
struct error_recovery_context {
    struct error_recovery_config config;
    
    /* State */
    int consecutive_failures;
    time_t last_failure_time;
    time_t circuit_open_time;
    int circuit_open;
    
    /* Statistics */
    uint64_t total_errors;
    uint64_t total_retries;
    uint64_t total_recoveries;
    uint64_t circuit_breaker_trips;
};

/**
 * Rate limiter configuration
 */
struct rate_limit_config {
    int max_requests;           /* Max requests per window */
    time_t time_window;         /* Time window (seconds) */
    int burst_size;             /* Max burst size */
};

/**
 * Rate limiter state
 */
struct rate_limiter {
    struct rate_limit_config config;
    
    /* Token bucket state */
    int tokens;
    time_t last_refill;
    
    /* Statistics */
    uint64_t total_requests;
    uint64_t accepted_requests;
    uint64_t rejected_requests;
};

/**
 * Monitor statistics
 */
struct monitor_stats {
    /* System metrics */
    time_t uptime_start;
    uint64_t total_operations;
    uint64_t successful_operations;
    uint64_t failed_operations;
    
    /* Performance metrics */
    double avg_operation_time;
    double max_operation_time;
    double min_operation_time;
    
    /* Resource metrics */
    uint64_t memory_allocated;
    uint64_t memory_freed;
    uint64_t peak_memory_usage;
    
    /* Network metrics */
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t network_errors;
    
    /* Health indicators */
    float health_score;         /* 0.0 - 1.0 */
    time_t last_health_check;
};

/**
 * Error Recovery Functions
 */

/**
 * error_recovery_create - Create error recovery context
 * @config: Configuration
 *
 * Returns: New context or NULL on failure
 */
struct error_recovery_context *error_recovery_create(
    struct error_recovery_config *config);

/**
 * error_recovery_destroy - Free error recovery context
 * @ctx: Context to destroy
 */
void error_recovery_destroy(struct error_recovery_context *ctx);

/**
 * error_recovery_should_retry - Check if operation should be retried
 * @ctx: Error recovery context
 * @error_code: Error code from failed operation
 *
 * Returns: 1 if should retry, 0 if should give up
 */
int error_recovery_should_retry(struct error_recovery_context *ctx,
                                int error_code);

/**
 * error_recovery_record_failure - Record operation failure
 * @ctx: Error recovery context
 *
 * Updates failure counters and circuit breaker state
 */
void error_recovery_record_failure(struct error_recovery_context *ctx);

/**
 * error_recovery_record_success - Record successful operation
 * @ctx: Error recovery context
 *
 * Resets failure counters and closes circuit breaker
 */
void error_recovery_record_success(struct error_recovery_context *ctx);

/**
 * error_recovery_get_retry_delay - Get delay before next retry
 * @ctx: Error recovery context
 * @attempt: Current retry attempt number
 *
 * Returns: Delay in seconds (with exponential backoff)
 */
time_t error_recovery_get_retry_delay(struct error_recovery_context *ctx,
                                     int attempt);

/**
 * Rate Limiting Functions
 */

/**
 * rate_limiter_create - Create rate limiter
 * @config: Configuration
 *
 * Returns: New rate limiter or NULL on failure
 */
struct rate_limiter *rate_limiter_create(struct rate_limit_config *config);

/**
 * rate_limiter_destroy - Free rate limiter
 * @limiter: Rate limiter to destroy
 */
void rate_limiter_destroy(struct rate_limiter *limiter);

/**
 * rate_limiter_allow - Check if request should be allowed
 * @limiter: Rate limiter
 *
 * Returns: 1 if allowed, 0 if rate limited
 */
int rate_limiter_allow(struct rate_limiter *limiter);

/**
 * rate_limiter_wait_time - Get time to wait before retry
 * @limiter: Rate limiter
 *
 * Returns: Time to wait in seconds
 */
time_t rate_limiter_wait_time(struct rate_limiter *limiter);

/**
 * rate_limiter_reset - Reset rate limiter state
 * @limiter: Rate limiter
 */
void rate_limiter_reset(struct rate_limiter *limiter);

/**
 * Monitoring Functions
 */

/**
 * monitor_stats_create - Create monitoring statistics
 *
 * Returns: New stats structure or NULL on failure
 */
struct monitor_stats *monitor_stats_create(void);

/**
 * monitor_stats_destroy - Free monitoring statistics
 * @stats: Stats to destroy
 */
void monitor_stats_destroy(struct monitor_stats *stats);

/**
 * monitor_record_operation - Record an operation
 * @stats: Monitor statistics
 * @duration: Operation duration in milliseconds
 * @success: 1 if successful, 0 if failed
 */
void monitor_record_operation(struct monitor_stats *stats,
                              double duration,
                              int success);

/**
 * monitor_record_memory - Record memory allocation/deallocation
 * @stats: Monitor statistics
 * @bytes: Bytes allocated (positive) or freed (negative)
 */
void monitor_record_memory(struct monitor_stats *stats, int64_t bytes);

/**
 * monitor_record_network - Record network activity
 * @stats: Monitor statistics
 * @bytes_sent: Bytes sent
 * @bytes_received: Bytes received
 * @had_error: 1 if network error occurred
 */
void monitor_record_network(struct monitor_stats *stats,
                           uint64_t bytes_sent,
                           uint64_t bytes_received,
                           int had_error);

/**
 * monitor_calculate_health - Calculate overall health score
 * @stats: Monitor statistics
 *
 * Returns: Health score [0.0 - 1.0]
 */
float monitor_calculate_health(struct monitor_stats *stats);

/**
 * monitor_print_stats - Print monitoring statistics
 * @stats: Monitor statistics
 * @fp: Output file pointer
 */
void monitor_print_stats(struct monitor_stats *stats, FILE *fp);

/**
 * monitor_export_metrics - Export metrics in Prometheus format
 * @stats: Monitor statistics
 * @filename: Output file path
 *
 * Returns: 0 on success, -1 on failure
 */
int monitor_export_metrics(struct monitor_stats *stats, const char *filename);

#endif /* PRODUCTION_MONITOR_H */
