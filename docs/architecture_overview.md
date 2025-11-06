# RsyncOG: Cognitive Architecture System - Technical Architecture Overview

**Version:** 1.0  
**Date:** November 2025  
**Copyright:** 2025 OpenCog Cognitive Architecture Project

---

## Executive Summary

RsyncOG is a revolutionary extension of the rsync file synchronization utility that integrates OpenCog cognitive architecture capabilities. The system enables autonomous multi-agent orchestration for distributed AtomSpace cognitive architectures with HyperGNN swarm sync formations. This document provides a comprehensive technical architecture overview with formal specifications.

### Core Innovations

1. **Cognitive Knowledge Representation**: AtomSpace hypergraph for distributed sync topology
2. **Autonomous Multi-Agent System**: Specialized cognitive agents for orchestration, monitoring, and coordination
3. **Probabilistic Reasoning**: PLN (Probabilistic Logic Networks) for sync pattern inference
4. **Experience-Based Learning**: Historical performance tracking with adaptive behavior
5. **Distributed Cognition**: Synchronized AtomSpace across network nodes
6. **Swarm Intelligence**: HyperGNN swarm formations for coordinated synchronization

---

## 1. System Architecture Overview

### 1.1 High-Level Architecture

```mermaid
graph TB
    subgraph "Presentation Layer"
        CLI[CLI Interface]
        DAEMON[Daemon Mode]
        CONFIG[Configuration Files]
    end
    
    subgraph "Cognitive Layer"
        subgraph "Agent System"
            A0[Agent Zero<br/>Orchestrator]
            AM[Agent Monitor<br/>Feedback]
            AA[Agent Auth<br/>Security]
            AS[Agent Sync<br/>Coordination]
            ASW[Agent Swarm<br/>Swarm Control]
            AH[Agent Hypergraph<br/>Graph Manager]
        end
        
        subgraph "Reasoning Engines"
            PLN[PLN Inference<br/>Probabilistic Logic]
            LEARN[Learning Module<br/>Experience-Based]
            RECONFIG[Dynamic Reconfig<br/>Adaptation]
        end
        
        subgraph "Knowledge Base"
            AS_CORE[AtomSpace Core<br/>Hypergraph DB]
            AS_DIST[Distributed AtomSpace<br/>Cross-Node Sync]
            GGML[GGML Tensors<br/>Efficient Processing]
            LLAMA[llama.cpp<br/>Neural Patterns]
        end
    end
    
    subgraph "Coordination Layer"
        SWARM[Swarm Formations<br/>HyperGNN Coordination]
        PERSIST[AtomSpace Persistence<br/>Serialization]
        MONITOR[Production Monitor<br/>Metrics & Health]
    end
    
    subgraph "Core Rsync Layer"
        SYNC[Sync Engine]
        DELTA[Delta Algorithm]
        NETWORK[Network I/O]
    end
    
    CLI --> A0
    DAEMON --> A0
    CONFIG --> A0
    
    A0 --> PLN
    A0 --> AS_CORE
    AM --> LEARN
    AS --> SWARM
    ASW --> SWARM
    AH --> AS_CORE
    
    PLN --> AS_CORE
    LEARN --> AS_CORE
    RECONFIG --> A0
    
    AS_CORE --> AS_DIST
    AS_CORE --> GGML
    AS_CORE --> PERSIST
    
    SWARM --> SYNC
    AS_DIST --> NETWORK
    MONITOR --> SYNC
    
    SYNC --> DELTA
    DELTA --> NETWORK
    
    style A0 fill:#ff9999
    style AS_CORE fill:#99ccff
    style PLN fill:#99ff99
    style SWARM fill:#ffcc99
```

### 1.2 Component Layers

| Layer | Components | Responsibility |
|-------|-----------|----------------|
| **Presentation** | CLI, Daemon, Config | User interaction and initialization |
| **Cognitive** | Agents, PLN, Learning | Autonomous reasoning and decision-making |
| **Knowledge** | AtomSpace, Persistence | Distributed knowledge representation |
| **Coordination** | Swarm, Monitor | Multi-node orchestration |
| **Core** | Sync Engine, Delta, Network | File synchronization operations |

