/*
 * OpenCog Integration Header for rsync
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef OPENCOG_RSYNC_H
#define OPENCOG_RSYNC_H

#include "cogagent.h"
#include "atomspace.h"
#include "swarm_sync.h"

/* OpenCog integration flag */
extern int opencog_enabled;

/* Global cognitive infrastructure */
extern struct atom_space *global_atomspace;
extern struct cog_agent *agent_zero;
extern struct cog_agent *agent_monitor;
extern struct cog_agent *agent_auth;

/**
 * Initialization and Shutdown
 */
int opencog_init(void);
void opencog_shutdown(void);

/**
 * Daemon Integration
 */
int opencog_daemon_init(const char *config_file);
int opencog_daemon_start(void);
void opencog_daemon_stop(void);

/**
 * Module Management via AtomSpace
 */
int opencog_register_module(const char *module_name, const char *path);
int opencog_unregister_module(const char *module_name);
struct atom *opencog_get_module_atom(const char *module_name);

/**
 * Swarm Formation Management
 */
struct swarm_formation *opencog_create_swarm(const char *swarm_name);
int opencog_add_swarm_member(struct swarm_formation *swarm,
                            const char *module_name,
                            const char *hostname,
                            int port);
int opencog_activate_swarm(struct swarm_formation *swarm);

/**
 * Authentication via Cognitive Agents
 */
int opencog_authenticate_user(const char *user, const char *challenge,
                             const char *response);
int opencog_check_access(const char *user, const char *module);

/**
 * Monitoring and Feedback
 */
int opencog_log_sync_start(const char *module, const char *user);
int opencog_log_sync_complete(const char *module, uint64_t bytes);
int opencog_log_sync_error(const char *module, const char *error);

/**
 * Configuration Generation
 */
int opencog_generate_rsyncd_conf(const char *output_path);

/**
 * Agent Zero Interface
 */
int opencog_agent_zero_add_module(const char *module_name,
                                 const char *path,
                                 int read_only);
int opencog_agent_zero_add_swarm(const char *swarm_name,
                                const char **members,
                                size_t member_count);

#endif /* OPENCOG_RSYNC_H */
