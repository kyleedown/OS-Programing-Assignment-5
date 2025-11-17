#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>


typedef enum{
    CMD_UNKNOWN = 0,
    CMD_RQ,
    CMD_RL,
    CMD_C,
    CMD_STAT,
    CMD_X
} CommandType;

#define MAX_ARGS 4
#define MAX_TOKEN 32
#define PROC_NAME_LEN 32
#define VISBAR_LEN 50

typedef struct {
    CommandType cmd;
    char args[MAX_ARGS][MAX_TOKEN];
    int argc;
} Command;

typedef struct Block {
    char process[PROC_NAME_LEN]; // "HOLE" or process name
    int start;
    int size;
    int allocated;   // 1 = allocated, 0 = free (hole)
    struct Block *next;
} Block;

/* Global memory list head and size */
Block *memory = NULL;
int MEMORY_SIZE = 0;

CommandType cmd_from_string(const char *s) {
    if (strcmp(s, "RQ") == 0) return CMD_RQ;
    if (strcmp(s, "RL") == 0) return CMD_RL;
    if (strcmp(s, "C") == 0) return CMD_C;
    if (strcmp(s, "STAT") == 0) return CMD_STAT;
    if (strcmp(s, "X") == 0) return CMD_X;
    return CMD_UNKNOWN;
}