---

## 2. Cognitive Agent Architecture

### 2.1 Agent Interaction Diagram

```mermaid
sequenceDiagram
    participant User
    participant A0 as Agent Zero
    participant AS as AtomSpace
    participant AM as Agent Monitor
    participant ASW as Agent Swarm
    participant PLN as PLN Engine
    participant Sync as Sync Engine
    
    User->>A0: Initialize System
    A0->>AS: Create AtomSpace
    A0->>AM: Start Monitoring
    A0->>ASW: Initialize Swarms
    
    User->>A0: Register Module
    A0->>AS: Create Module Atom
    A0->>PLN: Set Initial TV
    AS-->>A0: Atom Handle
    
    A0->>ASW: Form Swarm
    ASW->>AS: Create Swarm Links
    ASW->>Sync: Initiate Sync
    
    Sync-->>AM: Sync Complete
    AM->>AS: Update Truth Values
    AM->>PLN: Record Pattern
    
    PLN->>AS: Infer Predictions
    PLN-->>A0: Optimization Suggestions
    
    A0->>A0: Generate Config
    A0-->>User: New Configuration
```

### 2.2 Agent Types and Capabilities

```mermaid
classDiagram
    class CogAgent {
        +uint64_t agent_id
        +cog_agent_type type
        +cog_agent_state state
        +char name[256]
        +uint32_t capabilities
        +AtomSpace* atomspace
        +start()
        +stop()
        +process_tasks()
    }
    
    class AgentZero {
        +COG_CAP_ORCHESTRATE
        +COG_CAP_BUILD_CONFIG
        +generate_config()
        +add_module()
        +coordinate_agents()
    }
    
    class AgentMonitor {
        +COG_CAP_MONITOR
        +track_performance()
        +update_metrics()
        +trigger_alarms()
    }
    
    class AgentAuth {
        +COG_CAP_AUTH
        +validate_credentials()
        +manage_trust()
        +update_security()
    }
    
    class AgentSwarm {
        +COG_CAP_SWARM
        +form_swarm()
        +coordinate_sync()
        +manage_topology()
    }
    
    class AgentHypergraph {
        +COG_CAP_HYPERGRAPH
        +manage_atoms()
        +optimize_structure()
        +propagate_attention()
    }
    
    CogAgent <|-- AgentZero
    CogAgent <|-- AgentMonitor
    CogAgent <|-- AgentAuth
    CogAgent <|-- AgentSwarm
    CogAgent <|-- AgentHypergraph
```

### 2.3 Agent Communication Protocol

```mermaid
stateDiagram-v2
    [*] --> Init: Create Agent
    Init --> Idle: Initialization Complete
    
    Idle --> Active: Receive Task
    Idle --> Coordinating: Receive Swarm Msg
    
    Active --> Processing: Execute Task
    Processing --> Active: Task Complete
    Processing --> Error: Task Failed
    
    Coordinating --> SendingMsg: Broadcast
    SendingMsg --> WaitingResponse: Await ACK
    WaitingResponse --> Coordinating: Response Received
    WaitingResponse --> Error: Timeout
    
    Active --> Idle: Return to Idle
    Coordinating --> Idle: Coordination Complete
    
    Error --> Idle: Recovery
    Error --> Shutdown: Critical Error
    
    Idle --> Shutdown: Stop Command
    Shutdown --> [*]
```

---

## 3. AtomSpace Hypergraph Architecture

### 3.1 AtomSpace Structure

