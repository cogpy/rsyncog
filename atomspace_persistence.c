/*
 * AtomSpace Persistence Implementation
 *
 * Copyright (C) 2025 OpenCog Cognitive Architecture Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "atomspace_persistence.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* File format constants */
#define ATOMSPACE_FILE_MAGIC 0x4154534D  /* "ATSM" */
#define ATOMSPACE_FILE_VERSION 1
#define LEARNING_FILE_MAGIC 0x4C52484D   /* "LRHM" */
#define LEARNING_FILE_VERSION 1

/* Global persistence config */
static struct persistence_config *global_config = NULL;
static time_t last_auto_save = 0;

/**
 * File header for binary format
 */
struct atomspace_file_header {
    uint32_t magic;
    uint32_t version;
    uint64_t atom_count;
    uint64_t link_count;
    time_t created_time;
    time_t saved_time;
} __attribute__((packed));

/**
 * Binary atom record
 */
struct atom_record {
    uint64_t handle;
    uint8_t type;
    uint32_t name_len;
    struct truth_value tv;
    struct attention_value av;
} __attribute__((packed));

/**
 * atomspace_save_binary - Save in binary format
 */
int atomspace_save_binary(struct atom_space *atomspace, FILE *fp)
{
    struct atomspace_file_header hdr;
    struct atom_record rec;
    struct atom *atom;
    size_t i;
    int saved = 0;
    
    if (!atomspace || !fp)
        return -1;
    
    /* Write header */
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = ATOMSPACE_FILE_MAGIC;
    hdr.version = ATOMSPACE_FILE_VERSION;
    hdr.atom_count = atomspace->atom_count;
    hdr.link_count = 0;  /* TODO: count links */
    hdr.created_time = time(NULL);
    hdr.saved_time = time(NULL);
    
    if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
        return -1;
    
    /* Write atoms */
    for (i = 0; i < atomspace->atom_table_size; i++) {
        atom = atomspace->atom_table[i];
        while (atom) {
            /* Prepare record */
            memset(&rec, 0, sizeof(rec));
            rec.handle = atom->handle;
            rec.type = atom->type;
            rec.name_len = atom->name ? strlen(atom->name) : 0;
            rec.tv = atom->tv;
            rec.av = atom->av;
            
            /* Write record header */
            if (fwrite(&rec, sizeof(rec), 1, fp) != 1)
                break;
            
            /* Write name if present */
            if (rec.name_len > 0) {
                if (fwrite(atom->name, rec.name_len, 1, fp) != 1)
                    break;
            }
            
            saved++;
            atom = atom->hash_next;
        }
    }
    
    return saved;
}

/**
 * atomspace_load_binary - Load from binary format
 */
struct atom_space *atomspace_load_binary(FILE *fp)
{
    struct atomspace_file_header hdr;
    struct atom_record rec;
    struct atom_space *atomspace;
    struct atom *atom;
    char *name;
    size_t i;
    
    if (!fp)
        return NULL;
    
    /* Read header */
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
        return NULL;
    
    /* Validate header */
    if (hdr.magic != ATOMSPACE_FILE_MAGIC || hdr.version != ATOMSPACE_FILE_VERSION)
        return NULL;
    
    /* Create AtomSpace */
    atomspace = atomspace_create();
    if (!atomspace)
        return NULL;
    
    /* Load atoms */
    for (i = 0; i < hdr.atom_count; i++) {
        /* Read record header */
        if (fread(&rec, sizeof(rec), 1, fp) != 1)
            break;
        
        /* Read name */
        name = NULL;
        if (rec.name_len > 0) {
            name = malloc(rec.name_len + 1);
            if (!name)
                break;
            if (fread(name, rec.name_len, 1, fp) != 1) {
                free(name);
                break;
            }
            name[rec.name_len] = '\0';
        }
        
        /* Create atom */
        atom = atomspace_add_node(atomspace, rec.type, name);
        if (atom) {
            atom->handle = rec.handle;
            atom->tv = rec.tv;
            atom->av = rec.av;
        }
        
        if (name)
            free(name);
    }
    
    return atomspace;
}

/**
 * atomspace_save - Save to file
 */
int atomspace_save(struct atom_space *atomspace, const char *filename)
{
    FILE *fp;
    int result;
    
    if (!atomspace || !filename)
        return -1;
    
    fp = fopen(filename, "wb");
    if (!fp)
        return -1;
    
    result = atomspace_save_binary(atomspace, fp);
    fclose(fp);
    
    return result;
}

