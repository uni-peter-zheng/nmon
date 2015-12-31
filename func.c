/*************************************************************************
  > File Name: func.c
  > Author: ToLiMit
  > Mail: 348958453@qq.com
  > Created Time: Wed 28 Oct 2015 02:11:07 PM CST
 ************************************************************************/

#include "lmon.h"
#include "proc.h"
#include "func.h"
#include "stat.h"
#include "struct.h"

time_t timer;

char *month[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

void set_timer (int num)
{
	timer = time(0);
}

struct tm * get_timer (void)
{
	return localtime (&timer);
}

time_t get_timer_t (void)
{
	return timer;
}

char * timestamp (int loop, int show_rrd)
{
	static char string[64];
	if(show_rrd)
		sprintf(string,"%ld",(long)timer);
	else
		sprintf(string,"T%04d",loop);
	return string;
}

char * save_word(char *in, char *out)
{
	int   len;
	int   i;
	len = strlen(in);
	out[0] = 0;
	for (i = 0; i < len; i++) {
		if ( isalnum(in[i]) || in[i] == '_' || in[i] == '-' || in[i] == '/' ) {
			out[i] = in[i];
			out[i+1] = 0;
		} else
			break;
	}
	for (; i < len; i++)
		if (isalnum(in[i]))
			return &in[i];
	return &in[i];
}

void linux_bbbp(FILE * fp_ss, char *name, char *cmd, char *err)
{
	int   i;
	int   len;
#define STRLEN 4096
	char   str[STRLEN];
	FILE * pop;
	static int   lineno = 0;

	pop = popen(cmd, "r");
	if (pop == NULL) {
		fprintf(fp_ss, "BBBP,%03d,%s failed to run %s\n", lineno++, cmd, err);
	} else {
		fprintf(fp_ss, "BBBP,%03d,%s\n", lineno++, name);
		for (i = 0; i < 2048 && (fgets(str, STRLEN, pop) != NULL); i++) { /* 2048=sanity check only */
			len = strlen(str);
			if(len>STRLEN) len=STRLEN;
			if (str[len-1] == '\n') /*strip off the newline */
				str[len-1] = 0;
			/* fix lsconf style output so it does not confuse spread sheets */
			if(str[0] == '+') str[0]='p';
			if(str[0] == '*') str[0]='m';
			if(str[0] == '-') str[0]='n';
			if(str[0] == '/') str[0]='d';
			if(str[0] == '=') str[0]='e';
			fprintf(fp_ss, "BBBP,%03d,%s,\"%s\"\n", lineno++, name, str);
		}
		pclose(pop);
	}
}

char * dskgrp(struct disk_brk * disk_brk, int i)
{
	static char error_string[] = { "Too-Many-Disks" };
	static char *string[16] = {"",   "1",  "2",  "3",
		"4",  "5",  "6",  "7",
		"8",  "9",  "10", "11",
		"12", "13", "14", "15"};

	i = (int)((float)i/(float)disk_brk->disks_per_line);
	if(0 <= i && i <= 15 )
		return string[i];
	return error_string;
}


void *mymalloc(int size, int line)
{
	void * ptr;
	ptr= malloc(size);
	fprintf(stderr,"0x%x = malloc(%d) at line=%d\n", ptr, size, line);
	return ptr;
}

void myfree(void *ptr,int line)
{
	fprintf(stderr,"free(0x%x) at line=%d\n", ptr, line);
	free(ptr);
}

void *myrealloc(void *oldptr, int size, int line)
{
	void * ptr;
	ptr= realloc(oldptr,size);
	fprintf(stderr,"0x%x = realloc(0x%x, %d) at line=%d\n", ptr, oldptr, size, line);
	return ptr;
}

inline int NEWDISKGROUP (int disk, struct disk_brk * brk)
{
	return ((disk) % (brk->disks_per_line) == 0);
}

void get_endian(struct cpuinfo_brk * brk)
{
	FILE *pop;
	char tmpstr[64];

	pop = popen("/usr/bin/lscpu | grep Byte", "r");
	if(pop != NULL) {
		if(fgets(tmpstr, 63, pop) != NULL) {
			tmpstr[strlen(tmpstr)-1]=0; /* remove newline */
			brk->endian[0]=0;
			strncpy(brk->endian,&tmpstr[23],14);
		}
		pclose(pop);
	}
}

void init_cpuinfo_brk (struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;

	find_release(cpuinfo_brk);
	get_cpu_cnt(cpuinfo_brk);
	cpuinfo_brk->max_cpus= cpuinfo_brk->old_cpus = cpuinfo_brk->cpus;
}

void __find_release(struct cpuinfo_brk * cpuinfo_brk)
{
	FILE *pop;
	int i;
	char tmpstr[71];

	pop = popen("cat /etc/*ease 2>/dev/null", "r");
	if(pop != NULL) {
		tmpstr[0]=0;
		for(i=0;i<4;i++) {
			if(fgets(tmpstr, 70, pop) == NULL)
				break;
			tmpstr[strlen(tmpstr)-1]=0; /* remove newline */
			cpuinfo_brk->easy[i] = malloc(strlen(tmpstr)+1);
			strcpy(cpuinfo_brk->easy[i],tmpstr);
		}
		pclose(pop);
	}
	pop = popen("/usr/bin/lsb_release -idrc 2>/dev/null", "r");
	if(pop != NULL) {
		tmpstr[0]=0;
		for(i=0;i<4;i++) {
			if(fgets(tmpstr, 70, pop) == NULL)
				break;
			tmpstr[strlen(tmpstr)-1]=0; /* remove newline */
			cpuinfo_brk->lsb_release[i] = malloc(strlen(tmpstr)+1);
			strcpy(cpuinfo_brk->lsb_release[i],tmpstr);
		}
		pclose(pop);
	}
}

void find_release(struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;

	__find_release(cpuinfo_brk);
}

void get_cpu_cnt(struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;
	int i;

	struct proc_file * proc = cpuinfo_brk->ext->proc;
	/* Start with index [1] as [0] contains overall CPU statistics */
	for(i=1; i < proc[P_STAT].lines; i++) {
		if(strncmp("cpu",proc[P_STAT].line[i],3) == 0)
			cpuinfo_brk->cpus=i;
		else
			break;
	}
	if(cpuinfo_brk->cpus >= CPUMAX) {
		printf("This nmon supports only %d CPU threads (Logical CPUs) and the machine appears to have %d.\nnmon stopping as its unsafe to continue.\n", CPUMAX, cpuinfo_brk->cpus);
		exit(44);
	}
}

#ifdef X86
void get_intel_spec() {
	int i;
	int physicalcpu[256];
	int id;

	/* Get CPU info from /proc/stat and populate proc[P_STAT] */

	for(i=0; i<256;i++)
		physicalcpu[i]=0;

	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("vendor_id",proc[P_CPUINFO].line[i],9) == 0) {
			vendor_ptr = &proc[P_CPUINFO].line[i][12];
		}
	}
	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("model name",proc[P_CPUINFO].line[i],10) == 0) {
			model_ptr = &proc[P_CPUINFO].line[i][13];
		}
	}
	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("cpu MHz",proc[P_CPUINFO].line[i],7) == 0) {
			mhz_ptr = &proc[P_CPUINFO].line[i][11];
		}
	}
	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("bogomips",proc[P_CPUINFO].line[i],8) == 0) {
			bogo_ptr = &proc[P_CPUINFO].line[i][11];
		}
	}

	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("physical id",proc[P_CPUINFO].line[i],11) == 0) {
			id = atoi(&proc[P_CPUINFO].line[i][15]);
			if(id<256)
				physicalcpu[id] = 1;
		}
	}
	for(i=0; i<256;i++)
		if(physicalcpu[i] == 1)
			processorchips++;

	/* Start with index [1] as [0] contains overall CPU statistics */
	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("siblings",proc[P_CPUINFO].line[i],8) == 0) {
			siblings = atoi(&proc[P_CPUINFO].line[i][11]);
			break;
		}
	}
	for(i=0; i<proc[P_CPUINFO].lines; i++) {
		if(strncmp("cpu cores",proc[P_CPUINFO].line[i],9) == 0) {
			cores = atoi(&proc[P_CPUINFO].line[i][12]);
			break;
		}
	}
	if(siblings>cores)
		hyperthreads=siblings/cores;
	else
		hyperthreads=0;
}
#endif
int get_progress_data (struct global_data * g_data)
{
	if (g_data == NULL)
		return 0;
	int n = 0;
	struct data * p = g_data->p;
	void * tmp_p = NULL;
	int count = 0;

	count = getprocs(g_data, 0, g_data->p);
	if (count > p->nprocs) {
		n = count + 128; /* allow for growth in the number of processes in the mean time */
		while ((tmp_p = realloc(p->procs,(sizeof(struct procsinfo) * (n+1)))) == NULL) /* add one to avoid overrun */
			;
		p->procs = (struct procsinfo *)tmp_p;
		p->nprocs = n;
	}
	count = getprocs(g_data, 1, g_data->p);
	return count;
}

