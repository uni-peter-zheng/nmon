#ifndef __STRUCT_H
#define __STRUCT_H

#include "stat.h"

struct cpu_snap {
	double user;
	double kernel;
	double iowait;
	double idle;
	double steal;
};

struct jfs {
        char name[JFSNAMELEN];
        char device[JFSNAMELEN];
        char type[JFSNAMELEN];
        int  fd;
        int  mounted;
};

struct nfs_stat {
        long v2c[NFS_V2_NAMES_COUNT];   /* verison 2 client */
        long v3c[NFS_V3_NAMES_COUNT];   /* verison 3 client */
        long v4c[NFS_V4C_NAMES_COUNT];  /* verison 4 client */
        long v2s[NFS_V2_NAMES_COUNT];   /* verison 2 SERVER */
        long v3s[NFS_V3_NAMES_COUNT];   /* verison 3 SERVER */
        long v4s[NFS_V4S_NAMES_COUNT];  /* verison 4 SERVER */
};

#define NETMAX 32
struct net_stat {
        unsigned long if_name[17];
        unsigned long long if_ibytes;
        unsigned long long if_obytes;
        unsigned long long if_ibits;
        unsigned long long if_obits;
        unsigned long long if_ipackets;
        unsigned long long if_opackets;
        unsigned long if_ierrs;
        unsigned long if_oerrs;
        unsigned long if_idrop;
        unsigned long if_ififo;
        unsigned long if_iframe;
        unsigned long if_odrop;
        unsigned long if_ofifo;
        unsigned long if_ocarrier;
        unsigned long if_ocolls;
} ;
#ifdef PARTITIONS
#define PARTMAX 256
struct part_stat {
        int part_major;
        int part_minor;
        unsigned long part_blocks;
        char part_name[16];
        unsigned long part_rio;
        unsigned long part_rmerge;
        unsigned long part_rsect;
        unsigned long part_ruse;
        unsigned long part_wio;
        unsigned long part_wmerge;
        unsigned long part_wsect;
        unsigned long part_wuse;
        unsigned long part_run;
        unsigned long part_use;
        unsigned long part_aveq;
};
#endif /*PARTITIONS*/

struct cpu_stat {
        long long user;
        long long sys;
        long long wait;
        long long idle;
        long long irq;
        long long softirq;
        long long steal;
        long long nice;
        long long intr;
        long long ctxt;
        long long btime;
        long long procs;
        long long running;
        long long blocked;
        float uptime;
        float idletime;
        float mins1;
        float mins5;
        float mins15;
};

#define ulong unsigned long
struct dsk_stat {
        char    dk_name[32];
        int     dk_major;
        int     dk_minor;
        long    dk_noinfo;
        ulong   dk_reads;
        ulong   dk_rmerge;
        ulong   dk_rmsec;
        ulong   dk_rkb;
        ulong   dk_writes;
        ulong   dk_wmerge;
        ulong   dk_wmsec;
        ulong   dk_wkb;
        ulong   dk_xfers;
        ulong   dk_bsize;
        ulong   dk_time;
        ulong   dk_inflight;
        ulong   dk_backlog;
        ulong   dk_partition;
        ulong   dk_blocks; /* in /proc/partitions only */
        ulong   dk_use;
        ulong   dk_aveq;
};

struct mem_stat {
        long memtotal;
        long memfree;
        long memshared;
        long buffers;
        long cached;
        long swapcached;
        long active;
        long inactive;
        long hightotal;
        long highfree;
        long lowtotal;
        long lowfree;
        long swaptotal;
        long swapfree;
#ifdef LARGEMEM
        long dirty;
        long writeback;
        long mapped;
        long slab;
        long committed_as;
        long pagetables;
        long hugetotal;
        long hugefree;
        long hugesize;
#else
        long bigfree;
#endif /*LARGEMEM*/
};

