/*
 * Agent Zero Header - Internal API
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef AGENT_ZERO_H
#define AGENT_ZERO_H

#include "cogagent.h"
#include "atomspace.h"

int agent_zero_init(struct cog_agent *agent, struct atom_space *atomspace,
                   const char *config_path);
int agent_zero_generate_config(struct cog_agent *agent);
struct atom *agent_zero_add_module(struct cog_agent *agent,
                                  const char *module_name,
                                  const char *path,
                                  int read_only);
int agent_zero_add_swarm(struct cog_agent *agent, const char *swarm_name,
                        const char **members, size_t member_count);
int agent_zero_task_handler(struct cog_agent *agent, void *task_data);

#endif /* AGENT_ZERO_H */
