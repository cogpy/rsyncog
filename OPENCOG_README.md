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

- **PLN Inference**: Probabilistic logic networks for reasoning about sync patterns
- **Learning**: Update truth values based on historical sync success rates
- **Dynamic Reconfiguration**: Agent Zero regenerates config based on changing conditions
- **Distributed AtomSpace**: Sync AtomSpace itself across nodes for true distributed cognition
- **GGML Integration**: Tensor-based operations for hypergraph processing
- **llama.cpp Backend**: Neural network integration for pattern recognition

## License

GNU General Public License v3.0 - same as rsync

## Copyright

Copyright (C) 2025 OpenCog Cognitive Architecture Project

Based on rsync by Andrew Tridgell, Wayne Davison, and others.
