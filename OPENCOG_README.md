# OpenCog Autonomous Multi-Agent Orchestration for rsync

This implementation adds OpenCog cognitive architecture capabilities to rsync, enabling autonomous multi-agent orchestration for distributed AtomSpace cognitive architectures with HyperGNN swarm sync formations.

## Overview

The integration provides:

1. **Cognitive Agent Infrastructure** - Multi-agent system with specialized agents for orchestration, monitoring, and authentication
2. **AtomSpace Hypergraph** - Distributed knowledge representation using hypergraph data structures
3. **Agent Zero** - Master builder that autonomously generates rsyncd.conf configurations based on cognitive topology
4. **HyperGNN Swarm Sync** - Swarm formation management for coordinated distributed synchronization
5. **Attention Allocation (ECAN)** - Economic attention allocation for priority-based resource management
6. **Probabilistic Logic (PLN)** - Truth values and confidence tracking for cognitive reasoning

## Architecture Components

### Core Headers

- `cogagent.h` - Cognitive agent interface and message passing
- `atomspace.h` - AtomSpace hypergraph data structures  
- `swarm_sync.h` - HyperGNN swarm formation management
- `agent_zero.h` - Agent Zero internal API
- `opencog_rsync.h` - Integration layer with rsync

### Implementation Files

- `cogagent.c` - Agent lifecycle, communication, and task processing
- `atomspace.c` - Hypergraph operations, truth values, attention allocation
- `agent_zero.c` - Autonomous rsyncd.conf generation
- `swarm_sync.c` - Swarm coordination and health monitoring
- `opencog_rsync.c` - Integration glue and global state management

### Example Program

- `opencog_example.c` - Demonstrates the complete workflow

## Key Features

### 1. Agent Types

- **COG_AGENT_ZERO**: Master builder and configuration orchestrator
- **COG_AGENT_MONITOR**: Monitoring and feedback agent
- **COG_AGENT_AUTH**: Authentication and security agent
- **COG_AGENT_SYNC**: Sync coordination agent
- **COG_AGENT_SWARM**: HyperGNN swarm coordination
- **COG_AGENT_HYPERGRAPH**: AtomSpace hypergraph manager

### 2. AtomSpace Nodes

- **ATOM_MODULE**: rsync module representation
- **ATOM_RSYNC_DAEMON**: rsync daemon instance
- **ATOM_SYNC_PATH**: Sync path node
- **ATOM_HOST**: Host/server node
- **ATOM_SWARM**: Swarm formation node

### 3. Truth Values

Each atom has associated probabilistic logic values:
- **Strength**: Probability [0.0, 1.0]
- **Confidence**: Confidence in the value [0.0, 1.0]

### 4. Attention Values (ECAN)

Economic attention allocation for resource prioritization:
- **STI**: Short-term importance
- **LTI**: Long-term importance
- **VLTI**: Very long-term importance

### 5. Swarm States

HyperGNN swarm formations progress through states:
- SWARM_FORMING → SWARM_ACTIVE → SWARM_COORDINATING → SWARM_IDLE

## Building

```bash
./configure --disable-md2man --disable-xxhash
make
make opencog-example
```

## Usage Example

```bash
./opencog_example
```

This will:
1. Initialize OpenCog cognitive infrastructure
2. Create AtomSpace with module atoms
3. Form HyperGNN swarm formations
4. Generate `rsyncd.conf.generated` configuration
5. Simulate cognitive sync operations
6. Display AtomSpace statistics and swarm health
7. Clean up resources

## Generated Configuration

Agent Zero generates rsyncd.conf with:
- Global daemon parameters
- Module definitions from AtomSpace
- Swarm formation modules
- Embedded cognitive metadata (AtomSpace handles, truth values, attention values)

Example output in `rsyncd.conf.generated`:

```ini
[data_module]
    comment = Cognitive module managed by Agent Zero
    path = /var/rsync/data_module
    # AtomSpace Handle: 1
    # Truth Value: strength=1.000 confidence=0.900
    # Attention: STI=100 LTI=0
```

