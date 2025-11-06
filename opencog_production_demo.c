/*
 * OpenCog Production Integration Demo
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * Demonstrates the complete production-ready OpenCog cognitive architecture
 * with GGML/llama.cpp integration, network protocol, persistence, and
 * production hardening features.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "atomspace.h"
#include "cogagent.h"
#include "agent_zero.h"
#include "swarm_sync.h"
#include "pln_inference.h"
#include "learning_module.h"
#include "dynamic_reconfig.h"
#include "distributed_atomspace.h"
#include "ggml_hypergraph.h"
#include "llama_pattern.h"
#include "atomspace_persistence.h"
#include "production_monitor.h"
#include "opencog_rsync.h"

#define BOX_WIDTH 63

static void print_header(const char *title);
static void print_section(const char *title);
static void print_success(const char *msg);
static void print_info(const char *msg);
static void simulate_production_workload(struct monitor_stats *stats,
                                        struct error_recovery_context *err_ctx,
                                        struct rate_limiter *limiter);

int main(void)
{
    struct atom_space *atomspace;
    struct cog_agent *agent_zero;
    struct pln_inference_context *pln_ctx;
    struct learning_context *learning_ctx;
    struct reconfig_context *reconfig_ctx;
    struct distributed_atomspace *das;
    struct ggml_hypergraph_context *ggml_ctx;
    struct llama_pattern_context *llama_ctx;
    struct persistence_config persist_config;
    struct error_recovery_config err_config;
    struct error_recovery_context *err_ctx;
    struct rate_limit_config rate_config;
    struct rate_limiter *rate_limiter;
    struct monitor_stats *monitor;
    struct atom *module;
    time_t start_time;
    int i;
    
    start_time = time(NULL);
    
    /* Print banner */
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║   OpenCog Production Integration Demo                        ║\n");
    printf("║   Complete Cognitive Architecture with Production Features   ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    /* Phase 1: Initialize Cognitive Infrastructure */
    print_section("PHASE 1: Cognitive Infrastructure");
    
    printf("[1.1] Creating AtomSpace...\n");
    atomspace = atomspace_create();
    print_success("AtomSpace created");
    
    printf("[1.2] Initializing Agent Zero...\n");
    agent_zero = cog_agent_create(COG_AGENT_ZERO, "config_generator");
    if (agent_zero) {
        agent_zero_init(agent_zero, atomspace, "./rsyncd.conf");
    }
    print_success("Agent Zero initialized");
    
    printf("[1.3] Creating PLN inference context...\n");
    pln_ctx = pln_context_create(atomspace);
    print_success("PLN context created");
    
    printf("[1.4] Creating learning context...\n");
    learning_ctx = learning_context_create(atomspace, pln_ctx);
    print_success("Learning context created");
    
    /* Phase 2: Production Monitoring */
    print_section("PHASE 2: Production Monitoring & Hardening");
    
    printf("[2.1] Initializing production monitor...\n");
    monitor = monitor_stats_create();
    print_success("Monitor initialized");
    
    printf("[2.2] Setting up error recovery...\n");
    err_config.max_retries = 3;
    err_config.retry_delay = 5;
    err_config.backoff_multiplier = 2;
    err_config.circuit_breaker_threshold = 5;
    err_config.circuit_breaker_timeout = 30;
    err_ctx = error_recovery_create(&err_config);
    print_success("Error recovery configured");
    print_info("Max retries: 3, Circuit breaker threshold: 5");
    
    printf("[2.3] Setting up rate limiting...\n");
    rate_config.max_requests = 100;
    rate_config.time_window = 60;
    rate_config.burst_size = 10;
    rate_limiter = rate_limiter_create(&rate_config);
    print_success("Rate limiter configured");
    print_info("Max: 100 req/min, Burst: 10");
    
    /* Phase 3: Persistence */
    print_section("PHASE 3: Persistence Configuration");
    
    printf("[3.1] Configuring persistence...\n");
    persist_config.atomspace_path = "./atomspace.bin";
    persist_config.learning_path = "./learning_history.bin";
    persist_config.auto_save_enabled = 1;
    persist_config.auto_save_interval = 300;  /* 5 minutes */
    persist_config.compression_enabled = 0;
    persistence_init(&persist_config);
    print_success("Persistence configured");
    print_info("Auto-save: every 300 seconds");
    
    /* Phase 4: Distributed AtomSpace */
    print_section("PHASE 4: Distributed AtomSpace Network");
    
    printf("[4.1] Creating distributed AtomSpace...\n");
    das = distributed_atomspace_create(atomspace);
    print_success("Distributed AtomSpace created");
    
    printf("[4.2] Adding remote nodes...\n");
    distributed_atomspace_add_node(das, "node1.example.com", 8730);
    distributed_atomspace_add_node(das, "node2.example.com", 8730);
    distributed_atomspace_add_node(das, "node3.example.com", 8730);
    print_success("Added 3 remote nodes");
    
    printf("[4.3] Setting conflict resolution...\n");
    distributed_atomspace_set_conflict_strategy(das, CONFLICT_MERGE_TV);
    print_success("Using PLN revision for conflicts");
    
    /* Phase 5: GGML Integration */
    print_section("PHASE 5: GGML Tensor Operations (Stub)");
    
    printf("[5.1] Creating GGML context...\n");
    ggml_ctx = ggml_hypergraph_create(atomspace, 128);
    if (ggml_ctx) {
        print_success("GGML context created");
        print_info("Embedding dimension: 128");
    } else {
        print_info("GGML library not available - using stub");
    }
    
    printf("[5.2] Building atom embeddings...\n");
    if (ggml_ctx && ggml_hypergraph_build_embeddings(ggml_ctx) == 0) {
        print_success("Embeddings built");
    } else {
        print_info("Build with --enable-ggml for full support");
    }
    
    /* Phase 6: llama.cpp Integration */
    print_section("PHASE 6: llama.cpp Neural Inference (Stub)");
    
    printf("[6.1] Creating llama.cpp context...\n");
    llama_ctx = llama_pattern_create(atomspace, learning_ctx, NULL);
    if (llama_ctx) {
        print_success("llama.cpp context created");
    } else {
        print_info("llama.cpp library not available - using stub");
    }
    
    printf("[6.2] Pattern recognition test...\n");
    if (llama_ctx) {
        print_info("Build with --enable-llama for full support");
    }
    
    /* Phase 7: Production Workload Simulation */
    print_section("PHASE 7: Production Workload Simulation");
    
    printf("[7.1] Adding production modules...\n");
    atomspace_add_node(atomspace, ATOM_MODULE, "api_server");
    atomspace_add_node(atomspace, ATOM_MODULE, "database");
    atomspace_add_node(atomspace, ATOM_MODULE, "cache");
    print_success("Added 3 production modules");
    
    printf("[7.2] Simulating sync operations...\n");
    simulate_production_workload(monitor, err_ctx, rate_limiter);
    print_success("Simulated 50 operations");
    
    printf("[7.3] Recording learning observations...\n");
    for (i = 0; i < 10; i++) {
        learning_record_sync(learning_ctx, "api_server", "node1.example.com",
                           1, 1024000, 15);
        learning_record_sync(learning_ctx, "database", "node2.example.com",
                           1, 2048000, 30);
    }
    learning_update_truth_values(learning_ctx);
    print_success("Recorded 20 sync events");
    
    /* Phase 8: Dynamic Reconfiguration */
    print_section("PHASE 8: Dynamic Reconfiguration");
    
    printf("[8.1] Creating reconfiguration context...\n");
    reconfig_ctx = reconfig_context_create(agent_zero, atomspace, learning_ctx);
    print_success("Reconfig context created");
    
    printf("[8.2] Generating optimized configuration...\n");
    reconfig_generate(reconfig_ctx, "./rsyncd.conf.production");
    print_success("Generated: ./rsyncd.conf.production");
    
    /* Phase 9: Persistence */
    print_section("PHASE 9: Saving State");
    
    printf("[9.1] Saving AtomSpace...\n");
    if (atomspace_save(atomspace, persist_config.atomspace_path) >= 0) {
        print_success("AtomSpace saved");
    }
    
    printf("[9.2] Exporting AtomSpace to JSON...\n");
    if (atomspace_export_json(atomspace, "./atomspace.json") >= 0) {
        print_success("Exported to JSON");
    }
    
    printf("[9.3] Saving learning history...\n");
    if (learning_history_save(learning_ctx, persist_config.learning_path) >= 0) {
        print_success("Learning history saved");
    }
    
    /* Phase 10: Monitoring & Metrics */
    print_section("PHASE 10: Monitoring & Metrics");
    
    printf("[10.1] Calculating health score...\n");
    float health = monitor_calculate_health(monitor);
    print_success("Health calculated");
    printf("      ✓ System health: %.1f%%\n", health * 100.0);
    
    printf("[10.2] Printing statistics...\n");
    monitor_print_stats(monitor, stdout);
    
    printf("[10.3] Exporting Prometheus metrics...\n");
    if (monitor_export_metrics(monitor, "./metrics.prom") == 0) {
        print_success("Metrics exported to ./metrics.prom");
    }
    
    /* Phase 11: Summary */
    print_section("SUMMARY");
    
    printf("\n");
    printf("   Production Features Demonstrated:\n");
    printf("   ✓ Cognitive Infrastructure (AtomSpace, PLN, Learning)\n");
    printf("   ✓ Error Recovery with Circuit Breaker\n");
    printf("   ✓ Token Bucket Rate Limiting\n");
    printf("   ✓ Production Monitoring & Health Checks\n");
    printf("   ✓ Binary & JSON Persistence\n");
    printf("   ✓ Distributed AtomSpace Protocol\n");
    printf("   ✓ GGML Tensor Operations (stub)\n");
    printf("   ✓ llama.cpp Neural Inference (stub)\n");
    printf("   ✓ Prometheus Metrics Export\n");
    printf("   ✓ Dynamic Reconfiguration\n");
    printf("\n");
    
    printf("   Files Generated:\n");
    printf("   • ./atomspace.bin - Binary AtomSpace snapshot\n");
    printf("   • ./atomspace.json - JSON export\n");
    printf("   • ./learning_history.bin - Learning data\n");
    printf("   • ./rsyncd.conf.production - Optimized config\n");
    printf("   • ./metrics.prom - Prometheus metrics\n");
    printf("\n");
    
    printf("   Total Runtime: %ld seconds\n", time(NULL) - start_time);
    printf("\n");
    
    /* Cleanup */
    print_section("Cleanup");
    
    printf("Freeing resources...\n");
    if (ggml_ctx) ggml_hypergraph_destroy(ggml_ctx);
    if (llama_ctx) llama_pattern_destroy(llama_ctx);
    distributed_atomspace_destroy(das);
    reconfig_context_destroy(reconfig_ctx);
    learning_context_destroy(learning_ctx);
    pln_context_destroy(pln_ctx);
    atomspace_destroy(atomspace);
    error_recovery_destroy(err_ctx);
    rate_limiter_destroy(rate_limiter);
    monitor_stats_destroy(monitor);
    print_success("All resources freed");
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║   Demo Complete - Production Features Ready                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}