```mermaid
graph LR
    subgraph "AtomSpace"
        subgraph "Atoms (Nodes)"
            A1[Module Atom<br/>data_module<br/>TV: 1.0/0.9]
            A2[Host Atom<br/>server1.example<br/>TV: 0.95/0.85]
            A3[Swarm Atom<br/>swarm_alpha<br/>TV: 0.98/0.92]
            A4[Daemon Atom<br/>rsyncd<br/>TV: 1.0/1.0]
        end
        
        subgraph "Links (Edges)"
            L1[Sync Topology Link<br/>Module -> Host]
            L2[Swarm Member Link<br/>Swarm -> Module]
            L3[Auth Trust Link<br/>Daemon -> Module]
        end
        
        subgraph "Cognitive Values"
            TV[Truth Values<br/>Strength/Confidence]
            AV[Attention Values<br/>STI/LTI/VLTI]
        end
    end
    
    A1 --> L1
    A2 --> L1
    A3 --> L2
    A1 --> L2
    A4 --> L3
    A1 --> L3
    
    L1 --> TV
    A1 --> AV
    
    style A1 fill:#ffcccc
    style A2 fill:#ccffcc
    style A3 fill:#ccccff
    style L1 fill:#ffffcc
```

### 3.2 Atom Type Hierarchy

```mermaid
graph TD
    ROOT[Atom Base]
    
    ROOT --> NODE[Node Types]
    ROOT --> LINK[Link Types]
    
    NODE --> CONCEPT[Concept Node]
    NODE --> DAEMON[Daemon Node]
    NODE --> MODULE[Module Node]
    NODE --> HOST[Host Node]
    NODE --> SWARM[Swarm Node]
    NODE --> PATH[Path Node]
    
    LINK --> INHERIT[Inheritance Link]
    LINK --> SIMILAR[Similarity Link]
    LINK --> SYNC_TOP[Sync Topology Link]
    LINK --> SWARM_MEM[Swarm Member Link]
    LINK --> AUTH[Auth Trust Link]
    LINK --> DEPEND[Dependency Link]
    
    style ROOT fill:#333,color:#fff
    style NODE fill:#99ccff
    style LINK fill:#ffcc99
```

### 3.3 Truth Value and Attention Mechanism

```mermaid
graph TB
    subgraph "Truth Value System"
        TV_STR[Strength: 0.0 - 1.0<br/>Probability of truth]
        TV_CONF[Confidence: 0.0 - 1.0<br/>Evidence weight]
        TV_CALC[TV Calculation<br/>Bayesian inference]
    end
    
    subgraph "Attention Allocation (ECAN)"
        STI[Short-Term Importance<br/>Recent activity]
        LTI[Long-Term Importance<br/>Historical relevance]
        VLTI[Very Long-Term Importance<br/>Structural significance]
        ECAN_UPDATE[ECAN Update Cycle<br/>Resource distribution]
    end
    
    subgraph "Cognitive Synergy"
        ATOM[Atom Instance]
        OPS[Operations on Atom]
        LEARN[Learning Feedback]
    end
    
    OPS --> TV_CALC
    TV_CALC --> TV_STR
    TV_CALC --> TV_CONF
    
    LEARN --> ECAN_UPDATE
    ECAN_UPDATE --> STI
    ECAN_UPDATE --> LTI
    ECAN_UPDATE --> VLTI
    
    TV_STR --> ATOM
    TV_CONF --> ATOM
    STI --> ATOM
    LTI --> ATOM
    VLTI --> ATOM
    
    ATOM --> OPS
    
    style TV_CALC fill:#99ff99
    style ECAN_UPDATE fill:#ff9999
```

---

## 4. Data Flow Architecture

### 4.1 Sync Operation Data Flow

