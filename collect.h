/*************************************************************************
    > File Name: collect.h
    > Author: ToLiMit
    > Mail: 348958453@qq.com
    > Created Time: Wed 28 Oct 2015 03:21:46 PM CST
 ************************************************************************/

#ifndef __COLLECT_H
#define __COLLECT_H

void collect_smp (struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk);
void collect_disk (struct disk_brk *disk_brk);
void collect_lpar (struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk);
void collect_large (struct large_brk * large_brk);
void collect_net (struct net_brk * net_brk);
void collect_nfs (struct nfs_brk * nfs_brk);
void collect_kernel (struct kernel_brk * kernel_brk);
void collect_dgroup (struct disk_brk * disk_brk);
void collect_vm (struct mem_brk * mem_brk);
void collect_top_info (struct top_brk * top_brk, unsigned long pagesize, double ignore_procdisk_threshold);
#endif