/**
 * atomspace_load - Load from file
 */
struct atom_space *atomspace_load(const char *filename)
{
    FILE *fp;
    struct atom_space *atomspace;
    
    if (!filename)
        return NULL;
    
    fp = fopen(filename, "rb");
    if (!fp)
        return NULL;
    
    atomspace = atomspace_load_binary(fp);
    fclose(fp);
    
    return atomspace;
}

/**
 * atomspace_export_json - Export to JSON
 */
int atomspace_export_json(struct atom_space *atomspace, const char *filename)
{
    FILE *fp;
    struct atom *atom;
    size_t i;
    int exported = 0;
    
    if (!atomspace || !filename)
        return -1;
    
    fp = fopen(filename, "w");
    if (!fp)
        return -1;
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"version\": 1,\n");
    fprintf(fp, "  \"atom_count\": %lu,\n", atomspace->atom_count);
    fprintf(fp, "  \"atoms\": [\n");
    
    for (i = 0; i < atomspace->atom_table_size; i++) {
        atom = atomspace->atom_table[i];
        while (atom) {
            if (exported > 0)
                fprintf(fp, ",\n");
            
            fprintf(fp, "    {\n");
            fprintf(fp, "      \"handle\": %lu,\n", atom->handle);
            fprintf(fp, "      \"type\": %d,\n", atom->type);
            fprintf(fp, "      \"name\": \"%s\",\n", atom->name ? atom->name : "");
            fprintf(fp, "      \"truth_value\": {\n");
            fprintf(fp, "        \"strength\": %.6f,\n", atom->tv.strength);
            fprintf(fp, "        \"confidence\": %.6f\n", atom->tv.confidence);
            fprintf(fp, "      },\n");
            fprintf(fp, "      \"attention_value\": {\n");
            fprintf(fp, "        \"sti\": %d,\n", atom->av.sti);
            fprintf(fp, "        \"lti\": %d,\n", atom->av.lti);
            fprintf(fp, "        \"vlti\": %d\n", atom->av.vlti);
            fprintf(fp, "      }\n");
            fprintf(fp, "    }");
            
            exported++;
            atom = atom->hash_next;
        }
    }
    
    fprintf(fp, "\n  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return exported;
}

/**
 * learning_history_save - Save learning history
 */
int learning_history_save(struct learning_context *ctx, const char *filename)
{
    FILE *fp;
    uint32_t magic = LEARNING_FILE_MAGIC;
    uint32_t version = LEARNING_FILE_VERSION;
    struct sync_history_entry *entry;
    int saved = 0;
    
    if (!ctx || !filename)
        return -1;
    
    fp = fopen(filename, "wb");
    if (!fp)
        return -1;
    
    /* Write header */
    fwrite(&magic, sizeof(magic), 1, fp);
    fwrite(&version, sizeof(version), 1, fp);
    fwrite(&ctx->history_count, sizeof(ctx->history_count), 1, fp);
    fwrite(&ctx->learning_rate, sizeof(ctx->learning_rate), 1, fp);
    fwrite(&ctx->decay_factor, sizeof(ctx->decay_factor), 1, fp);
    
    /* Write history entries as linked list */
    entry = ctx->history_head;
    while (entry) {
        uint32_t module_len = entry->module_name ? strlen(entry->module_name) : 0;
        uint32_t host_len = entry->host_name ? strlen(entry->host_name) : 0;
        
        fwrite(&module_len, sizeof(module_len), 1, fp);
        if (module_len > 0)
            fwrite(entry->module_name, module_len, 1, fp);
        
        fwrite(&host_len, sizeof(host_len), 1, fp);
        if (host_len > 0)
            fwrite(entry->host_name, host_len, 1, fp);
        
        fwrite(&entry->success, sizeof(entry->success), 1, fp);
        fwrite(&entry->bytes_transferred, sizeof(entry->bytes_transferred), 1, fp);
        fwrite(&entry->duration, sizeof(entry->duration), 1, fp);
        fwrite(&entry->timestamp, sizeof(entry->timestamp), 1, fp);
        
        entry = entry->next;
        saved++;
    }
    
    /* Write statistics */
    fwrite(&ctx->stats, sizeof(ctx->stats), 1, fp);
    
    fclose(fp);
    return saved;
}

/**
 * learning_history_load - Load learning history
 */
