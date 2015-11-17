#ifndef __DISPLAY_H
#define __DISPLAY_H



void show_disk (struct disk_brk * brk);
void show_cpu_info (struct cpuinfo_brk * brk, struct lparcfg_brk * lparcfg_brk, struct utsname * uts);
void show_top_info (struct top_brk * top_brk, unsigned long pagesize, double ignore_procdisk_threshold);
void show_kernel (struct kernel_brk * kernel_brk);
void show_mem (struct mem_brk * mem_brk, struct lparcfg_brk * lparcfg_brk);
void show_jfs (struct jfs_brk * jfs_brk);
void show_smp(struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk);
void show_longterm (struct cpuinfo_brk * cpuinfo_brk);
void show_dgroup (struct disk_brk * disk_brk);
void show_verbose (struct cpuinfo_brk * cpuinfo_brk, struct disk_brk * disk_brk);
void show_lpar (struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk);
void show_diskmap (struct disk_brk * disk_brk);
void show_vm(struct mem_brk * mem_brk);
#endif