struct vm_stat {
        long long nr_dirty;
        long long nr_writeback;
        long long nr_unstable;
        long long nr_page_table_pages;
        long long nr_mapped;
        long long nr_slab;
        long long pgpgin;
        long long pgpgout;
        long long pswpin;
        long long pswpout;
        long long pgalloc_high;
        long long pgalloc_normal;
        long long pgalloc_dma;
        long long pgfree;
        long long pgactivate;
        long long pgdeactivate;
        long long pgfault;
        long long pgmajfault;
        long long pgrefill_high;
        long long pgrefill_normal;
        long long pgrefill_dma;
        long long pgsteal_high;
        long long pgsteal_normal;
        long long pgsteal_dma;
        long long pgscan_kswapd_high;
        long long pgscan_kswapd_normal;
        long long pgscan_kswapd_dma;
        long long pgscan_direct_high;
        long long pgscan_direct_normal;
        long long pgscan_direct_dma;
        long long pginodesteal;
        long long slabs_scanned;
        long long kswapd_steal;
        long long kswapd_inodesteal;
        long long pageoutrun;
        long long allocstall;
        long long pgrotated;
};

struct data {
        struct dsk_stat *dk;
        struct cpu_stat cpu_total;
        struct cpu_stat cpuN[CPUMAX];
        struct mem_stat mem;
        struct vm_stat vm;
        struct nfs_stat nfs;
        struct net_stat ifnets[NETMAX];
#ifdef PARTITIONS
        struct part_stat parts[PARTMAX];
#endif /*PARTITIONS*/

        struct timeval tv;
        double time;
        struct procsinfo * procs;

        int    nprocs;
};

struct procsinfo {
        int pi_pid;
        char pi_comm[64];
        char pi_state;
        int pi_ppid;
        int pi_pgrp;
        int pi_session;
        int pi_tty_nr;
        int pi_tty_pgrp;
        unsigned long pi_flags;
        unsigned long pi_minflt;
        unsigned long pi_cmin_flt;
        unsigned long pi_majflt;
        unsigned long pi_cmaj_flt;
        unsigned long pi_utime;
        unsigned long pi_stime;
        long pi_cutime;
        long pi_cstime;
        long pi_pri;
        long pi_nice;
#ifndef KERNEL_2_6_18
        long junk /* removed */;
#else
        long pi_num_threads;
#endif
        long pi_it_real_value;
        unsigned long pi_start_time;
        unsigned long pi_vsize;
        long pi_rss; /* - 3 */
        unsigned long pi_rlim_cur;
        unsigned long pi_start_code;
        unsigned long pi_end_code;
        unsigned long pi_start_stack;
        unsigned long pi_esp;
        unsigned long pi_eip;
        /* The signal information here is obsolete. */
        unsigned long pi_pending_signal;
        unsigned long pi_blocked_sig;
        unsigned long pi_sigign;
        unsigned long pi_sigcatch;
        unsigned long pi_wchan;
        unsigned long pi_nswap;
        unsigned long pi_cnswap;
        int pi_exit_signal;
        int pi_cpu;
#ifdef KERNEL_2_6_18
        unsigned long pi_rt_priority;
        unsigned long pi_policy;
        unsigned long long pi_delayacct_blkio_ticks;
#endif
        unsigned long statm_size;       /* total program size */
        unsigned long statm_resident;   /* resident set size */
        unsigned long statm_share;      /* shared pages */
        unsigned long statm_trs;        /* text (code) */
        unsigned long statm_drs;        /* data/stack */
        unsigned long statm_lrs;        /* library */
        unsigned long statm_dt;         /* dirty pages */

        unsigned long long read_io;     /* storage read bytes */
        unsigned long long write_io;    /* storage write bytes */
};

struct proc_file {
        FILE *fp;
        char *filename;
        int size;
        int lines;
        char *line[PROC_MAXLINES];
        char *buf;
        int read_this_interval; /* track updates for each update to stop  double data collection */
};

