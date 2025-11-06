# RsyncOG Formal Documentation

This directory contains comprehensive technical architecture documentation and formal Z++ specifications for the RsyncOG cognitive architecture system.

## Overview

RsyncOG extends rsync with OpenCog cognitive architecture capabilities, enabling autonomous multi-agent orchestration for distributed synchronization with hypergraph knowledge representation, probabilistic reasoning, and experience-based learning.

## Documents

### Architecture Documentation

**[architecture_overview.md](architecture_overview.md)** - Comprehensive technical architecture overview with Mermaid diagrams

This document provides:
- High-level system architecture diagrams
- Component interaction flows
- Agent communication protocols
- AtomSpace hypergraph structure
- Swarm formation lifecycle
- Distributed AtomSpace topology
- Data flow diagrams
- Technology stack details
- Integration boundaries

The architecture overview uses Mermaid diagrams for visual representation of:
- System component layers
- Agent types and capabilities
- Sequence diagrams for cognitive workflows
- State machines for agents and swarms
- Network topology for distributed deployment

### Z++ Formal Specifications

The formal specifications are written in Z++ (an extension of Z notation) and provide mathematically rigorous definitions of the system's data structures, state, operations, and integration contracts.

#### [data_model.zpp](data_model.zpp) - Data Layer Formalization

Formalizes the core data structures:
- **Basic Types**: Handles, timestamps, enumerations
- **Truth Values**: Probabilistic logic strength and confidence
- **Attention Values**: Economic attention allocation (STI/LTI/VLTI)
- **Atoms**: Hypergraph nodes representing concepts and entities
  - ModuleAtom, HostAtom, SwarmAtom specializations
- **Links**: Hypergraph edges connecting atoms
  - SyncTopologyLink, SwarmMemberLink, AuthTrustLink
- **AtomSpace**: Complete hypergraph knowledge base
- **Cognitive Agents**: Multi-agent system components
- **Swarm Formations**: Coordinated sync topologies
- **PLN Structures**: Sync patterns and inference data
- **Distributed Structures**: Network nodes and sync state

Each schema includes:
- State variables with types
- Invariants that must hold
- Relationships between components

#### [system_state.zpp](system_state.zpp) - System State Formalization

Defines the overall system state and global invariants:
- **SystemState**: Top-level schema encompassing all subsystems
  - AtomSpace, Agent system, Swarms, PLN, Learning, Distributed
- **PLN Inference Context**: Probabilistic logic networks state
- **Learning Context**: Experience-based learning state
- **Reconfiguration Context**: Dynamic adaptation state
- **Production Monitor**: Metrics and health monitoring
- **State Transitions**: Abstract schemas for state changes
- **Health Checks**: System health and consistency predicates

Global invariants ensure:
- Agent Zero uniqueness
- Shared AtomSpace references
- Referential integrity across subsystems
- Temporal consistency
- Resource bounds

#### [operations.zpp](operations.zpp) - Operation Specifications

Formalizes all state-modifying operations:
- **System Initialization**: InitializeSystem, StartSystem, ShutdownSystem
- **AtomSpace Operations**: AddModuleAtom, AddSyncTopologyLink, UpdateTruthValue, UpdateAttentionValue
- **Swarm Operations**: CreateSwarmFormation, ActivateSwarm
- **Sync Operations**: InitiateSync, RecordSyncSuccess, RecordSyncFailure
- **PLN Inference**: ApplyPLNDeduction, DiscoverSyncPattern
- **Dynamic Reconfiguration**: TriggerAutoReconfiguration, GenerateConfiguration
- **Distributed Sync**: SyncDistributedAtomSpace

Each operation schema specifies:
- Pre-conditions (required state before operation)
- Post-conditions (guaranteed state after operation)
- State changes (Δ notation for before/after)
- Inputs (? notation) and outputs (! notation)

#### [integrations.zpp](integrations.zpp) - External Integration Contracts

Specifies contracts for external system integrations:
- **Rsync Protocol**: RsyncDaemonConfig, ModuleConfig, ProcessRsyncRequest
- **Distributed Communication**: NetworkMessage, AtomSyncPayload, node synchronization
- **Monitoring**: PrometheusMetrics, ExportPrometheusMetrics
- **Authentication**: AuthenticateClient with cognitive trust assessment
- **Optional Backends**: GGML embeddings, llama.cpp patterns (stubs)
- **Configuration Generation**: ConfigFileContract for rsyncd.conf

Integration contracts ensure:
- Protocol compliance
- Data format validity
- Error handling
- Security constraints
- Performance bounds

## Reading the Specifications

### Z++ Notation Guide

The specifications use standard Z notation extended for clarity:

**Schema Boxes:**
```
┌─ SchemaName ───────────────────────────────────────┐
│ variable_name : Type                               │
│ another_var   : AnotherType                        │
├────────────────────────────────────────────────────┤
│ /* Predicates (invariants) */                      │
│ variable_name ≥ 0                                  │
│ another_var ≠ ∅                                    │
└────────────────────────────────────────────────────┘
```