## Integration with rsync Daemon

The cognitive modules are compiled into rsync and can be activated by:

1. Setting `OPENCOG_ENABLED` flag (future enhancement)
2. Calling `opencog_init()` in daemon mode
3. Using `opencog_daemon_init()` with rsyncd.conf path
4. Registering modules via `opencog_register_module()`

## API Functions

### Initialization

```c
int opencog_init(void);
void opencog_shutdown(void);
int opencog_daemon_init(const char *config_file);
```

### Module Management

```c
int opencog_register_module(const char *module_name, const char *path);
struct atom *opencog_get_module_atom(const char *module_name);
```

### Swarm Formation

```c
struct swarm_formation *opencog_create_swarm(const char *swarm_name);
int opencog_add_swarm_member(struct swarm_formation *swarm,
                            const char *module_name,
                            const char *hostname,
                            int port);
int opencog_activate_swarm(struct swarm_formation *swarm);
```

### Monitoring

```c
int opencog_log_sync_start(const char *module, const char *user);
int opencog_log_sync_complete(const char *module, uint64_t bytes);
int opencog_log_sync_error(const char *module, const char *error);
```

### Configuration Generation

```c
int opencog_generate_rsyncd_conf(const char *output_path);
int opencog_agent_zero_add_module(const char *module_name,
                                 const char *path,
                                 int read_only);
int opencog_agent_zero_add_swarm(const char *swarm_name,
                                const char **members,
                                size_t member_count);
```

## Architecture Diagram

```
┌─────────────────────────────────────────────────┐
│          OpenCog Cognitive Layer                │
├─────────────────────────────────────────────────┤
│  Agent Zero    │  Agent Monitor  │  Agent Auth  │
│  (Orchestrate) │  (Feedback)     │  (Security)  │
└────────┬────────┴────────┬─────────┴─────┬──────┘
         │                 │               │
         v                 v               v
    ┌────────────────────────────────────────┐
    │         AtomSpace Hypergraph           │
    │  ┌──────┐  ┌──────┐  ┌──────┐         │
    │  │Module│  │Swarm │  │ Host │         │
    │  │Atoms │  │Atoms │  │Atoms │         │
    │  └──┬───┘  └───┬──┘  └───┬──┘         │
    │     └──────────┼─────────┘            │
    │            Links & TV/AV               │
    └────────────────┬───────────────────────┘
                     │
         ┌───────────┴──────────────┐
         │   HyperGNN Swarm Sync    │
         │  ┌────────────────────┐  │
         │  │ Swarm Formations   │  │
         │  │ Health Monitoring  │  │
         │  │ Sync Coordination  │  │
         │  └────────────────────┘  │
         └──────────┬────────────────┘
                    │
         ┌──────────┴──────────┐
         │   rsync Daemon      │
         │   (clientserver.c)  │
         └─────────────────────┘
```

## Cognitive Workflow

1. **Initialization**: Create AtomSpace and agents
2. **Topology Building**: Add modules/hosts as atoms with truth values
3. **Swarm Formation**: Group modules into coordinated swarms
4. **Configuration Generation**: Agent Zero builds rsyncd.conf from AtomSpace
5. **Sync Operations**: Track success/failure via attention allocation
6. **Feedback Loop**: Update truth values and STI/LTI based on performance
7. **Autonomous Adaptation**: High-importance modules get priority

## Future Enhancements

### Implemented Features

The following future enhancements have now been implemented:

#### 1. PLN (Probabilistic Logic Networks) Inference

**Module**: `pln_inference.h`, `pln_inference.c`

Probabilistic logic networks for reasoning about sync patterns:
- **Deduction Rule**: Transitive inference (A→B, B→C ⇒ A→C)
- **Revision Rule**: Merge conflicting truth values from multiple observations
- **Prediction**: Predict sync success probability based on historical patterns
- **Pattern Discovery**: Automatically discover sync patterns from AtomSpace
- **Optimal Scheduling**: Infer optimal sync intervals based on importance and reliability

