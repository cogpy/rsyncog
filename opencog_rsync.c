/*
 * OpenCog Integration Implementation for rsync
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "opencog_rsync.h"
#include "agent_zero.h"
#include <stdlib.h>
#include <string.h>

/* OpenCog global state */
int opencog_enabled = 0;
struct atom_space *global_atomspace = NULL;
struct cog_agent *agent_zero = NULL;
struct cog_agent *agent_monitor = NULL;
struct cog_agent *agent_auth = NULL;

/**
 * opencog_init - Initialize OpenCog cognitive infrastructure
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_init(void)
{
    if (opencog_enabled)
        return 0;
    
    /* Create global AtomSpace */
    global_atomspace = atomspace_create();
    if (!global_atomspace)
        return -1;
    
    /* Create agent zero (master builder) */
    agent_zero = cog_agent_create(COG_AGENT_ZERO, "agent_zero");
    if (!agent_zero)
        goto error;
    
    if (cog_agent_init(agent_zero, global_atomspace) != 0)
        goto error;
    
    if (cog_agent_register(agent_zero) != 0)
        goto error;
    
    /* Create monitoring agent */
    agent_monitor = cog_agent_create(COG_AGENT_MONITOR, "agent_monitor");
    if (!agent_monitor)
        goto error;
    
    if (cog_agent_init(agent_monitor, global_atomspace) != 0)
        goto error;
    
    if (cog_agent_register(agent_monitor) != 0)
        goto error;
    
    /* Create authentication agent */
    agent_auth = cog_agent_create(COG_AGENT_AUTH, "agent_auth");
    if (!agent_auth)
        goto error;
    
    if (cog_agent_init(agent_auth, global_atomspace) != 0)
        goto error;
    
    if (cog_agent_register(agent_auth) != 0)
        goto error;
    
    opencog_enabled = 1;
    
    return 0;

error:
    opencog_shutdown();
    return -1;
}

/**
 * opencog_shutdown - Shutdown OpenCog infrastructure
 */
void opencog_shutdown(void)
{
    if (!opencog_enabled)
        return;
    
    if (agent_auth) {
        cog_agent_stop(agent_auth);
        cog_agent_destroy(agent_auth);
        agent_auth = NULL;
    }
    
    if (agent_monitor) {
        cog_agent_stop(agent_monitor);
        cog_agent_destroy(agent_monitor);
        agent_monitor = NULL;
    }
    
    if (agent_zero) {
        cog_agent_stop(agent_zero);
        cog_agent_destroy(agent_zero);
        agent_zero = NULL;
    }
    
    if (global_atomspace) {
        atomspace_destroy(global_atomspace);
        global_atomspace = NULL;
    }
    
    opencog_enabled = 0;
}

/**
 * opencog_daemon_init - Initialize daemon with OpenCog
 * @config_file: Path to rsyncd.conf
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_daemon_init(const char *config_file)
{
    if (!opencog_enabled) {
        if (opencog_init() != 0)
            return -1;
    }
    
    /* Build sync topology from config */
    if (config_file) {
        atomspace_build_sync_topology(global_atomspace, config_file);
    }
    
    return 0;
}

/**
 * opencog_daemon_start - Start cognitive daemon agents
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_daemon_start(void)
{
    if (!opencog_enabled)
        return -1;
    
    /* Start all agents */
    if (agent_zero)
        cog_agent_start(agent_zero);
    
    if (agent_monitor)
        cog_agent_start(agent_monitor);
    
    if (agent_auth)
        cog_agent_start(agent_auth);
    
    return 0;
}

/**
 * opencog_daemon_stop - Stop cognitive daemon agents
 */
void opencog_daemon_stop(void)
{
    if (agent_zero)
        cog_agent_stop(agent_zero);
    
    if (agent_monitor)
        cog_agent_stop(agent_monitor);
    
    if (agent_auth)
        cog_agent_stop(agent_auth);
}

/**
 * opencog_register_module - Register module in AtomSpace
 * @module_name: Module name
 * @path: Module path
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_register_module(const char *module_name, const char *path)
{
    struct atom *module_atom;
    
    if (!opencog_enabled || !module_name || !path)
        return -1;
    
    module_atom = atomspace_add_node(global_atomspace, ATOM_MODULE,
                                    module_name);
    if (!module_atom)
        return -1;
    
    /* Store path in rsync_data */
    module_atom->rsync_data = strdup(path);
    if (module_atom->rsync_data)
        module_atom->rsync_data_size = strlen(path) + 1;
    
    return 0;
}

/**
 * opencog_get_module_atom - Get module atom from AtomSpace
 * @module_name: Module name
 *
 * Returns: Module atom or NULL if not found
 */
struct atom *opencog_get_module_atom(const char *module_name)
{
    if (!opencog_enabled || !module_name)
        return NULL;
    
    return atomspace_find_node(global_atomspace, ATOM_MODULE, module_name);
}

/**
 * opencog_create_swarm - Create swarm formation
 * @swarm_name: Swarm identifier
 *
 * Returns: Swarm formation or NULL on failure
 */