```mermaid
flowchart TD
    START([User Initiates Sync])
    
    START --> REG[Agent Zero: Register Module]
    REG --> ATOM_CREATE[Create Module Atom in AtomSpace]
    ATOM_CREATE --> TV_INIT[Initialize Truth Value<br/>strength=1.0, confidence=0.9]
    TV_INIT --> AV_INIT[Initialize Attention Value<br/>STI=100]
    
    AV_INIT --> SWARM_CHECK{Swarm Formation?}
    SWARM_CHECK -->|Yes| SWARM_CREATE[Create Swarm Links]
    SWARM_CHECK -->|No| DIRECT_SYNC
    
    SWARM_CREATE --> SWARM_COORD[Agent Swarm: Coordinate]
    SWARM_COORD --> PLN_PREDICT[PLN: Predict Success]
    
    PLN_PREDICT --> DIRECT_SYNC[Initiate Rsync Operation]
    DIRECT_SYNC --> DELTA[Delta Transfer Algorithm]
    DELTA --> NETWORK[Network Transmission]
    
    NETWORK --> RESULT{Success?}
    RESULT -->|Success| UPDATE_SUCCESS[Agent Monitor: Update TV<br/>Increase strength/confidence]
    RESULT -->|Failure| UPDATE_FAIL[Agent Monitor: Update TV<br/>Decrease strength]
    
    UPDATE_SUCCESS --> LEARN_SUCCESS[Learning Module: Record Success]
    UPDATE_FAIL --> LEARN_FAIL[Learning Module: Record Failure]
    
    LEARN_SUCCESS --> RECONFIG_CHECK{Performance Threshold?}
    LEARN_FAIL --> RECONFIG_CHECK
    
    RECONFIG_CHECK -->|Below Threshold| TRIGGER_RECONFIG[Dynamic Reconfig:<br/>Regenerate Config]
    RECONFIG_CHECK -->|Above Threshold| PERSIST
    
    TRIGGER_RECONFIG --> PERSIST[Persist AtomSpace State]
    PERSIST --> DIST_SYNC{Distributed Mode?}
    
    DIST_SYNC -->|Yes| PROPAGATE[Distributed AtomSpace:<br/>Sync to Peers]
    DIST_SYNC -->|No| END
    
    PROPAGATE --> END([Sync Complete])
    
    style START fill:#99ff99
    style ATOM_CREATE fill:#99ccff
    style PLN_PREDICT fill:#ffcc99
    style LEARN_SUCCESS fill:#99ff99
    style LEARN_FAIL fill:#ff9999
    style END fill:#99ff99
```

### 4.2 PLN Inference Data Flow

```mermaid
flowchart LR
    subgraph "Input Layer"
        HISTORY[Sync History<br/>Success/Failure Records]
        ATOMSPACE[AtomSpace<br/>Current State]
        PATTERNS[Known Patterns]
    end
    
    subgraph "PLN Inference Engine"
        DEDUCTION[Deduction Rule<br/>A→B, B→C ⇒ A→C]
        REVISION[Revision Rule<br/>Merge conflicting TVs]
        ABDUCTION[Abduction Rule<br/>Infer causes]
        INDUCTION[Induction Rule<br/>Generalize patterns]
    end
    
    subgraph "Output Layer"
        PREDICTION[Success Prediction]
        SCHEDULE[Optimal Schedule]
        DISCOVERED[New Patterns]
        CONFIDENCE[Confidence Scores]
    end
    
    HISTORY --> DEDUCTION
    ATOMSPACE --> DEDUCTION
    PATTERNS --> INDUCTION
    
    DEDUCTION --> REVISION
    INDUCTION --> REVISION
    REVISION --> ABDUCTION
    
    ABDUCTION --> PREDICTION
    ABDUCTION --> SCHEDULE
    REVISION --> DISCOVERED
    DEDUCTION --> CONFIDENCE
    
    PREDICTION --> ATOMSPACE
    DISCOVERED --> PATTERNS
    
    style DEDUCTION fill:#99ccff
    style REVISION fill:#ffcc99
    style PREDICTION fill:#99ff99
```

### 4.3 Learning Module Workflow