**Key Functions**:
```c
struct pln_inference_context *pln_context_create(struct atom_space *atomspace);
struct truth_value pln_predict_sync_success(struct pln_inference_context *ctx,
                                           struct atom *module,
                                           struct atom *host);
int pln_infer_sync_patterns(struct pln_inference_context *ctx,
                           struct sync_pattern **patterns,
                           size_t max_patterns);
time_t pln_infer_optimal_schedule(struct pln_inference_context *ctx,
                                 struct atom *module);
```

#### 2. Learning Module

**Module**: `learning_module.h`, `learning_module.c`

Experience-based learning by tracking historical sync success rates:
- **Sync History**: Record every sync event with metrics (success, duration, bytes)
- **Truth Value Updates**: Automatically update atom beliefs from observations
- **Temporal Patterns**: Discover time-based patterns (hour of day, day of week)
- **Attention Adaptation**: Adjust importance based on performance
- **Statistics Tracking**: Monitor learning accuracy and patterns discovered

**Key Functions**:
```c
struct learning_context *learning_context_create(struct atom_space *atomspace,
                                                struct pln_inference_context *pln_ctx);
int learning_record_sync(struct learning_context *ctx,
                        const char *module_name,
                        const char *host_name,
                        int success,
                        uint64_t bytes,
                        time_t duration);
int learning_update_truth_values(struct learning_context *ctx);
float learning_get_success_rate(struct learning_context *ctx,
                               const char *module_name);
```

#### 3. Dynamic Reconfiguration

**Module**: `dynamic_reconfig.h`, `dynamic_reconfig.c`

Agent Zero autonomously regenerates configuration based on changing conditions:
- **Auto-Reconfiguration**: Trigger based on performance thresholds
- **Topology Optimization**: Remove stale, low-importance atoms
- **Module Adaptation**: Adapt configurations based on learned patterns
- **Priority Updates**: Update module priorities based on importance
- **Event Tracking**: Record reconfiguration history and metrics

**Key Functions**:
```c
struct reconfig_context *reconfig_context_create(struct cog_agent *agent_zero,
                                                 struct atom_space *atomspace,
                                                 struct learning_context *learning_ctx);
int reconfig_enable_auto(struct reconfig_context *ctx,
                        int enabled,
                        float threshold,
                        time_t min_interval);
int reconfig_generate(struct reconfig_context *ctx, const char *output_path);
int reconfig_optimize_topology(struct reconfig_context *ctx);
```

#### 4. Distributed AtomSpace

**Module**: `distributed_atomspace.h`, `distributed_atomspace.c`

Synchronize AtomSpace itself across distributed nodes for true distributed cognition:
- **Node Network**: Manage multiple remote AtomSpace nodes
- **Full Sync**: Complete AtomSpace synchronization across nodes
- **Incremental Sync**: Sync only changes since last sync
- **Conflict Resolution**: Multiple strategies (latest wins, merge TV, highest confidence)
- **Remote Mirrors**: Maintain local mirrors of remote AtomSpaces
- **Subscription**: Subscribe to atom updates from remote nodes

**Key Functions**:
```c
struct distributed_atomspace *distributed_atomspace_create(
    struct atom_space *local_atomspace);
uint64_t distributed_atomspace_add_node(struct distributed_atomspace *das,
                                       const char *hostname,
                                       int port);
int distributed_atomspace_sync_full(struct distributed_atomspace *das,
                                   uint64_t node_id);
int distributed_atomspace_set_conflict_strategy(
    struct distributed_atomspace *das,
    conflict_strategy strategy);
```

#### 5. GGML Integration

**Module**: `ggml_hypergraph.h`, `ggml_hypergraph.c`