#ifdef POWER
struct lparcfg_struct {
        char version_string[16];                /*lparcfg 1.3 */
        int version;
        char serial_number[16];                 /*HAL,0210033EA*/
        char system_type[64];                   /*HAL,9124-720*/
        /* new record is "IBM pSeries (emulated by qemu)" instead of "IBM 9119-MME" */
        int  partition_id;                      /*11*/
        /*
           R4=0x14
           R5=0x0
           R6=0x800b0000
           R7=0x1000000040004
           */
        int BoundThrds;                         /*=1*/
        int CapInc;                             /*=1*/
        long long DisWheRotPer;                 /*=2070000*/
        int MinEntCap;                          /*=10*/
        int MinEntCapPerVP;                     /*=10*/
        int MinMem;                             /*=2048*/
        int DesMem;                             /*=4096*/
        int MinProcs;                           /*=1*/
        int partition_max_entitled_capacity;    /*=400*/
        int system_potential_processors;        /*=4*/
        /**/
        int partition_entitled_capacity;        /*=20*/
        int system_active_processors;           /*=4*/
        int pool_capacity;                      /*=4*/
        int unallocated_capacity_weight;        /*=0*/
        int capacity_weight;                    /*=0*/
        int capped;                             /*=1*/
        int unallocated_capacity;               /*=0*/
        long long pool_idle_time;               /*=0*/
        long long pool_idle_saved;
        long long pool_idle_diff;
        int pool_num_procs;                     /*=0*/
        long long purr;                         /*=0*/
        long long purr_saved;
        long long purr_diff;
        long long timebase;
        int partition_active_processors;        /*=1*/
        int partition_potential_processors;     /*=40*/
        int shared_processor_mode;              /*=1*/
        int smt_mode;                           /* 1: off, 2: SMT-2, 4: SMT-4 */
        int cmo_enabled;                        /* 1 means AMS is Active */
        int entitled_memory_pool_number;        /*  pool number = 0 */
        int entitled_memory_weight;             /* 0 to 255 */
        long cmo_faults;                        /* Hypervisor Page-in faults = big number */
        long cmo_faults_save;                   /* above saved */
        long cmo_faults_diff;                   /* delta */
        long cmo_fault_time_usec;               /* Hypervisor time in micro seconds = big */
        long cmo_fault_time_usec_save;          /* above saved */
        long cmo_fault_time_usec_diff;          /* delta */
        long backing_memory;            /* AIX pmem in bytes */
        long cmo_page_size;             /* AMS page size in bytes */
        long entitled_memory_pool_size; /* AMS whole pool size in bytes */
        long entitled_memory_loan_request;      /* AMS requesting more memory loaning */

#ifdef EXPERIMENTAL
        /* new data in SLES11 for POWER 2.6.27 (may be a little earlier too) */
        long DesEntCap;
        long DesProcs;
        long DesVarCapWt;
        long DedDonMode;
        long group;
        long pool;
        long entitled_memory;
        long entitled_memory_group_number;
        long unallocated_entitled_memory_weight;
        long unallocated_io_mapping_entitlement;
        /* new data in SLES11 for POWER 2.6.27 */
#endif /* EXPERIMENTAL */
};
#endif

struct global_data {
		WINDOW * pad;

		struct data * p;
        struct data * q;
        struct data database[2];
        struct proc_file proc[P_NUMBER];
        FILE * fp_ss;
        double  elapsed;                /* actual seconds between screen updates */
        int stat8; /* used to determine the number of variables on a line */
        int seconds;
        int maxloops;
        struct utsname uts;             /* UNIX name, version, etc */
        char hostname[256];
		char user_filename[512];
		char user_filename_set;
		char log_path[512];
        char run_name[256];
        int run_name_set;
        char fullhostname[256];
/* Global name of programme for printing it */
        char * progname;
        int argc;
        char ** argv;
        int show_aaa;
        int show_all;
        int loop;
        int show_rrd;
        int colour;
        int x;
        int y;
        int log;
        int proc_first_time;
        int isroot;
		int bbbr_line;
		int cur_line;

		int update_data;
		int change_show;
};

struct topper {
        int     index;
        int     other;
        double  size;
        double  io;
        int     time;
};

struct disk_brk {
        int disks;
        int diskmax;
        int show_disk;
        int     show_diskmap;
        int show_dgroup;
        int disk_mode;
        int disk_first_time;
        double disk_total;
        double disk_busy;
        double disk_read;
        double disk_size;
        double disk_write;
        double disk_xfers;
        int disk_only_mode;
        double readers;
        double writers;
        char   *dgroup_filename;
        char   *dgroup_name[DGROUPS];
        int   *dgroup_data;
        int   dgroup_disks[DGROUPS];
        int   dgroup_total_disks;
        int   dgroup_total_groups;
        int dgroup_loaded;
        int auto_dgroup;
		char * disk_busy_map_ch;