**Common Symbols:**
- `ℕ` - Natural numbers (0, 1, 2, ...)
- `ℤ` - Integers (..., -1, 0, 1, ...)
- `ℝ` - Real numbers
- `𝔹` - Boolean (true/false)
- `⇸` - Partial function
- `→` - Total function
- `ℙ(X)` - Power set of X (all subsets)
- `seq X` - Sequence of X
- `⟨⟩` - Empty sequence
- `⟨a, b, c⟩` - Sequence containing a, b, c
- `#s` - Size/cardinality of set or sequence s
- `∅` - Empty set
- `∈` - Element of
- `∉` - Not element of
- `⊆` - Subset of
- `∪` - Union
- `∩` - Intersection
- `∧` - Logical AND
- `∨` - Logical OR
- `¬` - Logical NOT
- `⇒` - Implication
- `⇔` - If and only if
- `∀` - For all (universal quantifier)
- `∃` - There exists (existential quantifier)

**State Change Notation:**
- `ΔSchemaName` - Schema changes (before and after state)
- `ΞSchemaName` - Schema doesn't change (read-only)
- `variable'` - After-state of variable
- `variable` - Before-state of variable
- `input?` - Input parameter
- `output!` - Output parameter

**Domain and Range:**
- `dom f` - Domain of function f (input values)
- `ran f` - Range of function f (output values)

### How to Use These Specifications

1. **Understanding System Structure**: Start with `data_model.zpp` to understand the core data structures and their invariants.

2. **Understanding System State**: Read `system_state.zpp` to see how components fit together and what global invariants must hold.

3. **Understanding Behavior**: Study `operations.zpp` to see how the system changes state through operations.

4. **Understanding Integrations**: Review `integrations.zpp` to understand external system contracts.

5. **Implementation Verification**: When implementing or modifying code:
   - Check that data structures match the formal schemas
   - Verify that operations preserve all invariants
   - Ensure pre-conditions are checked before operations
   - Confirm post-conditions are achieved after operations

6. **Testing**: Use the specifications to:
   - Generate test cases from pre/post-conditions
   - Verify invariants in unit tests
   - Create integration test scenarios
   - Validate edge cases and error conditions

## Relationship to Implementation

The specifications map to the C implementation as follows:

### File Mappings

| Specification | C Header | C Implementation |
|--------------|----------|------------------|
| TruthValue, AttentionValue, Atom, AtomLink | `atomspace.h` | `atomspace.c` |
| CogAgent, CogMessage | `cogagent.h` | `cogagent.c` |
| SwarmFormation, SwarmMember | `swarm_sync.h` | `swarm_sync.c` |
| PLNInferenceContext, SyncPattern | `pln_inference.h` | `pln_inference.c` |
| LearningContext, SyncHistoryEntry | `learning_module.h` | `learning_module.c` |
| ReconfigContext | `dynamic_reconfig.h` | `dynamic_reconfig.c` |
| DistributedAtomSpace, AtomSpaceNode | `distributed_atomspace.h` | `distributed_atomspace.c` |
| ProductionMonitor | `production_monitor.h` | `production_monitor.c` |

### Type Mappings

| Z++ Type | C Type |
|----------|--------|
| HANDLE | `uint64_t` |
| TIMESTAMP | `time_t` |
| NAME | `char*` or `char[256]` |
| ℕ (Natural) | `uint64_t`, `size_t` |
| ℤ (Integer) | `int64_t`, `int16_t` |
| ℝ (Real) | `float`, `double` |
| 𝔹 (Boolean) | `int`, `bool` |
| seq X | Array or linked list in C |

### Verification

The specifications can be used for:
- **Static Analysis**: Tools can check that C code satisfies type constraints
- **Runtime Assertions**: Invariants can be encoded as assert() statements
- **Model Checking**: Formal verification tools can verify properties
- **Property-Based Testing**: Generate test cases from specifications

## Benefits of Formal Specification

1. **Precise Documentation**: No ambiguity in system behavior
2. **Design Validation**: Catch logical errors before implementation
3. **Implementation Guide**: Clear contract for what code must do
4. **Test Generation**: Systematic test case derivation
5. **Refactoring Safety**: Verify that changes preserve invariants
6. **Communication**: Unambiguous language for discussing system
7. **Verification**: Formal proofs of correctness possible

## Future Work

Potential extensions to the specifications:
- **Temporal Logic**: Specify timing and liveness properties
- **Concurrency**: Formalize thread safety and synchronization
- **Performance**: Specify algorithmic complexity bounds
- **Security**: Formalize security properties and threat models
- **Refinement**: Prove implementation refines specification

## References

- **Z Notation**: Spivey, J.M. "The Z Notation: A Reference Manual"
- **Formal Methods**: Woodcock, J., Davies, J. "Using Z: Specification, Refinement, and Proof"
- **OpenCog**: https://opencog.org
- **PLN**: Goertzel, B., et al. "Probabilistic Logic Networks"
- **ECAN**: Goertzel, B., et al. "Economic Attention Networks"

## License

Copyright (C) 2025 OpenCog Cognitive Architecture Project  
Licensed under GNU General Public License v3.0

---

**Generated**: November 2025  
**Version**: 1.0  
**Maintained by**: OpenCog Cognitive Architecture Project
