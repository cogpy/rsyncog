/*
 * OpenCog rsync Example - Demonstrates autonomous multi-agent orchestration
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Simple standalone example without full rsync dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include "cogagent.h"
#include "atomspace.h"
#include "swarm_sync.h"
#include "agent_zero.h"

/**
 * Example: Create distributed AtomSpace sync topology with swarm formations
 */
int main(void)
{
    struct atom_space *atomspace;
    struct cog_agent *zero, *monitor, *auth;
    struct atom *mod1, *mod2, *mod3, *mod4;
    struct swarm_formation *swarm1, *swarm2;
    const char *members1[] = {"data_module", "backup_module"};
    size_t i;
    
    printf("OpenCog Autonomous Multi-Agent Orchestration for rsync\n");
    printf("=======================================================\n\n");
    
    /* Initialize OpenCog cognitive infrastructure */
    printf("[1] Initializing OpenCog cognitive infrastructure...\n");
    
    atomspace = atomspace_create();
    if (!atomspace) {
        fprintf(stderr, "Failed to create AtomSpace\n");
        return 1;
    }
    printf("    ✓ AtomSpace created\n");
    
    zero = cog_agent_create(COG_AGENT_ZERO, "agent_zero");
    if (!zero || cog_agent_init(zero, atomspace) != 0) {
        fprintf(stderr, "Failed to create Agent Zero\n");
        return 1;
    }
    cog_agent_register(zero);
    printf("    ✓ Agent Zero (master builder) initialized\n");
    
    monitor = cog_agent_create(COG_AGENT_MONITOR, "agent_monitor");
    if (!monitor || cog_agent_init(monitor, atomspace) != 0) {
        fprintf(stderr, "Failed to create Monitor Agent\n");
        return 1;
    }
    cog_agent_register(monitor);
    printf("    ✓ Agent Monitor initialized\n");
    
    auth = cog_agent_create(COG_AGENT_AUTH, "agent_auth");
    if (!auth || cog_agent_init(auth, atomspace) != 0) {
        fprintf(stderr, "Failed to create Auth Agent\n");
        return 1;
    }
    cog_agent_register(auth);
    printf("    ✓ Agent Auth initialized\n\n");
    
    /* Add modules to AtomSpace */
    printf("[2] Adding rsync modules to AtomSpace...\n");
    
    mod1 = atomspace_add_node(atomspace, ATOM_MODULE, "data_module");
    atom_set_tv(mod1, 1.0, 0.9);
    atom_set_sti(mod1, 100);
    printf("    ✓ Added data_module\n");
    
    mod2 = atomspace_add_node(atomspace, ATOM_MODULE, "backup_module");
    atom_set_tv(mod2, 1.0, 0.9);
    atom_set_sti(mod2, 80);
    printf("    ✓ Added backup_module\n");
    
    mod3 = atomspace_add_node(atomspace, ATOM_MODULE, "logs_module");
    atom_set_tv(mod3, 0.5, 0.9);  /* read-only */
    atom_set_sti(mod3, 60);
    printf("    ✓ Added logs_module [read-only]\n");
    
    mod4 = atomspace_add_node(atomspace, ATOM_MODULE, "metrics_module");
    atom_set_tv(mod4, 0.5, 0.9);  /* read-only */
    atom_set_sti(mod4, 60);
    printf("    ✓ Added metrics_module [read-only]\n\n");
    
    /* Create swarm formations */
    printf("[3] Creating HyperGNN swarm formations...\n");
    
    swarm1 = swarm_create(zero, atomspace, "production_swarm");
    if (swarm1) {
        swarm_add_member(swarm1, mod1, "host1.example.com", 873);
        swarm_add_member(swarm1, mod2, "host2.example.com", 873);
        printf("    ✓ Created production_swarm with 2 members\n");
    }
    
    swarm2 = swarm_create(zero, atomspace, "monitoring_swarm");
    if (swarm2) {
        swarm_add_member(swarm2, mod3, "monitor1.example.com", 873);
        swarm_add_member(swarm2, mod4, "monitor2.example.com", 873);
        printf("    ✓ Created monitoring_swarm with 2 members\n\n");
    }
    
    /* Activate swarms */
    printf("[4] Activating swarm formations...\n");
    if (swarm1) {
        swarm_activate(swarm1);
        printf("    ✓ production_swarm activated\n");
    }
    if (swarm2) {
        swarm_activate(swarm2);
        printf("    ✓ monitoring_swarm activated\n");
    }
    printf("\n");
    
    /* Generate rsyncd.conf */
    printf("[5] Generating rsyncd.conf via Agent Zero...\n");
    if (agent_zero_init(zero, atomspace, "./rsyncd.conf.generated") == 0 &&
        agent_zero_generate_config(zero) == 0) {
        printf("    ✓ Configuration generated: ./rsyncd.conf.generated\n");
        printf("    ✓ AtomSpace topology embedded in config\n");
        printf("    ✓ Swarm formations configured\n\n");
    } else {
        fprintf(stderr, "    ✗ Failed to generate configuration\n\n");
    }
    
    /* Simulate sync operations with attention updates */
    printf("[6] Simulating cognitive sync operations...\n");
    printf("    ✓ Sync started on data_module\n");
    atom_set_sti(mod1, mod1->av.sti + 10);
    printf("    ✓ Short-term importance (STI) increased to %d\n", (int)mod1->av.sti);
    
    atom_set_tv(mod1, 1.0, 0.95);
    atom_set_lti(mod1, mod1->av.lti + 1);
    printf("    ✓ Sync completed successfully\n");
    printf("    ✓ Truth value updated (strength=%.2f, confidence=%.2f)\n",
           mod1->tv.strength, mod1->tv.confidence);
    printf("    ✓ Long-term importance (LTI) incremented to %d\n\n",
           (int)mod1->av.lti);
    
    /* Display AtomSpace statistics */
    printf("[7] AtomSpace Statistics:\n");
    printf("    • Total atoms: %llu\n",
           (unsigned long long)atomspace->atom_count);
    printf("    • Total links: %llu\n",
           (unsigned long long)atomspace->link_count);
    printf("    • Swarm formations: 2\n");
    printf("    • Active modules: 4\n\n");
    
    /* Display swarm health */
    printf("[8] Swarm Health Status:\n");
    if (swarm1) {
        int health = swarm_check_health(swarm1);
        printf("    • production_swarm: %d%% healthy\n", health);
    }
    if (swarm2) {
        int health = swarm_check_health(swarm2);
        printf("    • monitoring_swarm: %d%% healthy\n", health);
    }
    printf("\n");
    
    /* Cleanup */
    printf("[9] Shutting down cognitive infrastructure...\n");
    if (swarm1)
        swarm_destroy(swarm1);
    if (swarm2)
        swarm_destroy(swarm2);
    
    if (auth) {
        cog_agent_stop(auth);
        cog_agent_destroy(auth);
    }
    if (monitor) {
        cog_agent_stop(monitor);
        cog_agent_destroy(monitor);
    }
    if (zero) {
        cog_agent_stop(zero);
        cog_agent_destroy(zero);
    }
    
    atomspace_destroy(atomspace);
    printf("    ✓ All agents stopped\n");
    printf("    ✓ AtomSpace destroyed\n");
    printf("    ✓ Resources freed\n\n");
    
    printf("OpenCog rsync orchestration complete.\n");
    printf("Check ./rsyncd.conf.generated for the generated configuration.\n");
    
    return 0;
}