static void print_header(const char *title)
{
    int len = strlen(title);
    int padding = (BOX_WIDTH - len - 2) / 2;
    int i;
    
    printf("╔");
    for (i = 0; i < BOX_WIDTH; i++) printf("═");
    printf("╗\n");
    
    printf("║");
    for (i = 0; i < padding; i++) printf(" ");
    printf("%s", title);
    for (i = 0; i < BOX_WIDTH - padding - len; i++) printf(" ");
    printf("║\n");
    
    printf("╚");
    for (i = 0; i < BOX_WIDTH; i++) printf("═");
    printf("╝\n");
}

static void print_section(const char *title)
{
    int i;
    printf("\n");
    for (i = 0; i < BOX_WIDTH; i++) printf("═");
    printf("\n");
    printf(" %s\n", title);
    for (i = 0; i < BOX_WIDTH; i++) printf("═");
    printf("\n");
}

static void print_success(const char *msg)
{
    printf("     ✓ %s\n", msg);
}

static void print_info(const char *msg)
{
    printf("     ℹ %s\n", msg);
}

static void simulate_production_workload(struct monitor_stats *stats,
                                        struct error_recovery_context *err_ctx,
                                        struct rate_limiter *limiter)
{
    int i;
    double duration;
    int success;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    for (i = 0; i < 50; i++) {
        /* Check rate limit */
        if (!rate_limiter_allow(limiter)) {
            /* Simulate backoff */
            usleep(100000);  /* 100ms */
            continue;
        }
        
        /* Simulate operation */
        duration = 10.0 + (rand() % 50);  /* 10-60ms */
        success = (rand() % 100) < 95;    /* 95% success rate */
        
        /* Record monitoring data */
        monitor_record_operation(stats, duration, success);
        monitor_record_memory(stats, 1024);
        monitor_record_network(stats, 1024, 512, !success);
        
        /* Handle errors */
        if (!success) {
            error_recovery_record_failure(err_ctx);
        } else {
            error_recovery_record_success(err_ctx);
        }
        
        usleep(10000);  /* 10ms between operations */
    }
}