inline int get_progress_num (struct data * p)
{
	if (p == NULL)
		return 0;
	else
		return p->nprocs;
}

void update_net_data(struct net_brk * net_brk)
{
	if (net_brk == NULL)
		return;
	int i;
	struct data * p = net_brk->ext->p;
	struct data * q = net_brk->ext->q;
	double elapsed = net_brk->ext->elapsed;
#define IFDELTA(member) ((float)( (q->ifnets[i].member > p->ifnets[i].member) ? 0 : (p->ifnets[i].member - q->ifnets[i].member)/elapsed) )
#define IFDELTA_ZERO(member1,member2) ((IFDELTA(member1) == 0) || (IFDELTA(member2)== 0)? 0.0 : IFDELTA(member1)/IFDELTA(member2) )

	net_brk->errors = 0;
	for (i = 0; i < net_brk->networks; i++) {
		if(net_brk->net_read_peak[i] < IFDELTA(if_ibytes) / 1024.0)
			net_brk->net_read_peak[i] = IFDELTA(if_ibytes) / 1024.0;
		if(net_brk->net_write_peak[i] < IFDELTA(if_obytes) / 1024.0)
			net_brk->net_write_peak[i] = IFDELTA(if_obytes) / 1024.0;
		if(net_brk->net_read_peak_by_bits[i] < IFDELTA(if_ibits) / 1024.0)
			net_brk->net_read_peak_by_bits[i] = IFDELTA(if_ibits) / 1024.0;
		if(net_brk->net_write_peak_by_bits[i] < IFDELTA(if_obits) / 1024.0)
			net_brk->net_write_peak_by_bits[i] = IFDELTA(if_obits) / 1024.0;
		net_brk->errors += p->ifnets[i].if_ierrs - q->ifnets[i].if_ierrs
			+ p->ifnets[i].if_oerrs - q->ifnets[i].if_oerrs
			+ p->ifnets[i].if_ocolls - q->ifnets[i].if_ocolls;
	}
}

void init_data (struct data * p, struct data * q)
{
	if (q == NULL || p == NULL)
		return;
	/* Initialise the time stamps for the first loop */
	p->time = doubletime();
	q->time = doubletime();
}

void refresh_all_data (struct cpuinfo_brk * cpuinfo_brk, \
		struct lparcfg_brk * lparcfg_brk, struct net_brk * net_brk, struct nfs_brk * nfs_brk, \
		struct mem_brk * mem_brk, struct kernel_brk * kernel_brk, struct disk_brk * disk_brk, struct top_brk * top_brk, struct smp_brk * smp_brk)
{
	if (cpuinfo_brk == NULL || net_brk == NULL || nfs_brk == NULL || mem_brk == NULL || kernel_brk == NULL || disk_brk == NULL || top_brk == NULL || smp_brk == NULL)
		return;

	int n = 0;
	struct global_data * g_data = cpuinfo_brk->ext;
	void * tmp_p = NULL;
	switcher();
	n = get_progress_data(g_data);
	if (n != 0)
		top_brk->cur_ps = n;
	if (top_brk->topper_size < g_data->p->nprocs) {
		while ((tmp_p = realloc(top_brk->topper, sizeof(struct topper ) * (g_data->p->nprocs + 1))) == NULL) /* add one to avoid overrun */
			;
		top_brk->topper = (struct topper *)tmp_p;
		top_brk->topper_size = g_data->p->nprocs;
	}

	proc_read(g_data->proc, P_STAT);
	proc_cpu(cpuinfo_brk);
	proc_read(g_data->proc, P_VERSION);
	proc_read(g_data->proc, P_CPUINFO);
	proc_read(g_data->proc, P_MEMINFO);
	proc_read(g_data->proc, P_UPTIME);
	proc_read(g_data->proc, P_LOADAVG);
	proc_read(g_data->proc, P_NFS);
	proc_read(g_data->proc, P_NFSD);
	proc_read(g_data->proc, P_VMSTAT);

	/* Get memory info */
	mem_brk->result = read_vmstat(mem_brk);
	proc_mem(mem_brk);
#ifdef POWER
	proc_lparcfg(cpuinfo_brk, lparcfg_brk);
#endif
	proc_nfs(nfs_brk);
	proc_kernel(kernel_brk);
	proc_net(net_brk);
	update_net_data(net_brk);
	proc_disk(disk_brk);
}


void write_header_lines (char * user_filename, struct top_brk * top_brk, struct jfs_brk * jfs_brk, \
		struct net_brk * net_brk, struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk, \
		struct disk_brk * disk_brk)
{
	if (top_brk == NULL || jfs_brk == NULL || net_brk == NULL || lparcfg_brk == NULL || \
			cpuinfo_brk == NULL || disk_brk == NULL)
		return;

	int k,i;
	FILE * fp_ss = cpuinfo_brk->ext->fp_ss;
	int argc = cpuinfo_brk->ext->argc;
	struct tm *tim; /* used to work out the hour/min/second */
	struct data * p = cpuinfo_brk->ext->p;
	int tm_year = 0;
	int tm_mon = 0;

	set_timer(0);
	tim = get_timer();
	tim->tm_year += 1900 - 2000;  /* read localtime() manual page!! */
	tim->tm_mon  += 1; /* because it is 0 to 11 */
	tm_year = tim->tm_year;
	tm_mon = tim->tm_mon;

	fprintf(fp_ss,"AAA,progname,%s\n", cpuinfo_brk->ext->progname);
	fprintf(fp_ss,"AAA,command,");
	for(i=0;i<argc;i++)
		fprintf(fp_ss,"%s ",cpuinfo_brk->ext->argv[i]);
	fprintf(fp_ss,"\n");
	fprintf(fp_ss,"AAA,version,%s\n", VERSION);
	fprintf(fp_ss,"AAA,disks_per_line,%d\n", disk_brk->disks_per_line);
	fprintf(fp_ss,"AAA,max_disks,%d,set by -d option\n", disk_brk->diskmax);
	fprintf(fp_ss,"AAA,disks,%d,\n", disk_brk->disks);

