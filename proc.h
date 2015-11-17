#ifndef __PROC_H
#define __PROC_H



#include <dirent.h>
#include "struct.h"


void proc_init(struct proc_file * proc);
void proc_read(struct proc_file * proc, int num);
#ifdef POWER
void proc_lparcfg(struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk);
#endif
void proc_net(struct net_brk * net_brk);
int proc_procsinfo(struct global_data * g_data, int pid, int index, struct data * p);

#ifdef DEBUGPROC
void print_procs(int index);
#endif
int getprocs(struct global_data * g_data, int details, struct data * p);
void proc_cpu(struct cpuinfo_brk * cpuinfo_brk);
void proc_nfs(struct nfs_brk * nfs_brk);
void proc_kernel(struct kernel_brk * kernel_brk);
void proc_net(struct net_brk * net_brk);
void proc_disk(struct disk_brk * brk);
void proc_mem(struct mem_brk * mem_brk);

#endif