int learning_history_load(struct learning_context *ctx, const char *filename)
{
    FILE *fp;
    uint32_t magic, version;
    uint64_t count;
    struct sync_history_entry *entry;
    uint32_t module_len, host_len;
    int loaded = 0;
    size_t i;
    
    if (!ctx || !filename)
        return -1;
    
    fp = fopen(filename, "rb");
    if (!fp)
        return -1;
    
    /* Read and validate header */
    if (fread(&magic, sizeof(magic), 1, fp) != 1 || magic != LEARNING_FILE_MAGIC) {
        fclose(fp);
        return -1;
    }
    
    if (fread(&version, sizeof(version), 1, fp) != 1 || version != LEARNING_FILE_VERSION) {
        fclose(fp);
        return -1;
    }
    
    /* Read context data */
    fread(&count, sizeof(count), 1, fp);
    fread(&ctx->learning_rate, sizeof(ctx->learning_rate), 1, fp);
    fread(&ctx->decay_factor, sizeof(ctx->decay_factor), 1, fp);
    
    /* Load history entries */
    for (i = 0; i < count; i++) {
        entry = malloc(sizeof(struct sync_history_entry));
        if (!entry)
            break;
        
        memset(entry, 0, sizeof(struct sync_history_entry));
        
        /* Read module name */
        if (fread(&module_len, sizeof(module_len), 1, fp) != 1)
            break;
        if (module_len > 0) {
            entry->module_name = malloc(module_len + 1);
            if (!entry->module_name || fread(entry->module_name, module_len, 1, fp) != 1) {
                free(entry);
                break;
            }
            entry->module_name[module_len] = '\0';
        }
        
        /* Read host name */
        if (fread(&host_len, sizeof(host_len), 1, fp) != 1)
            break;
        if (host_len > 0) {
            entry->host_name = malloc(host_len + 1);
            if (!entry->host_name || fread(entry->host_name, host_len, 1, fp) != 1) {
                free(entry->module_name);
                free(entry);
                break;
            }
            entry->host_name[host_len] = '\0';
        }
        
        /* Read metrics */
        fread(&entry->success, sizeof(entry->success), 1, fp);
        fread(&entry->bytes_transferred, sizeof(entry->bytes_transferred), 1, fp);
        fread(&entry->duration, sizeof(entry->duration), 1, fp);
        fread(&entry->timestamp, sizeof(entry->timestamp), 1, fp);
        
        /* Add to list */
        if (!ctx->history_head) {
            ctx->history_head = entry;
            ctx->history_tail = entry;
        } else {
            ctx->history_tail->next = entry;
            ctx->history_tail = entry;
        }
        
        loaded++;
    }
    
    ctx->history_count = loaded;
    
    /* Read statistics */
    fread(&ctx->stats, sizeof(ctx->stats), 1, fp);
    
    fclose(fp);
    return loaded;
}

/**
 * persistence_init - Initialize persistence
 */
int persistence_init(struct persistence_config *config)
{
    if (!config)
        return -1;
    
    global_config = config;
    last_auto_save = time(NULL);
    
    return 0;
}

/**
 * persistence_auto_save - Auto-save if needed
 */
int persistence_auto_save(struct atom_space *atomspace,
                         struct learning_context *learning_ctx)
{
    time_t now;
    
    if (!global_config || !global_config->auto_save_enabled)
        return 0;
    
    now = time(NULL);
    if (now - last_auto_save < global_config->auto_save_interval)
        return 0;
    
    /* Perform save */
    if (atomspace && global_config->atomspace_path) {
        atomspace_save(atomspace, global_config->atomspace_path);
    }
    
    if (learning_ctx && global_config->learning_path) {
        learning_history_save(learning_ctx, global_config->learning_path);
    }
    
    last_auto_save = now;
    return 1;
}

/**
 * persistence_shutdown - Final save on shutdown
 */
int persistence_shutdown(struct atom_space *atomspace,
                        struct learning_context *learning_ctx)
{
    int result = 0;
    
    if (!global_config)
        return 0;
    
    /* Save AtomSpace */
    if (atomspace && global_config->atomspace_path) {
        if (atomspace_save(atomspace, global_config->atomspace_path) >= 0)
            result++;
    }
    
    /* Save learning history */
    if (learning_ctx && global_config->learning_path) {
        if (learning_history_save(learning_ctx, global_config->learning_path) >= 0)
            result++;
    }
    
    return result > 0 ? 0 : -1;
}