        int extended_disk;
        int disks_per_line;

        double * disk_busy_peak;
        double * disk_rate_peak;

        struct global_data * ext;
};

#ifdef POWER
struct lparcfg_brk
{
        int lparcfg_processed;
        int lpar_first_time;
        int show_lpar;
        struct lparcfg_struct lparcfg;
        int lparcfg_reread;
		int lpar_sanity;
		int result;
		char lpar_buffer[LPAR_LINE_MAX][LPAR_LINE_WIDTH];
		int lpar_count;
		
        struct global_data * ext;
};
#endif

struct cpuinfo_brk
{
        int show_cpu;
        int show_longterm;
        int show_verbose;
#ifdef POWER
        char endian[15];
        int power_vm_type;
#endif
        char * lsb_release[5];
        char * easy[5];
        int cpus;
#ifdef X86
        int   cores;
        int   siblings;
        int processorchips;
        int   hyperthreads;
        char * vendor_ptr;
        char * model_ptr;
        char * mhz_ptr;
        char * bogo_ptr;
#endif
        int old_cpus;   /* Number of CPU seen in previuos interval */
        int     max_cpus;       /* highest number of CPUs in DLPAR */
        double  cpu_sum;
        double  cpu_busy;
        int cpu_idle;
        int cpu_user;
        int cpu_sys;
        int cpu_wait;
        int cpu_steal;
        double * cpu_peak; /* ptr to array  - 1 for each cpu - 0 = average for machine */
		struct cpu_snap cpu_snap[MAX_SNAPS];
		int next_cpu_snap;
		int cpu_snap_all;
		int dotline;

        struct global_data * ext;
};

struct top_brk {
        int show_top;
        int show_topmode;
        int show_args;
        int topper_size;
        int max_sorted;
        int top_first_time;
        struct topper * topper;
		int collect_top;
		int collect_topmode;
        int cmdfound;
        int skipped;
		int show_count;
		int cur_ps;
        char *cmdlist[CMDMAX];

        struct global_data * ext;
};

struct mem_brk {
        int show_vm;
        int show_memory;
        int vm_first_time; /* default: 1 */
        unsigned long pagesize;
		int result;

        struct global_data * ext;
};

struct kernel_brk {
        int show_kernel;
		
        struct global_data * ext;
};

struct large_brk {
        int show_large;
        int first_huge;
        long huge_peak;
		
        struct global_data * ext;
};

struct jfs_brk {
        int show_jfs;
        struct jfs jfs[JFSMAX];
        struct statfs statfs_buffer;
        int jfses;
        int jfs_loaded;
	
        struct global_data * ext;
};

struct net_brk {
        int show_net;
        int     show_neterror;
        int networks;
        int errors;
        double net_read_peak[NETMAX];
        double net_write_peak[NETMAX];
        double net_read_peak_by_bits[NETMAX];
        double net_write_peak_by_bits[NETMAX];
		
        struct global_data * ext;
};

struct welcome_brk {
        int welcome;
        struct global_data * ext;
};

struct help_brk {
        int show_help;
        struct global_data * ext;
};

struct nfs_brk {
        int show_nfs;
        int nfs_first_time; /* default: 1*/
        int nfs_v2c_found;
        int nfs_v2s_found;
        int nfs_v3c_found;
        int nfs_v3s_found;
        int nfs_v4c_found;
        int nfs_v4s_found;
        int nfs_clear;
        int nfs_v4s_names_count;
        int nfs_v4c_names_count;
        char * nfs_v4c_names[NFS_V4C_NAMES_COUNT];
        char * nfs_v4s_names[NFS_V4S_NAMES_COUNT];
        char * nfs_v3_names[22];
        char * nfs_v2_names[NFS_V2_NAMES_COUNT];

        struct global_data * ext;
};

struct smp_brk {
        int show_smp;
        int smp_first_time; /*1*/
        int show_raw;
        char * cpu_line;
        struct global_data * ext;
};
#endif