Command get_choice(FILE *in){
    Command result = {0};
    char buffer[256];
    

    while(1){
        if (in == stdin) {
            printf("allocator> ");
            fflush(stdout);
        }
        if (!fgets(buffer, sizeof(buffer), in)) {
        result.cmd = CMD_X;  // treat as exit
        return result;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        char *token = strtok(buffer, " ");
        if (!token) continue;

        result.cmd = cmd_from_string(token);
        if(result.cmd == CMD_UNKNOWN){
            printf("Invalid option. Try again\n");
            continue;
        }

        int i = 0;
        while ((token = strtok(NULL," ")) != NULL && i < MAX_ARGS){
            strncpy(result.args[i], token, MAX_TOKEN-1);
            i++;
        }
        result.argc = i;

        return result;
    }
}

Block *create_block(const char *proc, int start, int size, int allocated) {
    Block *b = (Block *) malloc(sizeof(Block));
    if (!b) { perror("malloc"); exit(1); }
    strncpy(b->process, proc, PROC_NAME_LEN-1);
    b->process[PROC_NAME_LEN-1] = '\0';
    b->start = start;
    b->size = size;
    b->allocated = allocated;
    b->next = NULL;
    return b;
}

void free_list(Block *head) {
    while (head) {
        Block *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/* initialize memory with a single hole */
void init_memory(int size) {
    free_list(memory);
    memory = create_block("HOLE", 0, size, 0);
}

/* find if a process name already exists (allocated) */
int process_exists(const char *proc) {
    for (Block *b = memory; b; b = b->next) {
        if (b->allocated && strcmp(b->process, proc) == 0)
            return 1;
    }
    return 0;
}

/* merge adjacent free holes - iterate once and merge neighbors */
void merge_free_holes() {
    Block *cur = memory;
    while (cur && cur->next) {
        if (!cur->allocated && !cur->next->allocated) {
            Block *next = cur->next;
            cur->size += next->size;
            cur->next = next->next;
            free(next);
            // do NOT advance cur; maybe more holes to merge
        } else {
            cur = cur->next;
        }
    }
}

/* ---------- Allocation strategies ---------- */
/* return 0 on success, -1 on failure */
int allocate_first_fit(const char *proc, int req_size) {
    Block *cur = memory;
    Block *prev = NULL;
    while (cur) {
        if (!cur->allocated && cur->size >= req_size) {
            if (strcmp(cur->process, "HOLE") != 0) strncpy((char*)cur->process, "HOLE", PROC_NAME_LEN); // just in case
            if (cur->size == req_size) {
                // perfect fit: mark as allocated
                cur->allocated = 1;
                strncpy(cur->process, proc, PROC_NAME_LEN-1);
                cur->process[PROC_NAME_LEN-1] = '\0';
            } else {
                // split: allocated block before the remaining hole
                Block *alloc = create_block(proc, cur->start, req_size, 1);
                cur->start += req_size;
                cur->size -= req_size;
                // insert alloc before cur
                if (prev == NULL) {
                    alloc->next = memory;
                    memory = alloc;
                } else {
                    alloc->next = prev->next;
                    prev->next = alloc;
                }
            }
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}

int allocate_best_fit(const char *proc, int req_size) {
    Block *cur = memory;
    Block *prev = NULL;
    Block *best = NULL;
    Block *best_prev = NULL;

    int best_size = INT_MAX;
    while (cur) {
        if (!cur->allocated && cur->size >= req_size) {
            if (cur->size < best_size) {
                best_size = cur->size;
                best = cur;
                best_prev = prev;
            }
        }
        prev = cur;
        cur = cur->next;
    }

    if (!best) return -1;

    if (best->size == req_size) {
        best->allocated = 1;
        strncpy(best->process, proc, PROC_NAME_LEN-1);
        best->process[PROC_NAME_LEN-1] = '\0';
    } else {
        Block *alloc = create_block(proc, best->start, req_size, 1);
        best->start += req_size;
        best->size -= req_size;
        if (best_prev == NULL) {
            alloc->next = memory;
            memory = alloc;
        } else {
            alloc->next = best_prev->next;
            best_prev->next = alloc;
        }
    }
    return 0;
}

int allocate_worst_fit(const char *proc, int req_size) {
    Block *cur = memory;
    Block *prev = NULL;
    Block *worst = NULL;
    Block *worst_prev = NULL;

    int worst_size = -1;
    while (cur) {
        if (!cur->allocated && cur->size >= req_size) {
            if (cur->size > worst_size) {
                worst_size = cur->size;
                worst = cur;
                worst_prev = prev;
            }
        }
        prev = cur;
        cur = cur->next;
    }

    if (!worst) return -1;

    if (worst->size == req_size) {
        worst->allocated = 1;
        strncpy(worst->process, proc, PROC_NAME_LEN-1);
        worst->process[PROC_NAME_LEN-1] = '\0';
    } else {
        Block *alloc = create_block(proc, worst->start, req_size, 1);
        worst->start += req_size;
        worst->size -= req_size;
        if (worst_prev == NULL) {
            alloc->next = memory;
            memory = alloc;
        } else {
            alloc->next = worst_prev->next;
            worst_prev->next = alloc;
        }
    }
    return 0;
}

/* master allocate function */
void request_memory(const char *proc, int req_size, const char *flag) {
    if (!proc || strlen(proc) == 0) {
        printf("Error: missing process name\n");
        return;
    }
    if (req_size <= 0) {
        printf("Error: invalid size\n");
        return;
    }
    if (process_exists(proc)) {
        printf("Error: process %s already exists\n", proc);
        return;
    }

    char f = toupper((unsigned char)flag[0]);
    int result = -1;
    if (f == 'F') result = allocate_first_fit(proc, req_size);
    else if (f == 'B') result = allocate_best_fit(proc, req_size);
    else if (f == 'W') result = allocate_worst_fit(proc, req_size);
    else {
        printf("Error: unknown fit type '%s' (use F, B or W)\n", flag);
        return;
    }

    if (result == 0) {
        merge_free_holes(); // just in case splitting left adjacent holes (usually not necessary)
        printf("Allocated %s of size %d using %c-fit\n", proc, req_size, f);
    } else {
        printf("Error: Not enough memory for process %s of size %d\n", proc, req_size);
    }
}

/* ---------- Release ---------- */
void release_memory(const char *proc) {
    if (!proc || strlen(proc) == 0) {
        printf("Error: missing process name\n");
        return;
    }

    Block *cur = memory;
    int found = 0;
    while (cur) {
        if (cur->allocated && strcmp(cur->process, proc) == 0) {
            found = 1;
            cur->allocated = 0;
            strncpy(cur->process, "HOLE", PROC_NAME_LEN-1);
            cur->process[PROC_NAME_LEN-1] = '\0';
            break;
        }
        cur = cur->next;
    }
    if (!found) {
        printf("Error: process %s not found\n", proc);
        return;
    }
    merge_free_holes();
    printf("Process %s released\n", proc);
}

/* ---------- Compaction (create new list and free old) ---------- */
void compact_memory() {
    if (!memory) return;
    Block *newList = NULL;
    Block *tail = NULL;
    int currentAddress = 0;
    int totalFree = 0;

    for (Block *cur = memory; cur != NULL; cur = cur->next) {
        if (cur->allocated) {
            Block *b = create_block(cur->process, currentAddress, cur->size, 1);
            currentAddress += cur->size;
            if (!newList) { newList = tail = b; }
            else { tail->next = b; tail = b; }
        } else {
            totalFree += cur->size;
        }
    }
    if (totalFree > 0) {
        Block *hole = create_block("HOLE", currentAddress, totalFree, 0);
        if (!newList) newList = hole;
        else tail->next = hole;
    }

    free_list(memory);
    memory = newList;
    printf("Compaction completed\n");
}

/* ---------- STAT and visualization ---------- */
void print_status(int visualize) {
    if (!memory) {
        printf("Memory not initialized\n");
        return;
    }

    printf("Allocated memory:\n");
    Block *cur = memory;
    int any_alloc = 0;
    while (cur) {
        if (cur->allocated) {
            any_alloc = 1;
            printf("Process %s: Start = %d, End = %d, Size = %d\n",
                cur->process,
                cur->start,
                cur->start + cur->size,
                cur->size);
        }
        cur = cur->next;
    }
    if (!any_alloc) printf("  <none>\n");

    // Free memory list and collect stats
    printf("\nFree memory:\n");
    cur = memory;
    int hole_idx = 1;
    int any_hole = 0;
    long total_alloc = 0;
    long total_free = 0;
    int largest_hole = 0;
    int holes_count = 0;

    while (cur) {
        if (!cur->allocated) {
            any_hole = 1;
            printf("Hole %d: Start = %d, End = %d, Size = %d\n",
                hole_idx,
                cur->start,
                cur->start + cur->size,
                cur->size);
            hole_idx++;
            holes_count++;
            total_free += cur->size;
            if (cur->size > largest_hole) largest_hole = cur->size;
        } else {
            total_alloc += cur->size;
        }
        cur = cur->next;
    }
    if (!any_hole) printf("  <none>\n");

    // Summary
    double ext_frag_percent = 0.0;
    double avg_hole_size = 0.0;
    if (total_free > 0) {
        ext_frag_percent = 100.0 * (1.0 - ((double)largest_hole / (double)total_free));
        if (holes_count > 0) avg_hole_size = (double)total_free / (double)holes_count;
    }

    printf("\nSummary:\n");
    printf("Total allocated: %ld\n", total_alloc);
    printf("Total free: %ld\n", total_free);
    printf("Largest hole: %d\n", largest_hole);
    if (total_free > 0)
        printf("External fragmentation: %.2f%% (1 - largest_free/total_free)\n", ext_frag_percent);
    else
        printf("External fragmentation: 0.00%%\n");
    if (holes_count > 0)
        printf("Average hole size: %.2f\n", avg_hole_size);
    else
        printf("Average hole size: 0.00\n");

    if (visualize) {
        // Build a 50-character map
        char bar[VISBAR_LEN + 1];
        for (int i = 0; i < VISBAR_LEN; ++i) bar[i] = '.';
        bar[VISBAR_LEN] = '\0';

        // compute chars per block by proportional rounding
        cur = memory;
        int used_chars = 0;
        while (cur) {
            double fraction = (double)cur->size / (double)MEMORY_SIZE;
            int chars = (int)round(fraction * VISBAR_LEN);
            if (chars < 0) chars = 0;
            if (chars > VISBAR_LEN - used_chars) chars = VISBAR_LEN - used_chars; // cap
            for (int k = 0; k < chars; ++k) {
                if (cur->allocated) bar[used_chars + k] = '#';
                else bar[used_chars + k] = '.';
            }
            used_chars += chars;
            cur = cur->next;
            if (used_chars >= VISBAR_LEN) break;
        }
        // if rounding left empty characters, fill them with '.' (holes)
        // ensure first and last marker positions labels printed separately
        printf("\n[%s]\n", bar);
        printf("^0");
        for (int i = 0; i < VISBAR_LEN - 6; ++i) putchar(' ');
        printf("^%d\n", MEMORY_SIZE);
    }
}


/* ---------- Main execution ---------- */
int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("Usage: %s <memory_size> [SIM <file>]\n", argv[0]);
        return 1;
    }

    MEMORY_SIZE = atoi(argv[1]);
    if (MEMORY_SIZE <= 0) {
        printf("Invalid memory size\n");
        return 1;
    }

    init_memory(MEMORY_SIZE);

    FILE *input = stdin;
    int sim_mode = 0;
    if (argc > 2 && strcmp(argv[2], "SIM") == 0) {
        if (argc < 4) {
            printf("SIM mode requires filename: %s <size> SIM <file>\n", argv[0]);
            return 1;
        }
        input = fopen(argv[3], "r");
        if (!input) {
            perror("fopen");
            return 1;
        }
        sim_mode = 1;
    }

    Command c;
    while (1){
        c = get_choice(input);

        switch(c.cmd){
            case CMD_RQ: {
                if (c.argc < 3) {
                    printf("Invalid RQ format. Usage: RQ <process> <size> <F|B|W>\n");
                } else {
                    char pid[MAX_TOKEN];
                    strncpy(pid, c.args[0], MAX_TOKEN-1); pid[MAX_TOKEN-1] = '\0';
                    int size = atoi(c.args[1]);
                    char fit_type[MAX_TOKEN];
                    strncpy(fit_type, c.args[2], MAX_TOKEN-1); fit_type[MAX_TOKEN-1] = '\0';
                    request_memory(pid, size, fit_type);
                }
                break;
            }
            case CMD_RL: {
                if (c.argc < 1) {
                    printf("Invalid RL format. Usage: RL <process>\n");
                } else {
                    char pid[MAX_TOKEN];
                    strncpy(pid, c.args[0], MAX_TOKEN-1); pid[MAX_TOKEN-1] = '\0';
                    release_memory(pid);
                }
                break;
            }
            case CMD_C:
                compact_memory();
                break;
            case CMD_STAT: {
                int vis = 0;
                if (c.argc >= 1) {
                    // allow STAT -v or STAT -V as argument
                    if (strcmp(c.args[0], "-v") == 0 || strcmp(c.args[0], "-V") == 0) vis = 1;
                }
                print_status(vis);
                break;
            }
            case CMD_X:
                printf("Exiting...\n");
                free_list(memory);
                if (sim_mode && input != stdin) fclose(input);
                return 0;
            case CMD_UNKNOWN:
            default:
                printf("Unknown command\n");
                break;
        }
    }

    return 0;
}


// int main(int argc, char *argv[]){
//     int memorySize = atoi(argv[1]);
//     Command c;
//     FILE *input = stdin;
//     if (argc > 2 && (strcmp(argv[2],"SIM")) == 0) {
//         input = fopen(argv[3], "r");
//         if (!input) {
//             perror("fopen");
//             return 1;
//         }
//     }

//     char *pid;
//     int size;
//     char* fit_type;

//     while(1){
//         c = get_choice(input);
//         switch(c.cmd){
//             case CMD_RQ:
//                 pid = c.args[0];
//                 size = atoi(c.args[1]);
//                 fit_type = c.args[2];
//                 printf("PID: %s\nSize: %d\nFlag: %s\n",pid,size,fit_type);
//                 break;
//             case CMD_RL:
//                 pid = c.args[0];
//                 break;
//             case CMD_C:
//                 break;
//             case CMD_STAT:
//                 break;
//             case CMD_X:
//                 printf("exiting... ");
//                 return 0;
//             case CMD_UNKNOWN:
//                 break;
//         }
        
//     }
    


// }