```mermaid
sequenceDiagram
    participant Sync as Sync Engine
    participant Monitor as Agent Monitor
    participant Learning as Learning Module
    participant PLN as PLN Inference
    participant AS as AtomSpace
    
    Sync->>Monitor: Sync Event<br/>(success/failure)
    Monitor->>Learning: Record Observation
    
    Learning->>Learning: Create History Entry<br/>timestamp, metrics
    Learning->>Learning: Extract Temporal Context<br/>hour, day of week
    
    Learning->>PLN: Get Current Prediction
    PLN-->>Learning: Predicted TV
    
    Learning->>Learning: Compare Prediction vs Reality
    Learning->>Learning: Update Learning Stats
    
    Learning->>AS: Update Module Truth Value<br/>Bayesian update
    Learning->>AS: Adjust Attention Values<br/>Based on performance
    
    AS-->>Learning: Atoms Updated
    
    Learning->>PLN: Feed Pattern Discovery
    PLN->>PLN: Analyze New Pattern
    PLN-->>Learning: Pattern Confidence
    
    alt Pattern Confidence High
        Learning->>AS: Create Pattern Atom
        Learning->>Learning: Increment Patterns Learned
    end
    
    Learning-->>Monitor: Learning Complete
```

---

## 5. Swarm Formation Architecture

### 5.1 Swarm Lifecycle

```mermaid
stateDiagram-v2
    [*] --> Forming: Create Swarm
    
    Forming --> AddingMembers: Add Member
    AddingMembers --> AddingMembers: Add More
    AddingMembers --> Validating: All Members Added
    
    Validating --> Active: Validation Success
    Validating --> Error: Validation Failed
    
    Active --> Coordinating: Initiate Sync
    Coordinating --> Syncing: All Coordinated
    
    Syncing --> Monitoring: Transfer Started
    Monitoring --> Syncing: Continue
    Monitoring --> Completing: All Complete
    
    Completing --> Idle: Success
    Completing --> Error: Failure
    
    Idle --> Coordinating: New Sync Request
    Idle --> Disbanding: Disband Command
    
    Error --> Recovering: Auto-Recover
    Recovering --> Idle: Recovery Success
    Recovering --> Disbanding: Recovery Failed
    
    Disbanding --> [*]
    
    note right of Active
        Swarm is ready for
        coordinated operations
    end note
    
    note right of Coordinating
        HyperGNN coordination
        algorithm active
    end note
```

### 5.2 Swarm Topology

```mermaid
graph TB
    subgraph "Swarm Formation: swarm_alpha"
        COORDINATOR[Agent Swarm<br/>Coordinator]
        
        subgraph "Swarm Members"
            M1[Module: data_module<br/>Host: server1:873<br/>State: ACTIVE]
            M2[Module: backup_module<br/>Host: server2:873<br/>State: ACTIVE]
            M3[Module: archive_module<br/>Host: server3:873<br/>State: COORDINATING]
        end
        
        subgraph "AtomSpace Representation"
            SA[Swarm Atom<br/>swarm_alpha<br/>TV: 0.98/0.92]
            L1[Swarm Member Link]
            L2[Swarm Member Link]
            L3[Swarm Member Link]
        end
    end
    
    COORDINATOR --> M1
    COORDINATOR --> M2
    COORDINATOR --> M3
    
    SA --> L1
    SA --> L2
    SA --> L3
    
    L1 --> M1
    L2 --> M2
    L3 --> M3
    
    M1 -.sync.-> M2
    M2 -.sync.-> M3
    M3 -.sync.-> M1
    
    style COORDINATOR fill:#ff9999
    style SA fill:#99ccff
    style M1 fill:#99ff99
    style M2 fill:#99ff99
    style M3 fill:#ffcc99
```

---

## 6. Distributed AtomSpace Architecture

### 6.1 Distributed Network Topology

