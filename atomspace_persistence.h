/*
 * AtomSpace Persistence - Save/Load AtomSpace and Learning History
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ATOMSPACE_PERSISTENCE_H
#define ATOMSPACE_PERSISTENCE_H

#include "atomspace.h"
#include "learning_module.h"
#include <stdio.h>

/**
 * Persistence configuration
 */
struct persistence_config {
    const char *atomspace_path;     /* Path to AtomSpace save file */
    const char *learning_path;      /* Path to learning history file */
    int auto_save_enabled;          /* Enable automatic periodic saves */
    time_t auto_save_interval;      /* Auto-save interval in seconds */
    int compression_enabled;        /* Enable compression */
};

/**
 * AtomSpace Persistence Functions
 */

/**
 * atomspace_save - Save AtomSpace to file
 * @atomspace: AtomSpace to save
 * @filename: Output file path
 *
 * Returns: 0 on success, -1 on failure
 */
int atomspace_save(struct atom_space *atomspace, const char *filename);

/**
 * atomspace_load - Load AtomSpace from file
 * @filename: Input file path
 *
 * Returns: Loaded AtomSpace or NULL on failure
 */
struct atom_space *atomspace_load(const char *filename);

/**
 * atomspace_save_binary - Save AtomSpace in binary format
 * @atomspace: AtomSpace to save
 * @fp: Output file pointer
 *
 * Returns: Number of atoms saved, -1 on failure
 */
int atomspace_save_binary(struct atom_space *atomspace, FILE *fp);

/**
 * atomspace_load_binary - Load AtomSpace from binary format
 * @fp: Input file pointer
 *
 * Returns: Loaded AtomSpace or NULL on failure
 */
struct atom_space *atomspace_load_binary(FILE *fp);

/**
 * atomspace_export_json - Export AtomSpace to JSON format
 * @atomspace: AtomSpace to export
 * @filename: Output JSON file path
 *
 * Returns: 0 on success, -1 on failure
 */
int atomspace_export_json(struct atom_space *atomspace, const char *filename);

/**
 * Learning History Persistence Functions
 */

/**
 * learning_history_save - Save learning history to file
 * @ctx: Learning context
 * @filename: Output file path
 *
 * Returns: Number of events saved, -1 on failure
 */
int learning_history_save(struct learning_context *ctx, const char *filename);

/**
 * learning_history_load - Load learning history from file
 * @ctx: Learning context
 * @filename: Input file path
 *
 * Returns: Number of events loaded, -1 on failure
 */
int learning_history_load(struct learning_context *ctx, const char *filename);

/**
 * Auto-save Management
 */

/**
 * persistence_init - Initialize persistence subsystem
 * @config: Persistence configuration
 *
 * Returns: 0 on success, -1 on failure
 */
int persistence_init(struct persistence_config *config);

/**
 * persistence_auto_save - Trigger auto-save if needed
 * @atomspace: AtomSpace to save
 * @learning_ctx: Learning context to save
 *
 * Returns: 1 if saved, 0 if not needed, -1 on failure
 */
int persistence_auto_save(struct atom_space *atomspace,
                         struct learning_context *learning_ctx);

/**
 * persistence_shutdown - Shutdown persistence and perform final save
 * @atomspace: AtomSpace to save
 * @learning_ctx: Learning context to save
 *
 * Returns: 0 on success, -1 on failure
 */
int persistence_shutdown(struct atom_space *atomspace,
                        struct learning_context *learning_ctx);

#endif /* ATOMSPACE_PERSISTENCE_H */