struct swarm_formation *opencog_create_swarm(const char *swarm_name)
{
    if (!opencog_enabled || !swarm_name)
        return NULL;
    
    return swarm_create(agent_zero, global_atomspace, swarm_name);
}

/**
 * opencog_add_swarm_member - Add member to swarm
 * @swarm: Swarm formation
 * @module_name: Module name
 * @hostname: Member hostname
 * @port: Member port
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_add_swarm_member(struct swarm_formation *swarm,
                            const char *module_name,
                            const char *hostname,
                            int port)
{
    struct atom *module_atom;
    
    if (!swarm || !module_name || !hostname)
        return -1;
    
    module_atom = opencog_get_module_atom(module_name);
    if (!module_atom)
        return -1;
    
    return swarm_add_member(swarm, module_atom, hostname, port);
}

/**
 * opencog_activate_swarm - Activate swarm for operations
 * @swarm: Swarm formation
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_activate_swarm(struct swarm_formation *swarm)
{
    if (!swarm)
        return -1;
    
    return swarm_activate(swarm);
}

/**
 * opencog_authenticate_user - Authenticate user via cognitive agent
 * @user: Username
 * @challenge: Auth challenge
 * @response: Auth response
 *
 * Returns: 1 if authenticated, 0 if not, -1 on error
 */
int opencog_authenticate_user(const char *user, const char *challenge,
                             const char *response)
{
    /* Placeholder for cognitive authentication */
    /* TODO: Implement PLN-based trust reasoning */
    
    if (!opencog_enabled || !agent_auth)
        return -1;
    
    /* Default to standard authentication for now */
    return -1;
}

/**
 * opencog_log_sync_start - Log sync start to monitoring agent
 * @module: Module name
 * @user: Username
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_log_sync_start(const char *module, const char *user)
{
    struct atom *module_atom;
    
    if (!opencog_enabled || !module || !user)
        return -1;
    
    module_atom = opencog_get_module_atom(module);
    if (module_atom) {
        /* Increase short-term importance */
        atom_set_sti(module_atom, module_atom->av.sti + 10);
    }
    
    return 0;
}

/**
 * opencog_log_sync_complete - Log sync completion
 * @module: Module name
 * @bytes: Bytes transferred
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_log_sync_complete(const char *module, uint64_t bytes)
{
    struct atom *module_atom;
    
    if (!opencog_enabled || !module)
        return -1;
    
    module_atom = opencog_get_module_atom(module);
    if (module_atom) {
        /* Update truth value based on success */
        atom_set_tv(module_atom, 1.0, 0.95);
        
        /* Increase long-term importance for successful syncs */
        atom_set_lti(module_atom, module_atom->av.lti + 1);
    }
    
    return 0;
}

/**
 * opencog_log_sync_error - Log sync error
 * @module: Module name
 * @error: Error message
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_log_sync_error(const char *module, const char *error)
{
    struct atom *module_atom;
    
    if (!opencog_enabled || !module)
        return -1;
    
    module_atom = opencog_get_module_atom(module);
    if (module_atom) {
        /* Decrease truth value on error */
        float new_strength = module_atom->tv.strength * 0.9;
        atom_set_tv(module_atom, new_strength, module_atom->tv.confidence);
    }
    
    return 0;
}

/**
 * opencog_generate_rsyncd_conf - Generate config via agent zero
 * @output_path: Output file path
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_generate_rsyncd_conf(const char *output_path)
{
    if (!opencog_enabled || !agent_zero || !output_path)
        return -1;
    
    /* Initialize agent zero with output path */
    if (agent_zero_init(agent_zero, global_atomspace, output_path) != 0)
        return -1;
    
    /* Generate configuration */
    return agent_zero_generate_config(agent_zero);
}

/**
 * opencog_agent_zero_add_module - Add module via agent zero
 * @module_name: Module name
 * @path: Module path
 * @read_only: Read-only flag
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_agent_zero_add_module(const char *module_name,
                                 const char *path,
                                 int read_only)
{
    if (!opencog_enabled || !agent_zero)
        return -1;
    
    if (!agent_zero->agent_data) {
        /* Initialize agent zero with default path */
        if (agent_zero_init(agent_zero, global_atomspace,
                          "/etc/rsyncd.conf") != 0)
            return -1;
    }
    
    return agent_zero_add_module(agent_zero, module_name, path, read_only)
           ? 0 : -1;
}

/**
 * opencog_agent_zero_add_swarm - Add swarm via agent zero
 * @swarm_name: Swarm name
 * @members: Array of member module names
 * @member_count: Number of members
 *
 * Returns: 0 on success, -1 on failure
 */
int opencog_agent_zero_add_swarm(const char *swarm_name,
                                const char **members,
                                size_t member_count)
{
    if (!opencog_enabled || !agent_zero)
        return -1;
    
    if (!agent_zero->agent_data) {
        /* Initialize agent zero with default path */
        if (agent_zero_init(agent_zero, global_atomspace,
                          "/etc/rsyncd.conf") != 0)
            return -1;
    }
    
    return agent_zero_add_swarm(agent_zero, swarm_name, members,
                               member_count);
}
