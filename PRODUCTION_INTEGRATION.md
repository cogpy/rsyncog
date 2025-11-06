# OpenCog Production Integration Guide

## Overview

This document describes the production-ready features added to rsyncog, completing the integration of GGML tensor operations, llama.cpp neural inference, distributed AtomSpace networking, persistence, and production hardening.

## New Features

### 1. GGML Integration (Tensor Operations)

**Status**: Build system ready, stub implementation included

**Description**: Integrates GGML (Georgi Gerganov's Machine Learning library) for efficient tensor-based operations on hypergraph structures.

**Configuration**:
```bash
./configure --enable-ggml
make
```

**Headers**: `ggml_hypergraph.h`

**Key Functions**:
- `ggml_hypergraph_create()` - Create GGML tensor context for AtomSpace
- `ggml_hypergraph_build_embeddings()` - Build dense vector embeddings for atoms
- `ggml_hypergraph_compute_similarity()` - Fast tensor-based similarity computation
- `ggml_hypergraph_propagate_attention()` - Spread attention using matrix operations
- `ggml_hypergraph_cluster()` - Tensor-based clustering of related atoms

**When to Use**:
- Large-scale AtomSpace operations (>10,000 atoms)
- Fast similarity computations
- Attention propagation across hypergraph
- Clustering and pattern discovery

**Example**:
```c
struct ggml_hypergraph_context *ggml_ctx;
ggml_ctx = ggml_hypergraph_create(atomspace, 128);  /* 128-dim embeddings */
ggml_hypergraph_build_embeddings(ggml_ctx);
float similarity = ggml_hypergraph_compute_similarity(ggml_ctx, atom1, atom2);
```

### 2. llama.cpp Integration (Neural Pattern Recognition)

**Status**: Build system ready, stub implementation included

**Description**: Integrates llama.cpp for neural network-based pattern recognition on sync patterns and AtomSpace structures.

**Configuration**:
```bash
./configure --enable-llama
make
```

**Headers**: `llama_pattern.h`

**Key Functions**:
- `llama_pattern_create()` - Create neural pattern recognition context
- `llama_pattern_predict_success()` - Predict sync success using neural network
- `llama_pattern_classify_failure()` - Classify type of sync failure
- `llama_pattern_generate_schedule()` - AI-generated optimal sync schedules
- `llama_pattern_detect_anomaly()` - Detect anomalous sync patterns
- `llama_pattern_train()` - Fine-tune model on historical data

**When to Use**:
- Complex pattern recognition beyond statistical methods
- Anomaly detection in sync operations
- AI-driven scheduling optimization
- Failure type classification

**Example**:
```c
struct llama_pattern_context *llama_ctx;
llama_ctx = llama_pattern_create(atomspace, learning_ctx, "model.gguf");
struct pattern_result result = llama_pattern_predict_success(
    llama_ctx, "production_data", "node1.example.com");
printf("Success probability: %.2f%%\n", result.result.success_probability * 100.0);
```

### 3. Distributed AtomSpace Network Protocol

**Status**: Fully implemented

**Description**: Complete network protocol for synchronizing AtomSpace across distributed nodes with serialization, conflict resolution, and TCP connectivity.

**Headers**: `distributed_atomspace.h`

**Features**:
- **Binary Serialization**: Efficient atom/link serialization with metadata
- **TCP Network Protocol**: Socket-based communication between nodes
- **Conflict Resolution**: Multiple strategies (Latest Wins, Highest Confidence, PLN Merge)
- **Incremental Sync**: Sync only changes since last synchronization
- **Remote Mirrors**: Maintain local copies of remote AtomSpaces
- **Subscription Model**: Subscribe to atom updates from remote nodes

**Key Functions**:
- `distributed_atomspace_create()` - Create distributed context
- `distributed_atomspace_add_node()` - Add remote node to network
- `distributed_atomspace_connect()` - Establish TCP connection
- `distributed_atomspace_sync_full()` - Full synchronization
- `distributed_atomspace_sync_incremental()` - Incremental sync
- `distributed_atomspace_set_conflict_strategy()` - Configure conflict resolution

**Protocol Details**:
- **Magic Number**: 0x41544F4D ("ATOM")
- **Version**: 1
- **Message Types**: SYNC_REQUEST, SYNC_ATOM, SYNC_LINK, SYNC_COMPLETE, SYNC_ACK, CONFLICT_NOTIFY
- **Transport**: TCP sockets with non-blocking I/O
- **Serialization**: Packed binary structures for efficiency

**Example**:
```c
struct distributed_atomspace *das;
das = distributed_atomspace_create(local_atomspace);

/* Add remote nodes */
uint64_t node1 = distributed_atomspace_add_node(das, "node1.example.com", 8730);
uint64_t node2 = distributed_atomspace_add_node(das, "node2.example.com", 8730);

/* Configure conflict resolution */
distributed_atomspace_set_conflict_strategy(das, CONFLICT_MERGE_TV);

/* Connect and sync */
distributed_atomspace_connect(das, node1);
distributed_atomspace_sync_full(das, node1);  /* Sync all atoms */
distributed_atomspace_sync_incremental(das, 0);  /* Sync to all nodes */
```

### 4. Persistence (Binary & JSON)

**Status**: Fully implemented

**Description**: Complete persistence system for saving/loading AtomSpace state and learning history with auto-save support.

**Headers**: `atomspace_persistence.h`

**Features**:
- **Binary Format**: Efficient binary serialization with file magic/version
- **JSON Export**: Human-readable JSON format for inspection
- **Learning History**: Persist complete learning observations and statistics
- **Auto-Save**: Configurable automatic periodic saves
- **Incremental Saving**: Save only changes (for learning history)

**Key Functions**:
- `atomspace_save()` - Save AtomSpace to binary file
- `atomspace_load()` - Load AtomSpace from binary file
- `atomspace_export_json()` - Export AtomSpace to JSON
- `learning_history_save()` - Save learning observations
- `learning_history_load()` - Load learning history
- `persistence_init()` - Configure auto-save
- `persistence_auto_save()` - Trigger auto-save if needed
- `persistence_shutdown()` - Final save on shutdown

**File Formats**:

Binary AtomSpace Format:
```
Header (32 bytes):
- magic: 0x4154534D ("ATSM")
- version: 1
- atom_count: uint64_t
- link_count: uint64_t
- created_time: time_t
- saved_time: time_t

Atom Records (variable):
- handle: uint64_t
- type: uint8_t
- name_len: uint32_t
- name: char[name_len]
- tv: struct truth_value
- av: struct attention_value
```

Learning History Format:
```
Header:
- magic: 0x4C52484D ("LRHM")
- version: 1
- history_count: uint64_t
- learning_rate: float
- decay_factor: float

History Entries:
- module_name_len + module_name
- host_name_len + host_name
- success: int
- bytes_transferred: uint64_t
- duration: time_t
- timestamp: time_t

Statistics: struct learning_stats
```

**Example**:
```c
struct persistence_config config = {
    .atomspace_path = "./atomspace.bin",
    .learning_path = "./learning_history.bin",
    .auto_save_enabled = 1,
    .auto_save_interval = 300,  /* 5 minutes */
    .compression_enabled = 0
};

persistence_init(&config);

/* Auto-save happens automatically */
persistence_auto_save(atomspace, learning_ctx);

/* Manual save */
atomspace_save(atomspace, "./backup.bin");
atomspace_export_json(atomspace, "./atomspace.json");
learning_history_save(learning_ctx, "./history.bin");

/* Load */
struct atom_space *loaded = atomspace_load("./atomspace.bin");
learning_history_load(learning_ctx, "./history.bin");
```

### 5. Production Hardening

**Status**: Fully implemented

**Description**: Comprehensive production monitoring, error recovery, and rate limiting with Prometheus metrics export.

**Headers**: `production_monitor.h`

**Components**:

#### a. Error Recovery with Circuit Breaker

**Features**:
- **Retry Logic**: Configurable max retries with exponential backoff
- **Circuit Breaker**: Automatically opens after threshold failures
- **Backoff Multiplier**: Exponential delay between retries
- **Auto-Recovery**: Circuit closes after timeout

**Configuration**:
```c
struct error_recovery_config config = {
    .max_retries = 3,
    .retry_delay = 5,              /* seconds */
    .backoff_multiplier = 2,        /* exponential */
    .circuit_breaker_threshold = 5, /* failures */
    .circuit_breaker_timeout = 30   /* seconds */
};

struct error_recovery_context *err_ctx = error_recovery_create(&config);

/* Check if should retry */
if (error_recovery_should_retry(err_ctx, error_code)) {
    time_t delay = error_recovery_get_retry_delay(err_ctx, attempt);
    sleep(delay);
    /* retry operation */
}

/* Record outcomes */
error_recovery_record_failure(err_ctx);
error_recovery_record_success(err_ctx);
```

#### b. Token Bucket Rate Limiting

**Features**:
- **Token Bucket Algorithm**: Smooth rate limiting with burst support
- **Configurable Window**: Requests per time window
- **Burst Allowance**: Handle short bursts above average rate
- **Auto-Refill**: Tokens automatically refill over time

**Configuration**:
```c
struct rate_limit_config config = {
    .max_requests = 100,  /* per time window */
    .time_window = 60,    /* seconds */
    .burst_size = 10      /* max burst */
};

struct rate_limiter *limiter = rate_limiter_create(&config);

/* Check if request allowed */
if (rate_limiter_allow(limiter)) {
    /* Process request */
} else {
    time_t wait = rate_limiter_wait_time(limiter);
    /* Wait or reject */
}
```

#### c. Production Monitoring & Metrics

**Features**:
- **Operation Tracking**: Total, successful, failed operations
- **Performance Metrics**: Avg/max/min operation duration
- **Memory Tracking**: Allocated, freed, peak usage
- **Network Metrics**: Bytes sent/received, errors
- **Health Score**: 0.0-1.0 calculated from all metrics
- **Prometheus Export**: Standard metrics format

**Metrics Tracked**:
```
opencog_uptime_seconds          - System uptime
opencog_operations_total        - Total operations
opencog_operations_successful   - Successful operations
opencog_operations_failed       - Failed operations
opencog_operation_duration_avg  - Average duration (ms)
opencog_memory_allocated_bytes  - Total memory allocated
opencog_memory_peak_bytes       - Peak memory usage
opencog_network_bytes_sent      - Total bytes sent
opencog_network_bytes_received  - Total bytes received
opencog_network_errors_total    - Network errors
opencog_health_score            - Overall health [0.0-1.0]
```

**Example**:
```c
struct monitor_stats *stats = monitor_stats_create();

/* Record operation */
double duration = measure_operation();
monitor_record_operation(stats, duration, success);

/* Record memory */
monitor_record_memory(stats, bytes_allocated);

/* Record network */
monitor_record_network(stats, bytes_sent, bytes_received, had_error);

/* Calculate health */
float health = monitor_calculate_health(stats);

/* Export metrics */
monitor_export_metrics(stats, "./metrics.prom");
monitor_print_stats(stats, stdout);
```

## Building with Production Features

### Basic Build (Stubs)
```bash
./configure --disable-md2man --disable-xxhash
make
make opencog-production-demo
```

### Full Build (with GGML and llama.cpp)
```bash
# Install dependencies
sudo apt-get install libggml-dev  # if available
# Build llama.cpp from source and install

./configure --disable-md2man --disable-xxhash --enable-ggml --enable-llama
make
make opencog-production-demo
```

## Running the Demo

```bash
./opencog_production_demo
```

This will demonstrate all production features and generate:
- `./atomspace.bin` - Binary AtomSpace snapshot
- `./atomspace.json` - JSON export
- `./learning_history.bin` - Learning history data
- `./rsyncd.conf.production` - Optimized configuration
- `./metrics.prom` - Prometheus metrics

## Production Deployment

### 1. Monitoring Setup

Configure Prometheus to scrape metrics:

```yaml
# prometheus.yml
scrape_configs:
  - job_name: 'opencog_rsync'
    static_configs:
      - targets: ['localhost:9090']
    file_sd_configs:
      - files:
        - '/path/to/metrics.prom'
```

### 2. Auto-Save Configuration

Enable automatic persistence:

```c
struct persistence_config config = {
    .atomspace_path = "/var/lib/opencog/atomspace.bin",
    .learning_path = "/var/lib/opencog/learning.bin",
    .auto_save_enabled = 1,
    .auto_save_interval = 300,  /* 5 minutes */
};
persistence_init(&config);
```

### 3. Distributed Network

Setup distributed nodes:

```c
/* On each node */
struct distributed_atomspace *das = distributed_atomspace_create(atomspace);

/* Add peer nodes */
distributed_atomspace_add_node(das, "peer1.prod.example.com", 8730);
distributed_atomspace_add_node(das, "peer2.prod.example.com", 8730);
distributed_atomspace_add_node(das, "peer3.prod.example.com", 8730);

/* Configure for production */
distributed_atomspace_set_conflict_strategy(das, CONFLICT_MERGE_TV);

/* Periodic sync */
while (running) {
    distributed_atomspace_sync_incremental(das, 0);  /* All nodes */
    sleep(60);  /* Every minute */
}
```

### 4. Error Recovery

Configure robust error handling:

```c
struct error_recovery_config err_config = {
    .max_retries = 5,
    .retry_delay = 10,
    .backoff_multiplier = 2,
    .circuit_breaker_threshold = 10,
    .circuit_breaker_timeout = 60
};

struct error_recovery_context *err_ctx = error_recovery_create(&err_config);
```

### 5. Rate Limiting

Protect against overload:

```c
struct rate_limit_config rate_config = {
    .max_requests = 1000,  /* per minute */
    .time_window = 60,
    .burst_size = 100
};

struct rate_limiter *limiter = rate_limiter_create(&rate_config);
```

## Performance Tuning

### GGML Optimization
- **Embedding Dimension**: Balance between accuracy (higher) and performance (lower)
- **Recommended**: 64-256 dimensions for most workloads
- **Large Scale**: 512+ dimensions for >100k atoms

### Network Protocol
- **Sync Frequency**: Balance between consistency and network load
- **Incremental Sync**: Use for frequent updates (every 1-5 minutes)
- **Full Sync**: Use for recovery or initialization only

### Persistence
- **Auto-Save Interval**: 300-600 seconds for most workloads
- **Binary Format**: Use for production (10x faster than JSON)
- **JSON Export**: Use for debugging and inspection only

### Monitoring
- **Health Checks**: Run every 30-60 seconds
- **Metrics Export**: Update every 15-30 seconds for Prometheus
- **Log Level**: ERROR in production, INFO for debugging

## Security Considerations

### Network Protocol
- Add TLS/SSL encryption for distributed sync (future enhancement)
- Implement authentication tokens for node connections
- Use firewall rules to restrict node access

### Persistence
- Encrypt sensitive AtomSpace data at rest
- Restrict file permissions (600 or 640)
- Validate loaded data integrity

### Rate Limiting
- Configure based on actual capacity
- Monitor rejection rates
- Adjust dynamically based on load

## Troubleshooting

### GGML Not Available
```
Build with --enable-ggml for full support
```
- Install GGML library or build from source
- Stub implementation provides basic functionality

### llama.cpp Not Available
```
Build with --enable-llama for full support
```
- Install llama.cpp or build from source
- Stub implementation falls back to truth values

### Network Connection Failed
```
distributed_atomspace_connect() returns -1
```
- Check node hostname and port
- Verify firewall rules
- Check network connectivity

### Persistence Failed
```
atomspace_save() returns -1
```
- Check file permissions
- Verify disk space
- Check directory exists

### Circuit Breaker Open
```
error_recovery_should_retry() returns 0
```
- Wait for timeout period
- Check root cause of failures
- Adjust threshold if needed

## Future Enhancements

1. **GGML Integration**: Link with actual GGML library
2. **llama.cpp Integration**: Link with llama.cpp for neural inference
3. **TLS/SSL**: Encrypt network protocol
4. **Compression**: Add zstd compression for persistence
5. **Sharding**: Distribute AtomSpace across multiple nodes
6. **Replication**: Multi-master replication with CRDT
7. **Backup**: Automated backup rotation
8. **Alerting**: Integration with alerting systems

## License

GNU General Public License v3.0 - same as rsync

## Copyright

Copyright (C) 2025 OpenCog Cognitive Architecture Project

Based on rsync by Andrew Tridgell, Wayne Davison, and others.