	fprintf(fp_ss,"AAA,host,%s\n", cpuinfo_brk->ext->hostname);
	fprintf(fp_ss,"AAA,user,%s\n", getenv("USER"));
	fprintf(fp_ss,"AAA,OS,Linux,%s,%s,%s\n", cpuinfo_brk->ext->uts.release, cpuinfo_brk->ext->uts.version, cpuinfo_brk->ext->uts.machine);
	fprintf(fp_ss,"AAA,runname,%s\n", cpuinfo_brk->ext->run_name);
	fprintf(fp_ss,"AAA,time,%02d:%02d.%02d\n", tim->tm_hour, tim->tm_min, tim->tm_sec);
	fprintf(fp_ss,"AAA,date,%02d-%3s-%02d\n", tim->tm_mday, month[tm_mon-1], tm_year + 2000);
	fprintf(fp_ss,"AAA,interval,%d\n", cpuinfo_brk->ext->seconds);
	fprintf(fp_ss,"AAA,snapshots,%d\n", cpuinfo_brk->ext->maxloops);
#ifdef POWER
	fprintf(fp_ss,"AAA,cpus,%d,%d\n", cpuinfo_brk->cpus / lparcfg_brk->lparcfg.smt_mode, cpuinfo_brk->cpus);        /* physical CPU, logical CPU */
	fprintf(fp_ss,"AAA,CPU ID length,3\n"); /* Give analyzer a chance to easily find length of CPU number - 3 digits here! */
#else
	fprintf(fp_ss,"AAA,cpus,%d\n", cpuinfo_brk->cpus);
#endif
#ifdef X86
	fprintf(fp_ss,"AAA,x86,VendorId,%s\n",       vendor_ptr);
	fprintf(fp_ss,"AAA,x86,ModelName,%s\n",      model_ptr);
	fprintf(fp_ss,"AAA,x86,MHz,%s\n",            mhz_ptr);
	fprintf(fp_ss,"AAA,x86,bogomips,%s\n",       bogo_ptr);
	fprintf(fp_ss,"AAA,x86,ProcessorChips,%d\n", processorchips);
	fprintf(fp_ss,"AAA,x86,Cores,%d\n",          cores);
	fprintf(fp_ss,"AAA,x86,hyperthreads,%d\n",   hyperthreads);
	fprintf(fp_ss,"AAA,x86,VirtualCPUs,%d\n",    cpuinfo_brk->cpus);
#endif
	fprintf(fp_ss,"AAA,proc_stat_variables,%d\n", cpuinfo_brk->ext->stat8);

	fprintf(fp_ss,"AAA,note0, Warning - use the UNIX sort command to order this file before loading into a spreadsheet\n");
	fprintf(fp_ss,"AAA,note1, The First Column is simply to get the output sorted in the right order\n");
	fprintf(fp_ss,"AAA,note2, The T0001-T9999 column is a snapshot number. To work out the actual time; see the ZZZ section at the end\n");

	for (i = 1; i <=  cpuinfo_brk->cpus; i++)
		fprintf(fp_ss,"CPU%03d,CPU %d %s,User%%,Sys%%,Wait%%,Idle%%,Steal%%\n", i, i, cpuinfo_brk->ext->run_name);
	fprintf(fp_ss,"CPU_ALL,CPU Total %s,User%%,Sys%%,Wait%%,Idle%%,Steal%%,Busy,CPUs\n", cpuinfo_brk->ext->run_name);
	fprintf(fp_ss,"MEM,Memory MB %s,memtotal,hightotal,lowtotal,swaptotal,memfree,highfree,lowfree,swapfree,memshared,cached,active,bigfree,buffers,swapcached,inactive\n", cpuinfo_brk->ext->run_name);

#ifdef POWER
	proc_lparcfg(cpuinfo_brk, lparcfg_brk);
	if(lparcfg_brk->lparcfg.cmo_enabled)
		fprintf(fp_ss,"MEMAMS,AMS %s,Poolid,Weight,Hypervisor-Page-in/s,HypervisorTime(seconds),not_available_1,not_available_2,not_available_3,Physical-Memory(MB),Page-Size(KB),Pool-Size(MB),Loan-Request(KB)\n", cpuinfo_brk->ext->run_name);

#ifdef EXPERIMENTAL
	fprintf(fp_ss,"MEMEXPERIMENTAL,New lparcfg numbers %s,DesEntCap,DesProcs,DesVarCapWt,DedDonMode,group,pool,entitled_memory,entitled_memory_group_number,unallocated_entitled_memory_weight,unallocated_io_mapping_entitlement\n", cpuinfo_brk->ext->run_name);
#endif /* EXPERIMENTAL */
#endif /* POWER */

	fprintf(fp_ss,"PROC,Processes %s,Runnable,Blocked,pswitch,syscall,read,write,fork,exec,sem,msg\n", cpuinfo_brk->ext->run_name);
	/*
	   fprintf(fp_ss,"PAGE,Paging %s,faults,pgin,pgout,pgsin,pgsout,reclaims,scans,cycles\n", run_name);
	   fprintf(fp_ss,"FILE,File I/O %s,iget,namei,dirblk,readch,writech,ttyrawch,ttycanch,ttyoutch\n", run_name);
	   */


	fprintf(fp_ss,"NET,Network I/O %s", cpuinfo_brk->ext->run_name);
	/*for (i = 0; i < net_brk->networks; i++)
		fprintf(fp_ss,",%-2s-read-KB/s", (char *)p->ifnets[i].if_name);
	for (i = 0; i < net_brk->networks; i++)
		fprintf(fp_ss,",%-2s-write-KB/s", (char *)p->ifnets[i].if_name);*/
	for (i = 0; i < net_brk->networks; i++)
		fprintf(fp_ss,",%-2s-read-Kb/s", (char *)p->ifnets[i].if_name);
	for (i = 0; i < net_brk->networks; i++)
		fprintf(fp_ss,",%-2s-write-Kb/s", (char *)p->ifnets[i].if_name);
	fprintf(fp_ss,"\n");
	fprintf(fp_ss,"NETPACKET,Network Packets %s", cpuinfo_brk->ext->run_name);
	for (i = 0; i < net_brk->networks; i++)
		fprintf(fp_ss,",%-2s-read/s,", (char *)p->ifnets[i].if_name);
	for (i = 0; i < net_brk->networks; i++)
		fprintf(fp_ss,",%-2s-write/s,", (char *)p->ifnets[i].if_name);
	/* iremoved as it is not below in the BUSY line fprintf(fp_ss,"\n"); */
#ifdef DEBUG
	if(debug)printf("disks=%d x%sx\n",(char *)disk_brk->disks,p->dk[0].dk_name);
#endif /*DEBUG*/
	for (i = 0; i < disk_brk->disks; i++)  {
		if(NEWDISKGROUP(i, disk_brk))
			fprintf(fp_ss,"\nDISKBUSY%s,Disk %%Busy %s", dskgrp(disk_brk,i) , cpuinfo_brk->ext->run_name);
		fprintf(fp_ss,",%s", (char *)p->dk[i].dk_name);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i, disk_brk))
			fprintf(fp_ss,"\nDISKREAD%s,Disk Read KB/s %s", dskgrp(disk_brk,i),cpuinfo_brk->ext->run_name);
		fprintf(fp_ss,",%s", (char *)p->dk[i].dk_name);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i, disk_brk))
			fprintf(fp_ss,"\nDISKWRITE%s,Disk Write KB/s %s", (char *)dskgrp(disk_brk,i),cpuinfo_brk->ext->run_name);
		fprintf(fp_ss,",%s", (char *)p->dk[i].dk_name);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i, disk_brk))
			fprintf(fp_ss,"\nDISKXFER%s,Disk transfers per second %s", (char *)dskgrp(disk_brk,i),cpuinfo_brk->ext->run_name);
		fprintf(fp_ss,",%s", p->dk[i].dk_name);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i, disk_brk))
			fprintf(fp_ss,"\nDISKBSIZE%s,Disk Block Size %s", dskgrp(disk_brk,i),cpuinfo_brk->ext->run_name);
		fprintf(fp_ss,",%s", (char *)p->dk[i].dk_name);
	}
	if( disk_brk->extended_disk == 1 && disk_brk->disk_mode == DISK_MODE_DISKSTATS )    {
		for (i = 0; i < disk_brk->disks; i++) {
			if(NEWDISKGROUP(i, disk_brk))
				fprintf(fp_ss,"\nDISKREADS%s,Disk Rd/s %s", dskgrp(disk_brk,i),cpuinfo_brk->ext->run_name);
			fprintf(fp_ss,",%s", (char *)p->dk[i].dk_name);
		}
		for (i = 0; i < disk_brk->disks; i++) {
			if(NEWDISKGROUP(i, disk_brk))
				fprintf(fp_ss,"\nDISKWRITES%s,Disk Wrt/s %s", dskgrp(disk_brk,i),cpuinfo_brk->ext->run_name);
			fprintf(fp_ss,",%s", (char *)p->dk[i].dk_name);
		}
	}

	fprintf(fp_ss,"\n");
	list_dgroup(disk_brk, p->dk);
	jfs_load(jfs_brk, LOAD);
	fprintf(fp_ss,"JFSFILE,JFS Filespace %%Used %s", cpuinfo_brk->ext->hostname);
	for (k = 0; k < jfs_brk->jfses; k++) {
		if(jfs_brk->jfs[k].mounted && strncmp(jfs_brk->jfs[k].name,"/proc",5)
				&& strncmp(jfs_brk->jfs[k].name,"/sys",4)
				&& strncmp(jfs_brk->jfs[k].name,"/run/",5)
				&& strncmp(jfs_brk->jfs[k].name,"/dev/",5)
				&& strncmp(jfs_brk->jfs[k].name,"/var/lib/nfs/rpc",16)
		  )  /* /proc gives invalid/insane values */
			fprintf(fp_ss,",%s", jfs_brk->jfs[k].name);
	}
	fprintf(fp_ss,"\n");
	jfs_load(jfs_brk, UNLOAD);
