/*
 * GGML Integration - Tensor-Based Hypergraph Operations
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Integrates GGML (Georgi Gerganov's Machine Learning library) for
 * efficient tensor-based operations on hypergraph structures.
 */

#ifndef GGML_HYPERGRAPH_H
#define GGML_HYPERGRAPH_H

#include "atomspace.h"
#include <stdint.h>

/* Forward declarations */
struct ggml_context;
struct ggml_tensor;
struct ggml_hypergraph_context;

/**
 * GGML Hypergraph Context - Tensor operations on AtomSpace
 */
struct ggml_hypergraph_context {
    struct atom_space *atomspace;
    struct ggml_context *ggml_ctx;
    
    /* Tensor representations */
    struct ggml_tensor *atom_embeddings;     /* Atom feature vectors */
    struct ggml_tensor *link_adjacency;      /* Adjacency matrix */
    struct ggml_tensor *attention_weights;   /* Attention values */
    
    /* Dimensions */
    size_t embedding_dim;
    size_t max_atoms;
};

/**
 * GGML Hypergraph Functions
 */

/**
 * ggml_hypergraph_create - Create GGML hypergraph context
 * @atomspace: AtomSpace to operate on
 * @embedding_dim: Dimension of atom embeddings
 *
 * Returns: New context or NULL on failure
 */
struct ggml_hypergraph_context *ggml_hypergraph_create(
    struct atom_space *atomspace,
    size_t embedding_dim);

/**
 * ggml_hypergraph_destroy - Free GGML context
 * @ctx: Context to destroy
 */
void ggml_hypergraph_destroy(struct ggml_hypergraph_context *ctx);

/**
 * ggml_hypergraph_build_embeddings - Build tensor embeddings from atoms
 * @ctx: GGML context
 *
 * Returns: 0 on success, -1 on failure
 */
int ggml_hypergraph_build_embeddings(struct ggml_hypergraph_context *ctx);

/**
 * ggml_hypergraph_compute_similarity - Fast tensor-based similarity
 * @ctx: GGML context
 * @atom1: First atom
 * @atom2: Second atom
 *
 * Returns: Similarity score [0.0-1.0]
 */
float ggml_hypergraph_compute_similarity(struct ggml_hypergraph_context *ctx,
                                        struct atom *atom1,
                                        struct atom *atom2);

/**
 * ggml_hypergraph_propagate_attention - Spread attention using tensor ops
 * @ctx: GGML context
 * @steps: Number of propagation steps
 *
 * Returns: 0 on success, -1 on failure
 */
int ggml_hypergraph_propagate_attention(struct ggml_hypergraph_context *ctx,
                                       int steps);

/**
 * ggml_hypergraph_cluster - Cluster atoms using tensor operations
 * @ctx: GGML context
 * @num_clusters: Number of clusters to find
 *
 * Returns: Array of cluster assignments or NULL
 */
int *ggml_hypergraph_cluster(struct ggml_hypergraph_context *ctx,
                             size_t num_clusters);

#endif /* GGML_HYPERGRAPH_H */