```mermaid
graph TB
    subgraph "Node 1: Primary"
        AS1[AtomSpace Instance]
        AGENTS1[Agent System]
        SYNC1[Sync Engine]
    end
    
    subgraph "Node 2: Secondary"
        AS2[AtomSpace Instance]
        AGENTS2[Agent System]
        SYNC2[Sync Engine]
    end
    
    subgraph "Node 3: Secondary"
        AS3[AtomSpace Instance]
        AGENTS3[Agent System]
        SYNC3[Sync Engine]
    end
    
    subgraph "Distributed AtomSpace Layer"
        DAS[Distributed AtomSpace<br/>Coordination]
        CONFLICT[Conflict Resolution]
        REPLICATION[Replication Manager]
    end
    
    AS1 <--> DAS
    AS2 <--> DAS
    AS3 <--> DAS
    
    DAS --> CONFLICT
    DAS --> REPLICATION
    
    AS1 -.atom sync.-> AS2
    AS2 -.atom sync.-> AS3
    AS3 -.atom sync.-> AS1
    
    SYNC1 -.file sync.-> SYNC2
    SYNC2 -.file sync.-> SYNC3
    SYNC3 -.file sync.-> SYNC1
    
    style DAS fill:#ff9999
    style CONFLICT fill:#ffcc99
    style AS1 fill:#99ccff
    style AS2 fill:#99ccff
    style AS3 fill:#99ccff
```

### 6.2 Conflict Resolution Strategies

```mermaid
flowchart TD
    START([Atom Conflict Detected])
    
    START --> STRATEGY{Resolution Strategy}
    
    STRATEGY -->|LATEST_WINS| COMPARE_TIME[Compare Timestamps]
    STRATEGY -->|HIGHEST_CONFIDENCE| COMPARE_CONF[Compare Confidence]
    STRATEGY -->|MERGE_TV| MERGE[PLN Revision Rule]
    STRATEGY -->|MANUAL| MANUAL_RESOLVE[Flag for Manual Resolution]
    
    COMPARE_TIME --> SELECT_LATEST[Select Latest Atom]
    COMPARE_CONF --> SELECT_HIGH_CONF[Select Highest Confidence]
    MERGE --> PLN_MERGE[Apply PLN Merge]
    MANUAL_RESOLVE --> QUEUE[Add to Resolution Queue]
    
    SELECT_LATEST --> UPDATE[Update Local AtomSpace]
    SELECT_HIGH_CONF --> UPDATE
    PLN_MERGE --> UPDATE
    QUEUE --> NOTIFY[Notify Administrator]
    
    UPDATE --> PROPAGATE[Propagate to Other Nodes]
    NOTIFY --> WAIT[Wait for Resolution]
    
    PROPAGATE --> END([Conflict Resolved])
    WAIT --> END
    
    style START fill:#ffcc99
    style STRATEGY fill:#99ccff
    style MERGE fill:#99ff99
    style END fill:#99ff99
```

---

## 7. Technology Stack

### 7.1 Core Technologies

| Component | Technology | Purpose |
|-----------|-----------|---------|
| **Language** | C (C99 standard) | Core implementation |
| **Build System** | GNU Autoconf/Automake | Cross-platform compilation |
| **Knowledge Representation** | Hypergraph (Custom) | AtomSpace implementation |
| **Probabilistic Logic** | PLN (Custom) | Inference engine |
| **Serialization** | Binary Format | AtomSpace persistence |
| **Networking** | BSD Sockets | Distributed communication |
| **Math Library** | libm | Floating-point operations |
| **Optional: GGML** | GGML (External) | Tensor operations (stub) |
| **Optional: llama.cpp** | llama.cpp (External) | Neural patterns (stub) |

---

## Appendix A: Glossary

| Term | Definition |
|------|------------|
| **AtomSpace** | Hypergraph-based knowledge representation system |
| **Atom** | Node in the hypergraph representing a concept or entity |
| **Link** | Edge in the hypergraph connecting atoms |
| **Truth Value (TV)** | Probabilistic strength and confidence pair |
| **Attention Value (AV)** | Short/long-term importance values (STI/LTI/VLTI) |
| **PLN** | Probabilistic Logic Networks - inference engine |
| **ECAN** | Economic Attention Allocation Network |
| **Agent Zero** | Master orchestrator agent that generates configurations |
| **Swarm Formation** | Coordinated group of sync modules |
| **HyperGNN** | Hypergraph neural network for swarm coordination |
| **Cognitive Synergy** | Multiple cognitive processes working together |

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained By:** OpenCog Cognitive Architecture Project