Tensor-based operations for efficient hypergraph processing:
- **Atom Embeddings**: Dense vector representations of atoms
- **Adjacency Matrices**: Tensor representation of hypergraph structure
- **Fast Similarity**: Tensor dot product for similarity computation
- **Attention Propagation**: Spread attention using matrix operations
- **Clustering**: Tensor-based clustering of related atoms

**Note**: Currently stub implementation. Requires linking with GGML library for full functionality.

**Key Functions**:
```c
struct ggml_hypergraph_context *ggml_hypergraph_create(
    struct atom_space *atomspace,
    size_t embedding_dim);
int ggml_hypergraph_build_embeddings(struct ggml_hypergraph_context *ctx);
float ggml_hypergraph_compute_similarity(struct ggml_hypergraph_context *ctx,
                                        struct atom *atom1,
                                        struct atom *atom2);
```

#### 6. llama.cpp Backend

**Module**: `llama_pattern.h`, `llama_pattern.c`

Neural network integration for advanced pattern recognition:
- **Success Prediction**: Neural network-based sync success prediction
- **Failure Classification**: Classify types of sync failures
- **Schedule Generation**: AI-generated optimal sync schedules
- **Anomaly Detection**: Detect unusual sync patterns
- **Model Training**: Fine-tune on historical sync data

**Note**: Currently stub implementation. Requires linking with llama.cpp library for full functionality.

**Key Functions**:
```c
struct llama_pattern_context *llama_pattern_create(
    struct atom_space *atomspace,
    struct learning_context *learning_ctx,
    const char *model_path);
struct pattern_result llama_pattern_predict_success(
    struct llama_pattern_context *ctx,
    const char *module_name,
    const char *host_name);
int llama_pattern_train(struct llama_pattern_context *ctx, int epochs);
```

### Building with Future Enhancements

```bash
./configure --disable-md2man --disable-xxhash
make
make opencog-future-demo
```

### Running the Demo

```bash
./opencog_future_demo
```

The demo showcases:
1. PLN inference for pattern reasoning
2. Learning from simulated sync observations
3. Dynamic reconfiguration based on performance
4. Distributed AtomSpace synchronization
5. GGML tensor operations (stub)
6. llama.cpp pattern recognition (stub)

### Configuration Files Generated

- `rsyncd.conf.generated` - Original example configuration
- `rsyncd.conf.enhanced` - Enhanced configuration with learned optimizations

### Integration Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  Cognitive Layer                        │
├─────────────────────────────────────────────────────────┤
│  llama.cpp Pattern Recognition (Neural)                 │
│  ↕                                                       │
│  PLN Inference Engine (Symbolic)                        │
│  ↕                                                       │
│  Learning Module (Experience-Based)                     │
│  ↕                                                       │
│  Dynamic Reconfiguration (Adaptive)                     │
├─────────────────────────────────────────────────────────┤
│           AtomSpace Hypergraph                          │
│  ← GGML Tensor Ops → ← Distributed Sync →              │
├─────────────────────────────────────────────────────────┤
│  Agent Zero │ Agent Monitor │ Agent Auth │ Agent Swarm │
├─────────────────────────────────────────────────────────┤
│              rsync Daemon (clientserver.c)              │
└─────────────────────────────────────────────────────────┘
```

### Cognitive Synergy

The future enhancements embody CogPrime's principle of **cognitive synergy**:

- **PLN + Learning**: Symbolic reasoning enhanced by statistical learning
- **Learning + Reconfiguration**: Learned patterns drive adaptive behavior
- **Reconfiguration + Distributed**: Adaptations propagate across network
- **GGML + llama.cpp**: Efficient tensor ops support neural processing
- **All layers**: Each component assists others in overcoming bottlenecks

This integration demonstrates how multiple cognitive processes working together 
create intelligence greater than the sum of parts.

## License

GNU General Public License v3.0 - same as rsync

## Copyright

Copyright (C) 2025 OpenCog Cognitive Architecture Project

Based on rsync by Andrew Tridgell, Wayne Davison, and others.
