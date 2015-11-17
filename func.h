/*************************************************************************
  > File Name: func.h
  > Author: ToLiMit
  > Mail: 348958453@qq.com
  > Created Time: Wed 28 Oct 2015 02:11:48 PM CST
 ************************************************************************/

#ifndef __FUNC_H
#define __FUNC_H

#ifdef MALLOC_DEBUG
#define MALLOC(argument)        mymalloc(argument,__LINE__)
#define FREE(argument)          myfree(argument,__LINE__)
#define REALLOC(argument1,argument2)    myrealloc(argument1,argument2,__LINE__)
#else
#define MALLOC(argument)        malloc(argument)
#define FREE(argument)          free(argument)
#define REALLOC(argument1,argument2)    realloc(argument1,argument2)
#endif /* MALLOC STUFF */

#define DKDELTA(member) ( (q->dk[i].member > p->dk[i].member) ? 0 : (p->dk[i].member - q->dk[i].member))
#define SIDELTA(member) ( (q->si.member > p->si.member)       ? 0 : (p->si.member - q->si.member))

#define TIMEDELTA(member,index1,index2) ((p->procs[index1].member) - (q->procs[index2].member))
#define TIMED(member) ((double)(p->procs[i].member.tv_sec))

#define LOOP(loop,show) timestamp(loop, show)

#define FLIP(variable) if(variable) variable=0; else variable=1;


#define CURSE if(cursed)  /* Only use this for single line curses calls */
#define COLOUR if(colour) /* Only use this for single line colour curses calls */
#define RRD if(show_rrd)

#define VMDELTA(variable) (p->vm.variable - q->vm.variable)
#define VMCOUNT(variable) (p->vm.variable                 )

#define NFS_TOTAL(member) (double)(p->member)
#define NFS_DELTA(member) (((double)(p->member - q->member)/elapsed))

#define RAW(member)      (long)((long)(p->cpuN[i].member)   - (long)(q->cpuN[i].member))
#define RAWTOTAL(member) (long)((long)(p->cpu_total.member) - (long)(q->cpu_total.member))

#define BANNER(pad,x,string) {mvwhline(pad, x, 0, ACS_HLINE,COLS-2); \
        wattron(pad,A_STANDOUT); \
        wprintw(pad," "); \
        wprintw(pad,string); \
        wprintw(pad," "); \
        wattroff(pad,A_STANDOUT); }

void set_timer (int);
struct tm * get_timer (void);
time_t get_timer_t (void);
char * timestamp (int, int);
char *dskgrp(struct disk_brk * disk_brk, int i);
inline int NEWDISKGROUP (int, struct disk_brk *);
#ifdef POWER
void get_endian(struct cpuinfo_brk *);
#endif
void init_cpuinfo_brk(struct cpuinfo_brk * cpuinfo_brk);
void find_release(struct cpuinfo_brk * cpuinfo_brk);
void get_cpu_cnt(struct cpuinfo_brk * cpuinfo_brk);
#ifdef X86
void get_intel_spec();
#endif //X86
inline int get_progress_num(struct data * p);
inline int get_progess_data(struct global_data * g_data);
void init_data(struct data * p, struct data * q);
void get_all_data(void);
inline void update_lparcfg_data (struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk);
int read_vmstat(struct mem_brk * mem_brk);
void refresh_all_data(struct cpuinfo_brk * cpuinfo_brk, \
                struct lparcfg_brk * lparcfg_brk, struct net_brk * net_brk, struct nfs_brk * nfs_brk, \
                     struct mem_brk * mem_brk, struct kernel_brk * kernel_brk, struct disk_brk * disk_brk, struct top_brk * top_brk, struct smp_brk * smp_brk);
FILE * open_log_file (struct global_data * g_data, int varperftmp);
void jfs_load (struct jfs_brk * jfs_brk, int load);
int cpu_compare (const void * a, const void * b);
int size_compare (const void * a, const void * b);
int disk_compare (const void * a, const void * b);

int is_dgroup_name(struct disk_brk *, char *);
void list_dgroup(struct disk_brk *, struct dsk_stat *);
void load_dgroup(struct disk_brk *, struct dsk_stat *);
void save_snap(struct cpuinfo_brk * cpuinfo_brk, double user, double kernel, double iowait, double idle, double steal);
void plot_snap(struct cpuinfo_brk * cpuinfo_brk);
void snap_clear(struct cpuinfo_brk *);
inline int snap_average(struct cpuinfo_brk *);

void save_smp_show(struct smp_brk * smp_brk, int cpu_no, int row, long user, long kernel, long iowait, long idle, long nice, long irq, long softirq, long steal);
void save_smp_save(struct smp_brk * smp_brk, int cpu_no, int row, long user, long kernel, long iowait, long idle, long nice, long irq, long softirq, long steal);
void plot_smp_show(struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk, int cpu_no, int row, double user, double kernel, double iowait, double idle, double steal);
void plot_smp_save(struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk, int cpu_no, int row, double user, double kernel, double iowait, double idle, double steal);
#endif