#ifdef POWER
	if (lparcfg_brk->result && lparcfg_brk->lparcfg.shared_processor_mode != 0 && cpuinfo_brk->power_vm_type == VM_POWERVM) {
		fprintf(fp_ss,"LPAR,Shared CPU LPAR Stats %s,PhysicalCPU,capped,shared_processor_mode,system_potential_processors,system_active_processors,pool_capacity,MinEntCap,partition_entitled_capacity,partition_max_entitled_capacity,MinProcs,Logical CPU,partition_active_processors,partition_potential_processors,capacity_weight,unallocated_capacity_weight,BoundThrds,MinMem,unallocated_capacity,pool_idle_time,smt_mode\n",cpuinfo_brk->ext->hostname);

	}
#endif /*POWER*/
	fprintf(fp_ss,"TOP,%%CPU Utilisation\n");
#ifndef KERNEL_2_6_18
	fprintf(fp_ss,"TOP,+PID,Time,%%CPU,%%Usr,%%Sys,Size,ResSet,ResText,ResData,ShdLib,MinorFault,MajorFault,Command\n");
#else
	fprintf(fp_ss,"TOP,+PID,Time,%%CPU,%%Usr,%%Sys,Size,ResSet,ResText,ResData,ShdLib,MinorFault,MajorFault,Command,Threads,IOwaitTime\n");
#endif
	linux_bbbp(fp_ss, "/etc/release",     "/bin/cat /etc/*ease 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lsb_release",      "/usr/bin/lsb_release -a 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "fdisk-l",          "/sbin/fdisk -l 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lsblk",            "/usr/bin/lsblk 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lscpu",            "/usr/bin/lscpu 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lshw",             "/usr/bin/lshw 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/cpuinfo",    "/bin/cat /proc/cpuinfo 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/meminfo",    "/bin/cat /proc/meminfo 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/stat",       "/bin/cat /proc/stat 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/version",    "/bin/cat /proc/version 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/net/dev",    "/bin/cat /proc/net/dev 2>/dev/null", WARNING);
#ifdef POWER
	linux_bbbp(fp_ss, "ppc64_utils - lscfg",        "/usr/sbin/lscfg 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_utils - ls-vdev",      "/usr/sbin/ls-vdev 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_utils - ls-veth",      "/usr/sbin/ls-veth 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_utils - ls-vscsi",     "/usr/sbin/ls-vscsi 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_utils - lsmcode",      "/usr/sbin/lsmcode 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_cpu - smt",    "/usr/sbin/ppc64_cpu --smt 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_cpu - cores",          "/usr/sbin/ppc64_cpu --cores-present 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_cpu - DSCR",           "/usr/sbin/ppc64_cpu --dscr 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_cpu - snooze",         "/usr/sbin/ppc64_cpu --smt-snooze-delay 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_cpu - run-mode",       "/usr/sbin/ppc64_cpu --run-mode 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ppc64_cpu - frequency",      "/usr/sbin/ppc64_cpu --frequency 2>/dev/null", WARNING);

	linux_bbbp(fp_ss, "bootlist -m nmonal -o",      "/usr/sbin/bootlist -m normal -o 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lsslot",             "/usr/sbin/lsslot      2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lparstat -i",                "/usr/sbin/lparstat -i 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lsdevinfo",                  "/usr/sbin/lsdevinfo 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ls-vdev",                    "/usr/sbin/ls-vdev  2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ls-veth",                    "/usr/sbin/ls-veth  2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ls-vscsi",                   "/usr/sbin/ls-vscsi 2>/dev/null", WARNING);

#endif
	linux_bbbp(fp_ss, "/proc/diskinfo",   "/bin/cat /proc/diskinfo 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/diskstats",   "/bin/cat /proc/diskstats 2>/dev/null", WARNING);

	linux_bbbp(fp_ss, "/sbin/multipath",   "/sbin/multipath -l 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/dev/mapper",        "ls -l /dev/mapper 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/dev/mpath",                 "ls -l /dev/mpath 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/dev/dm-*",                  "ls -l /dev/dm-* 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/dev/md*",                   "ls -l /dev/md* 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/dev/sd*",                   "ls -l /dev/sd* 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/partitions", "/bin/cat /proc/partitions 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/1/stat",     "/bin/cat /proc/1/stat 2>/dev/null", WARNING);
#ifndef KERNEL_2_6_18
	linux_bbbp(fp_ss, "/proc/1/statm",    "/bin/cat /proc/1/statm 2>/dev/null", WARNING);
#endif
#ifdef MAINFRAME
	linux_bbbp(fp_ss, "/proc/sysinfo",    "/bin/cat /proc/sysinfo 2>/dev/null", WARNING);
#endif
	linux_bbbp(fp_ss, "/proc/net/rpc/nfs",        "/bin/cat /proc/net/rpc/nfs 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/net/rpc/nfsd",        "/bin/cat /proc/net/rpc/nfsd 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/proc/modules",    "/bin/cat /proc/modules 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "ifconfig",        "/sbin/ifconfig 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/bin/df-m",        "/bin/df -m 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/bin/mount",        "/bin/mount 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "/etc/fstab",    "/bin/cat /etc/fstab 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "netstat -r",    "/bin/netstat -r 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "uptime",    "/usr/bin/uptime  2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "getconf mem_brk.pagesize",    "/usr/bin/getconf PAGESIZE  2>/dev/null", WARNING);

#ifdef POWER
	linux_bbbp(fp_ss, "/proc/ppc64/lparcfg",    "/bin/cat /proc/ppc64/lparcfg 2>/dev/null", WARNING);
	linux_bbbp(fp_ss, "lscfg-v",    "/usr/sbin/lscfg -v 2>/dev/null", WARNING);
#endif
	fflush (NULL);
	//sleep(1); /* to get the first stats to cover this one second and avoids divide by zero issues */
}

inline void update_lparcfg_data (struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk)
{
	if (lparcfg_brk == NULL || cpuinfo_brk == NULL)
		return;

	int i;
	struct proc_file * proc = lparcfg_brk->ext->proc;

	if(lparcfg_brk->lparcfg.timebase == -1) {
		lparcfg_brk->lparcfg.timebase=0;
		for(i=0;i<proc[P_CPUINFO].lines-1;i++) {
			if(!strncmp("timebase",proc[P_CPUINFO].line[i],8)) {
				sscanf(proc[P_CPUINFO].line[i],"timebase : %lld",&lparcfg_brk->lparcfg.timebase);
				break;
			}
		}
	}
	proc_lparcfg(cpuinfo_brk, lparcfg_brk);
}

long long get_vm_value(struct global_data * g_data, char * s)        
{
	int currline;
	int currchar;
	long long result = -1;
	char *check;
	int len;
	int found;

	for(currline=0; currline<g_data->proc[P_VMSTAT].lines; currline++) {
		len = strlen(s);
		for(currchar=0,found=1; currchar<len; currchar++) {
			if( g_data->proc[P_VMSTAT].line[currline][currchar] == 0 ||
					s[currchar] != g_data->proc[P_VMSTAT].line[currline][currchar]) {
				found=0;
				break;
			}
		}
		if(found && g_data->proc[P_VMSTAT].line[currline][currchar] == ' ')      {
			result = strtoll(&g_data->proc[P_VMSTAT].line[currline][currchar+1],&check,10);
			if( *check == g_data->proc[P_VMSTAT].line[currline][currchar+1]) {
				fprintf(stderr,"%s has an unexpected format: >%s<\n", g_data->proc[P_VMSTAT].filename, g_data->proc[P_VMSTAT].line[currline]);
				return -1;
			}
			return result;
		}
	}
	return -1;
}

int read_vmstat(struct mem_brk * mem_brk)
{
	if (mem_brk == NULL)
		return (-1);

	struct proc_file * proc = mem_brk->ext->proc;
	struct data * p = mem_brk->ext->p;

	if( proc[P_VMSTAT].read_this_interval == 0 || proc[P_VMSTAT].lines == 0)
		return (-1);

#define GETVM(g_data,variable) p->vm.variable = get_vm_value((g_data),__STRING(variable) );
	/* Note: if the variable requested is not found in /proc/vmstat then it is set to -1 */
	GETVM(mem_brk->ext,nr_dirty);
	GETVM(mem_brk->ext,nr_writeback);
	GETVM(mem_brk->ext,nr_unstable);
	GETVM(mem_brk->ext,nr_page_table_pages);
	GETVM(mem_brk->ext,nr_mapped);
	GETVM(mem_brk->ext,nr_slab);
	GETVM(mem_brk->ext,pgpgin);
	GETVM(mem_brk->ext,pgpgout);
	GETVM(mem_brk->ext,pswpin);
	GETVM(mem_brk->ext,pswpout);
	GETVM(mem_brk->ext,pgalloc_high);
	GETVM(mem_brk->ext,pgalloc_normal);
	GETVM(mem_brk->ext,pgalloc_dma);
	GETVM(mem_brk->ext,pgfree);
	GETVM(mem_brk->ext,pgactivate);
	GETVM(mem_brk->ext,pgdeactivate);
	GETVM(mem_brk->ext,pgfault);
	GETVM(mem_brk->ext,pgmajfault);
	GETVM(mem_brk->ext,pgrefill_high);
	GETVM(mem_brk->ext,pgrefill_normal);
	GETVM(mem_brk->ext,pgrefill_dma);
	GETVM(mem_brk->ext,pgsteal_high);
	GETVM(mem_brk->ext,pgsteal_normal);
	GETVM(mem_brk->ext,pgsteal_dma);
	GETVM(mem_brk->ext,pgscan_kswapd_high);
	GETVM(mem_brk->ext,pgscan_kswapd_normal);
	GETVM(mem_brk->ext,pgscan_kswapd_dma);
	GETVM(mem_brk->ext,pgscan_direct_high);
	GETVM(mem_brk->ext,pgscan_direct_normal);
	GETVM(mem_brk->ext,pgscan_direct_dma);
	GETVM(mem_brk->ext,pginodesteal);
	GETVM(mem_brk->ext,slabs_scanned);
	GETVM(mem_brk->ext,kswapd_steal);
	GETVM(mem_brk->ext,kswapd_inodesteal);
	GETVM(mem_brk->ext,pageoutrun);
	GETVM(mem_brk->ext,allocstall);
	GETVM(mem_brk->ext,pgrotated);

	return 1;
}

FILE * open_log_file (struct global_data * g_data, int varperftmp)
{
	if (g_data == NULL) {
		perror ("Can`t create log direction.Params error");
		exit(42);
	}
	/* Output the header lines for the spread sheet */
	struct tm *tim; /* used to work out the hour/min/second */
	FILE * fp;
	char * log_dir = DEFAULT_LOG_DIR;

	set_timer(0);
	tim = get_timer();
	tim->tm_year += 1900 - 2000;  /* read localtime() manual page!! */
	tim->tm_mon  += 1; /* because it is 0 to 11 */

	if (NULL == opendir (log_dir)) {
		if (mkdir (log_dir, 0755) < 0) {
			perror ("Can`t create log direction.");
			printf ("nmon: log dir=%s\n", log_dir);
			exit(42);
		}
	}

	if (log_dir[strlen (log_dir) - 1] == '/') {
		sprintf(g_data->log_path, "%s%s_%02d%02d%02d_%02d%02d.nmon", log_dir, g_data->hostname, tim->tm_year, tim->tm_mon, tim->tm_mday, tim->tm_hour, tim->tm_min);
	}
	else {
		sprintf(g_data->log_path, "%s/%s_%02d%02d%02d_%02d%02d.nmon", log_dir, g_data->hostname, tim->tm_year, tim->tm_mon, tim->tm_mday, tim->tm_hour, tim->tm_min);
	}

	if(strlen (g_data->user_filename) != 0)
		strcpy(g_data->log_path, g_data->user_filename);

	if((fp = fopen(g_data->log_path,"w")) ==0 ) {
		perror("nmon: failed to open output file");
		printf("nmon: output filename=%s\n",g_data->log_path);
		exit(42);
	}

	return fp;
}

void jfs_load(struct jfs_brk * jfs_brk, int load)
{
	int i;
	struct stat stat_buffer;
	FILE * mfp; /* FILE pointer for mtab file*/
	struct mntent *mp; /* mnt point stats */
	static int jfs_loaded = 0;

	if(load==LOAD) {
		if(jfs_loaded == 0) {
			mfp = setmntent("/etc/mtab","r");
			for(i=0; i<JFSMAX && (mp = getmntent(mfp) ) != NULL; i++) {
				strncpy(jfs_brk->jfs[i].device, mp->mnt_fsname,JFSNAMELEN);
				strncpy(jfs_brk->jfs[i].name, mp->mnt_dir,JFSNAMELEN);
				strncpy(jfs_brk->jfs[i].type, mp->mnt_type,JFSTYPELEN);
				mp->mnt_fsname[JFSNAMELEN-1]=0;
				mp->mnt_dir[JFSNAMELEN-1]=0;
				mp->mnt_type[JFSTYPELEN-1]=0;
			}
			endfsent();
			jfs_loaded = 1;
			jfs_brk->jfses=i;
		}

		/* 1st or later time - just reopen the mount points */
		for(i=0;i<JFSMAX && jfs_brk->jfs[i].name[0] !=0;i++) {
			if(stat(jfs_brk->jfs[i].name, &stat_buffer) != -1 ) {
				jfs_brk->jfs[i].fd = open(jfs_brk->jfs[i].name, O_RDONLY);
				if(jfs_brk->jfs[i].fd != -1 )
					jfs_brk->jfs[i].mounted = 1;
				else
					jfs_brk->jfs[i].mounted = 0;
			}
			else
				jfs_brk->jfs[i].mounted = 0;
		}
	} else { /* this is an unload request */
		if(jfs_loaded)
			for(i=0;i<JFSMAX && jfs_brk->jfs[i].name[0] != 0;i++) {
				if(jfs_brk->jfs[i].mounted)
					close(jfs_brk->jfs[i].fd);
				jfs_brk->jfs[i].fd=0;
			}
		else
			/* do nothing */ ;
	}
}

int     cpu_compare(const void *a, const void *b)
{
	return (int)(((struct topper *)b)->time - ((struct topper *)a)->time);
}

int     size_compare(const void *a, const void *b)
{
	return (int)((((struct topper *)b)->size - ((struct topper *)a)->size));
}

int     disk_compare(const void *a, const void *b)
{
	return (int)((((struct topper *)b)->io - ((struct topper *)a)->io));
}

void load_dgroup(struct disk_brk * disk_brk, struct dsk_stat *dk)
{
	if (disk_brk == NULL || dk == NULL)
		return;
	FILE * gp;
	char   line[4096];
	char   name[1024];
	int   i, j;
	char   *nextp;

	if (disk_brk->dgroup_loaded == 2)
		return;
	disk_brk->dgroup_data = malloc(sizeof(int)*DGROUPS * DGROUPITEMS);
	for (i = 0; i < DGROUPS; i++)
		for (j = 0; j < DGROUPITEMS; j++)
			disk_brk->dgroup_data[i*DGROUPITEMS+j] = -1;

	gp = fopen(disk_brk->dgroup_filename, "r");

	if (gp == NULL) {
		perror("opening disk group file");
		fprintf(stderr,"ERROR: failed to open %s\n", disk_brk->dgroup_filename);
		exit(9);
	}

	for (disk_brk->dgroup_total_groups = 0;
			fgets(line, 4096-1, gp) != NULL && disk_brk->dgroup_total_groups < DGROUPS;
			disk_brk->dgroup_total_groups++) {
		/* ignore lines starting with # */
		if(line[0] == '#' ) { /* was a comment line */
			/* Decrement dgroup_total_groups by 1 to correct index for next g_data->loop */
			disk_brk->dgroup_total_groups--;
			continue;
		}
		/* save the name */
		nextp = save_word(line, name);
		if(strlen(name) == 0) { /* was a blank line */
			fprintf(stderr,"ERROR nmon:ignoring odd line in diskgroup file \"%s\"\n",line);
			/* Decrement dgroup_total_groups by 1 to correct index for next g_data->loop */
			disk_brk->dgroup_total_groups--;
			continue;
		}
		/* Added +1 to be able to correctly store the terminating \0 character */
		disk_brk->dgroup_name[disk_brk->dgroup_total_groups] = malloc(strlen(name)+1);
		strcpy(disk_brk->dgroup_name[disk_brk->dgroup_total_groups], name);

		/* save the hdisks */
		for (i = 0; i < DGROUPITEMS && *nextp != 0; i++) {
			nextp = save_word(nextp, name);
			for (j = 0; j < disk_brk->disks; j++) {
				if ( strcmp(dk[j].dk_name, name) == 0 ) {
					/*DEBUG printf("DGadd group=%s,name=%s,disk=%s,dgroup_total_groups=%d,dgroup_total_disks=%d,j=%d,i=%d,index=%d.\n",
					  dgroup_name[dgroup_total_groups],
					  name, dk[j].dk_name, dgroup_total_groups, dgroup_total_disks, j, i,dgroup_total_groups*DGROUPITEMS+i);
					  */
					disk_brk->dgroup_data[disk_brk->dgroup_total_groups*DGROUPITEMS+i] = j;
					disk_brk->dgroup_disks[disk_brk->dgroup_total_groups]++;
					disk_brk->dgroup_total_disks++;
					break;
				}
			}
			if (j == disk_brk->disks)
				fprintf(stderr,"ERROR nmon:diskgroup file - failed to find disk=%s for group=%s disks known=%d\n",
						name, disk_brk->dgroup_name[disk_brk->dgroup_total_groups],disk_brk->disks);
		}
	}
	fclose(gp);
	disk_brk->dgroup_loaded = 2;
}


void list_dgroup(struct disk_brk * disk_brk, struct dsk_stat *dk)
{
	if (disk_brk == NULL || dk == NULL)
		return;
	FILE * fp_ss = disk_brk->ext->fp_ss;
	int   i, j, k, n;
	int   first = 1;

	/* DEBUG for (n = 0, i = 0; i < dgroup_total_groups; i++) {
	   fprintf(fp_ss, "CCCG,%03d,%s", n++, dgroup_name[i]);
	   for (j = 0; j < dgroup_disks[i]; j++) {
	   if (dgroup_data[i*DGROUPITEMS+j] != -1) {
	   fprintf(fp_ss, ",%d=%d", j, dgroup_data[i*DGROUPITEMS+j]);
	   }
	   }
	   fprintf(fp_ss, "\n");
	   }
	   */

	for (n = 0, i = 0; i < disk_brk->dgroup_total_groups; i++) {
		if (first) {
			fprintf(fp_ss, "BBBG,%03d,User Defined Disk Groups Name,Disks\n", n++);
			first = 0;
		}
		fprintf(fp_ss, "BBBG,%03d,%s", n++, disk_brk->dgroup_name[i]);
		for (k = 0, j = 0; j < disk_brk->dgroup_disks[i]; j++) {
			if (disk_brk->dgroup_data[i*DGROUPITEMS+j] != -1) {
				fprintf(fp_ss, ",%s", dk[disk_brk->dgroup_data[i*DGROUPITEMS+j]].dk_name);
				k++;
			}
			/* add extra line if we have lots to stop spreadsheet line width problems */
			if (k == 128) {
				fprintf(fp_ss, "\nBBBG,%03d,%s continued", n++, disk_brk->dgroup_name[i]);
			}
		}
		fprintf(fp_ss, "\n");
	}
	fprintf(fp_ss, "DGBUSY,Disk Group Busy %s", disk_brk->ext->hostname);
	for (i = 0; i < DGROUPS; i++) {
		if (disk_brk->dgroup_name[i] != 0)
			fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
	}
	fprintf(fp_ss, "\n");
	fprintf(fp_ss, "DGREAD,Disk Group Read KB/s %s", disk_brk->ext->hostname);
	for (i = 0; i < DGROUPS; i++) {
		if (disk_brk->dgroup_name[i] != 0)
			fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
	}
	fprintf(fp_ss, "\n");
	fprintf(fp_ss, "DGWRITE,Disk Group Write KB/s %s", disk_brk->ext->hostname);
	for (i = 0; i < DGROUPS; i++) {
		if (disk_brk->dgroup_name[i] != 0)
			fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
	}
	fprintf(fp_ss, "\n");
	fprintf(fp_ss, "DGSIZE,Disk Group Block Size KB %s", disk_brk->ext->hostname);
	for (i = 0; i < DGROUPS; i++) {
		if (disk_brk->dgroup_name[i] != 0)
			fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
	}
	fprintf(fp_ss, "\n");
	fprintf(fp_ss, "DGXFER,Disk Group Transfers/s %s", disk_brk->ext->hostname);
	for (i = 0; i < DGROUPS; i++) {
		if (disk_brk->dgroup_name[i] != 0)
			fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
	}
	fprintf(fp_ss, "\n");

	/* If requested provide additional data available in /g_data->proc/diskstats */
	if( disk_brk->extended_disk == 1 && disk_brk->disk_mode == DISK_MODE_DISKSTATS )        {
		fprintf(fp_ss, "DGREADS,Disk Group read/s %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGREADMERGE,Disk Group merged read/s %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGREADSERV,Disk Group read service time (SUM ms) %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGWRITES,Disk Group write/s %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGWRITEMERGE,Disk Group merged write/s %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGWRITESERV,Disk Group write service time (SUM ms) %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGINFLIGHT,Disk Group in flight IO %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGIOTIME,Disk Group time spent for IO (ms) %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, "DGBACKLOG,Disk Group Backlog time (ms) %s", disk_brk->ext->hostname);
		for (i = 0; i < DGROUPS; i++) {
			if (disk_brk->dgroup_name[i] != 0)
				fprintf(fp_ss, ",%s", disk_brk->dgroup_name[i]);
		}
		fprintf(fp_ss, "\n");
	}
}

int is_dgroup_name(struct disk_brk * disk_brk, char *name)
{
	if (disk_brk == NULL)
		return 0;
	int i;
	for (i = 0; i < DGROUPS; i++) {
		if(disk_brk->dgroup_name[i] == (char *)0 )
			return 0;
		if (strncmp(name,disk_brk->dgroup_name[i],strlen(name)) == 0)
			return 1;
	}
	return 0;
}

inline int snap_average(struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return 0;
	int i;
	int end;
	int total = 0;

	if(cpuinfo_brk->cpu_snap_all)
		end = MAX_SNAPS;
	else
		end = cpuinfo_brk->next_cpu_snap;

	for(i=0;i<end;i++) {
		total = total + cpuinfo_brk->cpu_snap[i].user + cpuinfo_brk->cpu_snap[i].kernel;
	}
	return (total / end) ;
}

void snap_clear(struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;
	int i;
	for(i=0;i<MAX_SNAPS;i++) {
		cpuinfo_brk->cpu_snap[i].user = 0;
		cpuinfo_brk->cpu_snap[i].kernel = 0;
		cpuinfo_brk->cpu_snap[i].iowait = 0;
		cpuinfo_brk->cpu_snap[i].idle = 0;
		cpuinfo_brk->cpu_snap[i].steal= 0;
	}
	cpuinfo_brk->next_cpu_snap=0;
	cpuinfo_brk->cpu_snap_all=0;
}

void plot_snap(struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;

	int i;
	int j;
	int colour = cpuinfo_brk->ext->colour;
	WINDOW * pad = cpuinfo_brk->ext->pad;
	int * x = &cpuinfo_brk->ext->x;

	mvwprintw(pad,*x+0, 0, " CPU +---Long-Term-------------------------------------------------------------+");
	mvwprintw(pad,*x+1, 0,"100%%-|");
	mvwprintw(pad,*x+2, 1, "95%%-|");
	mvwprintw(pad,*x+3, 1, "90%%-|");
	mvwprintw(pad,*x+4, 1, "85%%-|");
	mvwprintw(pad,*x+5, 1, "80%%-|");
	mvwprintw(pad,*x+6, 1, "75%%-|");
	mvwprintw(pad,*x+7, 1, "70%%-|");
	mvwprintw(pad,*x+8, 1, "65%%-|");
	mvwprintw(pad,*x+9, 1, "60%%-|");
	mvwprintw(pad,*x+10, 1, "55%%-|");
	mvwprintw(pad,*x+11, 1, "50%%-|");
	mvwprintw(pad,*x+12, 1, "45%%-|");
	mvwprintw(pad,*x+13, 1, "40%%-|");
	mvwprintw(pad,*x+14, 1, "35%%-|");
	mvwprintw(pad,*x+15, 1, "30%%-|");
	mvwprintw(pad,*x+16, 1, "25%%-|");
	mvwprintw(pad,*x+17, 1, "20%%-|");
	mvwprintw(pad,*x+18, 1,"15%%-|");
	mvwprintw(pad,*x+19, 1,"10%%-|");
	mvwprintw(pad,*x+20, 1," 5%%-|");

	mvwprintw(pad,*x+21, 4, " +-------------------------------------------------------------------------+");
	if (colour){
		if(colour)  wattrset(pad, COLOR_PAIR(2));
		mvwprintw(pad,*x+0, 26, "User%%");
		if(colour)  wattrset(pad, COLOR_PAIR(1));
		mvwprintw(pad,*x+0, 36, "System%%");
		if(colour)  wattrset(pad, COLOR_PAIR(4));
		mvwprintw(pad,*x+0, 49, "Wait%%");
		if(colour)  wattrset(pad, COLOR_PAIR(5));
		mvwprintw(pad,*x+0, 59, "Steal%%");
		if(colour)  wattrset(pad, COLOR_PAIR(0));
	}

	for (j = 0; j < MAX_SNAPS; j++) {
		for (i = 0; i < MAX_SNAP_ROWS; i++) {
			wmove(pad,*x + MAX_SNAP_ROWS - i, j+SNAP_OFFSET);
			if (cpuinfo_brk->cpu_snap[j].user + cpuinfo_brk->cpu_snap[j].kernel + cpuinfo_brk->cpu_snap[j].iowait + cpuinfo_brk->cpu_snap[j].idle + cpuinfo_brk->cpu_snap[j].steal == 0) { /* if not all zeros */
				if(colour)  wattrset(pad,COLOR_PAIR(0));
				wprintw(pad," ");
			} else if( (cpuinfo_brk->cpu_snap[j].user / 100 * MAX_SNAP_ROWS) > i+0.5) {
				if(colour)  wattrset(pad,COLOR_PAIR(9));
				wprintw(pad,"U");
			} else if( (cpuinfo_brk->cpu_snap[j].user + cpuinfo_brk->cpu_snap[j].kernel )/ 100 * MAX_SNAP_ROWS > i+0.5) {
				if(colour)  wattrset(pad,COLOR_PAIR(8));
				wprintw(pad,"s");
			} else if( (cpuinfo_brk->cpu_snap[j].user + cpuinfo_brk->cpu_snap[j].kernel + cpuinfo_brk->cpu_snap[j].iowait )/ 100 * MAX_SNAP_ROWS > i+0.5) {
				if(colour)  wattrset(pad,COLOR_PAIR(10));
				wprintw(pad,"w");
			} else if( (cpuinfo_brk->cpu_snap[j].user + cpuinfo_brk->cpu_snap[j].kernel + cpuinfo_brk->cpu_snap[j].iowait + cpuinfo_brk->cpu_snap[j].idle)/ 100 * MAX_SNAP_ROWS > i) { /*no +0.5 or too few Steal's */
				if(colour)  wattrset(pad,COLOR_PAIR(0));
				wprintw(pad," ");
			} else if (cpuinfo_brk->cpu_snap[j].user + cpuinfo_brk->cpu_snap[j].kernel + cpuinfo_brk->cpu_snap[j].iowait + cpuinfo_brk->cpu_snap[j].idle + cpuinfo_brk->cpu_snap[j].steal > i) { /* if not all zeros */
				if(colour)  wattrset(pad,COLOR_PAIR(5));
				wprintw(pad,"S");
			}
		}
	}
	if(colour)  wattrset(pad,COLOR_PAIR(0));
	for (i = 0; i < MAX_SNAP_ROWS; i++) {
		wmove(pad,*x + MAX_SNAP_ROWS - i, cpuinfo_brk->next_cpu_snap + SNAP_OFFSET);
		wprintw(pad,"|");
	}
	wmove(pad,*x + MAX_SNAP_ROWS + 1 - (snap_average(cpuinfo_brk) / 5), cpuinfo_brk->next_cpu_snap + SNAP_OFFSET);
	wprintw(pad,"+");
	if(cpuinfo_brk->dotline) {
		for (i = 0; i < MAX_SNAPS; i++) {
			wmove(pad,*x + MAX_SNAP_ROWS+1-cpuinfo_brk->dotline*2, i+SNAP_OFFSET);
			wprintw(pad,"+");
		}
		cpuinfo_brk->dotline = 0;
	}
}

/* This saves the CPU overall usage for later ploting on the screen */
void save_snap(struct cpuinfo_brk * cpuinfo_brk, double user, double kernel, double iowait, double idle, double steal)
{
	if (cpuinfo_brk == NULL)
		return;

	cpuinfo_brk->cpu_snap[cpuinfo_brk->next_cpu_snap].user = user;
	cpuinfo_brk->cpu_snap[cpuinfo_brk->next_cpu_snap].kernel = kernel;
	cpuinfo_brk->cpu_snap[cpuinfo_brk->next_cpu_snap].iowait = iowait;
	cpuinfo_brk->cpu_snap[cpuinfo_brk->next_cpu_snap].idle = idle;
	cpuinfo_brk->cpu_snap[cpuinfo_brk->next_cpu_snap].steal = steal;
}

void plot_smp_save(struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk, int cpu_no, int row, double user, double kernel, double iowait, double idle, double steal)
{
	static int first_steal  = 1;
	struct global_data * g_data = smp_brk->ext;
	if(smp_brk->ext->show_rrd) return;

	/* Sanity check the numnbers */
	if( user < 0.0 || kernel < 0.0 || iowait < 0.0 || idle < 0.0 || idle >100.0 || steal <0 ) {
		user = kernel = iowait = idle = steal = 0;
	}

	if(first_steal && steal >0 ) {
		fprintf(g_data->fp_ss,"AAA,steal,1\n");
		first_steal=0;
	}
	if(cpu_no == 0)
		fprintf(g_data->fp_ss,"CPU_ALL,%s,%.1lf,%.1lf,%.1lf,%.1f,%.1lf,,%d\n", LOOP(g_data->loop,g_data->show_rrd),
				user, kernel, iowait, idle, steal, cpuinfo_brk->cpus);
	else {
		fprintf(g_data->fp_ss,"CPU%03d,%s,%.1lf,%.1lf,%.1lf,%.1lf,%.1f\n", cpu_no, LOOP(g_data->loop,g_data->show_rrd),
				user, kernel, iowait, idle, steal );
	}
}

void plot_smp_show(struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk, int cpu_no, int row, double user, double kernel, double iowait, double idle, double steal)
{
	if (smp_brk == NULL || lparcfg_brk == NULL)
		return;

	int peak_col;
	int i;
	WINDOW * pad = smp_brk->ext->pad;
	if(cpu_no == 0)
		mvwprintw(pad,row, 0, "Avg");
	else
		mvwprintw(pad,row, 0, "%3d", cpu_no);
	mvwprintw(pad,row,  3, "% 6.1lf", user);
	mvwprintw(pad,row,  9, "% 6.1lf", kernel);
	mvwprintw(pad,row, 15, "% 6.1lf", iowait);
	if(steal) {
		mvwprintw(pad,row, 21, "% 6.1lf", steal);
	} else {
		mvwprintw(pad,row, 21, "% 6.1lf", idle);
	}
	mvwprintw(pad,row, 27, "|");
	wmove(pad,row, 28);
	for (i = 0; i < (int)(user   / 2); i++){
		wattrset(pad,COLOR_PAIR(9));
		wprintw(pad,"U");
	}
	for (i = 0; i < (int)(kernel / 2); i++){
		wattrset(pad,COLOR_PAIR(8));
		wprintw(pad,"s");
	}
	for (i = 0; i < (int)(iowait / 2); i++) {
		wattrset(pad,COLOR_PAIR(10));
		wprintw(pad,"W");
	}
	wattrset(pad,COLOR_PAIR(0));
	for (i = 0; i <= (int)(idle   / 2); i++) {  /* added "=" to try to conteract missing halves */
#ifdef POWER
		if( lparcfg_brk->lparcfg.smt_mode > 1 && ((cpu_no -1) % lparcfg_brk->lparcfg.smt_mode) == 0 && (i % 2))
			wprintw(pad,".");
		else
#endif
			wprintw(pad," ");
	}
	for (i = 0; i < (int)((steal+1)  / 2); i++) {
		wattrset(pad,COLOR_PAIR(5));
		wprintw(pad,"S");
	}
	wattrset(pad,COLOR_PAIR(0));
	mvwprintw(pad,row, 77, "| ");

	if(cpuinfo_brk->cpu_peak[cpu_no] < (user + kernel + iowait) )
		cpuinfo_brk->cpu_peak[cpu_no] = (double)((int)user/2 + (int)kernel/2 + (int)iowait/2)*2.0;
	peak_col = 28 +(int)(cpuinfo_brk->cpu_peak[cpu_no]/2);
	if(peak_col > 77)
		peak_col=77;
	mvwprintw(pad,row, peak_col, ">");
}

void save_smp_save(struct smp_brk * smp_brk, int cpu_no, int row, long user, long kernel, long iowait, long idle, long nice, long irq, long softirq, long steal)
{
	if (smp_brk == NULL)
		return;

	struct global_data * g_data = smp_brk->ext;
	static int firsttime = 1;
	if(firsttime) {
		fprintf(g_data->fp_ss,"CPUTICKS_ALL,AAA,user,sys,wait,idle,nice,irq,softirq,steal\n");
		fprintf(g_data->fp_ss,"CPUTICKS%03d,AAA,user,sys,wait,idle,nice,irq,softirq,steal\n", cpu_no);
		firsttime=0;
	}
	if(cpu_no==0) {
		fprintf(g_data->fp_ss,"CPUTICKS_ALL,%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",
				LOOP(g_data->loop,g_data->show_rrd), user, kernel, iowait, idle, nice, irq, softirq, steal);
	} else {
		fprintf(g_data->fp_ss,"CPUTICKS%03d,%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",
				cpu_no, LOOP(g_data->loop,g_data->show_rrd), user, kernel, iowait, idle, nice, irq, softirq, steal);
	}
}

void save_smp_show(struct smp_brk * smp_brk, int cpu_no, int row, long user, long kernel, long iowait, long idle, long nice, long irq, long softirq, long steal)
{
	struct global_data * g_data = smp_brk->ext;

	mvwprintw(g_data->pad, row,0, "%3d usr=%4ld sys=%4ld wait=%4ld idle=%4ld steal=%2ld nice=%4ld irq=%2ld sirq=%2ld\n",
			cpu_no, user, kernel, iowait, idle, steal, nice, irq, softirq);
	return;
}
