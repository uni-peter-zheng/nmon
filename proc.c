#include "lmon.h"
#include "proc.h"
#include "struct.h"
#include "stat.h"
#include "func.h"

extern int reread;
extern int partitions_short;
extern int partitions;

extern int proc_cpu_done;

void proc_init(struct proc_file * proc)
{
	proc[P_CPUINFO].filename = "/proc/cpuinfo";
	proc[P_STAT].filename    = "/proc/stat";
	proc[P_VERSION].filename = "/proc/version";
	proc[P_MEMINFO].filename = "/proc/meminfo";
	proc[P_UPTIME].filename  = "/proc/uptime";
	proc[P_LOADAVG].filename = "/proc/loadavg";
	proc[P_NFS].filename     = "/proc/net/rpc/nfs";
	proc[P_NFSD].filename    = "/proc/net/rpc/nfsd";
	proc[P_VMSTAT].filename  = "/proc/vmstat";
}

void strip_spaces(char *s)
{
	char *p;
	int spaced=1;

	p=s;
	for(p=s;*p!=0;p++) {
		if(*p == ':')
			*p=' ';
		if(*p != ' ') {
			*s=*p;
			s++;
			spaced=0;
		} else if(spaced) {
			/* do no thing as this is second space */
		} else {
			*s=*p;
			s++;
			spaced=1;
		}

	}
	*s = 0;
}

void proc_read (struct proc_file * proc, int num)
{
	int i;
	int size;
	int found;
	char buf[1024];

	if(proc[num].read_this_interval == 1 )
		return;

	if(proc[num].fp == 0) {
		if( (proc[num].fp = fopen(proc[num].filename,"r")) == NULL) {
			sprintf(buf, "failed to open file %s", proc[num].filename);
			error(buf);
			proc[num].fp = 0;
			return;
		}
	}
	rewind(proc[num].fp);

	/* We re-read P_STAT, now flag proc_cpu() that it has to re-process that data */
	if( num == P_STAT)
		proc_cpu_done = 0;

	if(proc[num].size == 0) {
		/* first time so allocate  initial now */
		proc[num].buf = MALLOC(512);
		proc[num].size = 512;
	}

	for(i=0;i<4096;i++) {   /* MAGIC COOKIE WARNING  POWER8 default install can have 2655 processes */
		size = fread(proc[num].buf, 1, proc[num].size-1, proc[num].fp);
		if(size < proc[num].size -1)
			break;
		proc[num].size +=512;
		proc[num].buf = REALLOC(proc[num].buf,proc[num].size);
		rewind(proc[num].fp);
	}

	proc[num].buf[size]=0;
	proc[num].lines=0;
	proc[num].line[0]=&proc[num].buf[0];
	if(num == P_VERSION) {
		found=0;
		for(i=0;i<size;i++) { /* remove some weird stuff found the hard way in various Linux versions and device drivers */
			/* place ") (" on two lines */
			if( found== 0 &&
					proc[num].buf[i]   == ')' &&
					proc[num].buf[i+1] == ' ' &&
					proc[num].buf[i+2] == '(' ) {
				proc[num].buf[i+1] = '\n';
				found=1;
			} else {
				/* place ") #" on two lines */
				if( proc[num].buf[i]   == ')' &&
						proc[num].buf[i+1] == ' ' &&
						proc[num].buf[i+2] == '#' ) {
					proc[num].buf[i+1] = '\n';
				}
				/* place "#1" on two lines */
				if(
						proc[num].buf[i]   == '#' &&
						proc[num].buf[i+2] == '1' ) {
					proc[num].buf[i] = '\n';
				}
			}
		}
	}
	for(i=0;i<size;i++) {
		/* replace Tab characters with space */
		if(proc[num].buf[i] == '\t')    {
			proc[num].buf[i]= ' ';
		}
		else if(proc[num].buf[i] == '\n') {
			/* replace newline characters with null */
			proc[num].lines++;
			proc[num].buf[i] = '\0';
			proc[num].line[proc[num].lines] = &proc[num].buf[i+1];
		}
		if(proc[num].lines==PROC_MAXLINES-1)
			break;
	}
	if(reread) {
		fclose(proc[num].fp);
		proc[num].fp = 0;
	}
	/* Set flag so we do not re-read the data even if called multiple times in same interval */
	proc[num].read_this_interval = 1;
}


#ifdef POWER
/* XXXXXXX need to test if rewind() worked or not for lparcfg */
/* Reset at end of each interval so LPAR cfg is only read once each interval
 * even if proc_lparcfg() is called multiple times
 * Note: lparcfg is not read via proc_read() !
 */

char * locate(struct lparcfg_brk * lparcfg_brk, char *s)
{
	int i;
	int len;
	len=strlen(s);
	for(i=0;i < lparcfg_brk->lpar_count; i++)
		if( !strncmp(s,lparcfg_brk->lpar_buffer[i],len))
			return lparcfg_brk->lpar_buffer[i];
	return "";
}

long long read_longlong(struct lparcfg_brk * lparcfg_brk, char *s)
{
	long long x;
	int ret;
	int len;
	int i;
	char *str;
	str = locate(lparcfg_brk, s);
	len=strlen(str);
	if(len == 0) {
		return NUMBER_NOT_VALID;
	}
	for(i=0;i<len;i++) {
		if(str[i] == '=') {
			ret = sscanf(&str[i+1], "%lld", &x);
			if(ret != 1) {
				fprintf(stderr,"sscanf for %s failed returned = %d line=%s\n", s, ret, str);
				return -1;
			}
			/* fprintf(fp,"DEBUG read %s value %lld\n",s,x);*/
			return x;
		}
	}
	fprintf(stderr,"read_long_long failed returned line=%s\n", str);
	return -2;
}

/* Return of 0 means data not available */
void proc_lparcfg(struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk)
{
	static FILE *fp = (FILE *)-1;
	/* Only try to read /proc/ppc64/lparcfg once - remember if it's readable */
	static int lparinfo_not_available=0;
	int i;
	char *str;
	/* If we already read and processed /proc/lparcfg in this interval - just return */
	if( lparcfg_brk->lparcfg_processed == 1) {
		lparcfg_brk->result = 1;    
		return;
	}

	if( lparinfo_not_available == 1) {
		lparcfg_brk->result = 0;    
		return;
	}

	if( fp == (FILE *)-1) {
		if( (fp = fopen("/proc/ppc64/lparcfg","r")) == NULL) {
			error("failed to open - /proc/ppc64/lparcfg");
			fp = (FILE *)-1;
			lparinfo_not_available = 1;
			if(cpuinfo_brk->power_vm_type == VM_UNKNOWN) {
				/*  Heuristics for spotting PowerKVM Host
					a) inside ifdef POWER so can't be x86
					b) /proc/ppc64/lparcfg is missing
					c) /etc/ *ease files have hints
					Confirmed true for IBM_POWERKVM 2.1
					*/
				if(strncmp( cpuinfo_brk->easy[1], "IBM_PowerKVM", 12) == 0)
					cpuinfo_brk->power_vm_type = VM_POWERKVM_HOST;
				else
					cpuinfo_brk->power_vm_type = VM_NATIVE;
			}
			lparcfg_brk->result = 0;
			return;
		}
	}

	for(lparcfg_brk->lpar_count=0; lparcfg_brk->lpar_count < LPAR_LINE_MAX - 1; lparcfg_brk->lpar_count++) {
		if(fgets(lparcfg_brk->lpar_buffer[lparcfg_brk->lpar_count],LPAR_LINE_WIDTH-1,fp) == NULL)
			break;
	}
	if(lparcfg_brk->lparcfg_reread) { /* XXXX  unclear if close+open is necessary   - unfortunately this requires version many of Linux on POWER install to test early releases */
		fclose(fp);
		fp = (FILE *)-1;
	} else rewind(fp);

	str=locate(lparcfg_brk, "lparcfg");          sscanf(str, "lparcfg %s", lparcfg_brk->lparcfg.version_string);
	str=locate(lparcfg_brk, "serial_number");    sscanf(str, "serial_number=%s", lparcfg_brk->lparcfg.serial_number);
	str=locate(lparcfg_brk, "system_type");
	for(i=0;i<strlen(str);i++) /* Remove new spaces in massive string meaning PowerKVM Guest !!*/
		if(str[i] == ' ')
			str[i] = '-';
	sscanf(str, "system_type=%s", lparcfg_brk->lparcfg.system_type);
	if(cpuinfo_brk->power_vm_type == VM_UNKNOWN) {
		/*  Heuristics for spotting PowerKVM Guest
			a) inside ifdef POWER so can't be x86
			b) we have a /proc/ppc64/lparcfg - probably mostly missing (1.9)
			c) system type string includes "qemu"
			Confirmed true for SLES11.3 RHEL6.5 and Ubuntu 14.4.1
			*/
		if(strstr( lparcfg_brk->lparcfg.system_type, "(emulated-by-qemu)") == 0)
			cpuinfo_brk->power_vm_type = VM_POWERVM; /* not found */
		else
			cpuinfo_brk->power_vm_type = VM_POWERKVM_GUEST;
	}

#define GETDATA(variable) lparcfg_brk->lparcfg.variable = read_longlong(lparcfg_brk, __STRING(variable) );

	GETDATA(partition_id);
	GETDATA(BoundThrds);
	GETDATA(CapInc);
	GETDATA(DisWheRotPer);
	GETDATA(MinEntCap);
	GETDATA(MinEntCapPerVP);
	GETDATA(MinMem);
	GETDATA(DesMem);
	GETDATA(MinProcs);
	GETDATA(partition_max_entitled_capacity);
	GETDATA(system_potential_processors);
	GETDATA(partition_entitled_capacity);
	GETDATA(system_active_processors);
	GETDATA(pool_capacity);
	GETDATA(unallocated_capacity_weight);
	GETDATA(capacity_weight);
	GETDATA(capped);
	GETDATA(unallocated_capacity);
	lparcfg_brk->lparcfg.pool_idle_saved = lparcfg_brk->lparcfg.pool_idle_time;
	GETDATA(pool_idle_time);
	lparcfg_brk->lparcfg.pool_idle_diff = lparcfg_brk->lparcfg.pool_idle_time - lparcfg_brk->lparcfg.pool_idle_saved;
	GETDATA(pool_num_procs);
	lparcfg_brk->lparcfg.purr_saved = lparcfg_brk->lparcfg.purr;
	GETDATA(purr);
	lparcfg_brk->lparcfg.purr_diff = lparcfg_brk->lparcfg.purr - lparcfg_brk->lparcfg.purr_saved;
	GETDATA(partition_active_processors);
	GETDATA(partition_potential_processors);
	GETDATA(shared_processor_mode);
	/* Brute force, may provide temporary incorrect data during
	 * dynamic reconfiguraiton envents
	 */
	lparcfg_brk->lparcfg.smt_mode = cpuinfo_brk->cpus / lparcfg_brk->lparcfg.partition_active_processors;

	/* AMS additions */
	GETDATA(cmo_enabled);
	if( lparcfg_brk->lparcfg.cmo_enabled == NUMBER_NOT_VALID )      {
		lparcfg_brk->lparcfg.cmo_enabled = 0;
	}
	if( lparcfg_brk->lparcfg.cmo_enabled ) {
		GETDATA(entitled_memory_pool_number);   /*  pool number = 0 */
		GETDATA(entitled_memory_weight);        /* 0 to 255 */

		lparcfg_brk->lparcfg.cmo_faults_save = lparcfg_brk->lparcfg.cmo_faults;
		GETDATA(cmo_faults);                    /* Hypervisor Page-in faults = big number */
		lparcfg_brk->lparcfg.cmo_faults_diff = lparcfg_brk->lparcfg.cmo_faults - lparcfg_brk->lparcfg.cmo_faults_save;

		lparcfg_brk->lparcfg.cmo_fault_time_usec_save = lparcfg_brk->lparcfg.cmo_fault_time_usec;
		GETDATA(cmo_fault_time_usec);           /* Hypervisor time in micro seconds = big number */
		lparcfg_brk->lparcfg.cmo_fault_time_usec_diff = lparcfg_brk->lparcfg.cmo_fault_time_usec - lparcfg_brk->lparcfg.cmo_fault_time_usec_save;

		GETDATA(backing_memory);                /* AIX pmem in bytes */
		GETDATA(cmo_page_size);                 /* AMS page size in bytes */
		GETDATA(entitled_memory_pool_size);     /* AMS whole pool size in bytes */
		GETDATA(entitled_memory_loan_request);  /* AMS requesting more memory loaning */

	}
#ifdef EXPERIMENTAL
	GETDATA(DesEntCap);
	GETDATA(DesProcs);
	GETDATA(DesVarCapWt);
	GETDATA(DedDonMode);
	GETDATA(group);
	GETDATA(pool);
	GETDATA(entitled_memory);
	GETDATA(entitled_memory_group_number);
	GETDATA(unallocated_entitled_memory_weight);
	GETDATA(unallocated_io_mapping_entitlement);
#endif /* EXPERIMENTAL */

	lparcfg_brk->lparcfg_processed=1;
	lparcfg_brk->result = 1;
}
#endif /*POWER*/

void proc_net(struct net_brk * net_brk)
{
	if (net_brk == NULL)
		return;

	struct data * p = net_brk->ext->p;
	static FILE *fp = (FILE *)-1;
	char buf[1024];
	int i=0;
	int ret;
	unsigned long junk;

	if( fp == (FILE *)-1) {
		if( (fp = fopen("/proc/net/dev","r")) == NULL) {
			error("failed to open - /proc/net/dev");
			net_brk->networks=0;
			return;
		}
	}
	if(fgets(buf,1024,fp) == NULL) goto end; /* throw away the header lines */
	if(fgets(buf,1024,fp) == NULL) goto end; /* throw away the header lines */
	/*
	   Inter-|   Receive                                                |  Transmit
	   face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
lo:    1956      30    0    0    0     0          0         0     1956      30    0    0    0     0       0          0
eth0:       0       0    0    0    0     0          0         0   458718       0  781    0    0     0     781          0
sit0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
*/
	for(i=0;i<NETMAX;i++) {
		if(fgets(buf,1024,fp) == NULL)
			break;
		strip_spaces((char *)buf);
		/* 1   2   3    4   5   6   7   8   9   10   11   12  13  14  15  16 */
		ret = sscanf(&buf[0], "%s %llu %llu %lu %lu %lu %lu %lu %lu %llu %llu %lu %lu %lu %lu %lu",
				(char *)&p->ifnets[i].if_name,
				&p->ifnets[i].if_ibytes,
				&p->ifnets[i].if_ipackets,
				&p->ifnets[i].if_ierrs,
				&p->ifnets[i].if_idrop,
				&p->ifnets[i].if_ififo,
				&p->ifnets[i].if_iframe,
				&junk,
				&junk,
				&p->ifnets[i].if_obytes,
				&p->ifnets[i].if_opackets,
				&p->ifnets[i].if_oerrs,
				&p->ifnets[i].if_odrop,
				&p->ifnets[i].if_ofifo,
				&p->ifnets[i].if_ocolls,
				&p->ifnets[i].if_ocarrier
				);
		p->ifnets[i].if_ibits = p->ifnets[i].if_ibytes * 8;
		p->ifnets[i].if_obits = p->ifnets[i].if_obytes * 8;

		if(ret != 16)
			fprintf(stderr,"sscanf wanted 16 returned = %d line=%s\n", ret, (char *)buf);
	}
end:
	if(reread) {
		fclose(fp);
		fp = (FILE *)-1;
	} else rewind(fp);
	net_brk->networks = i;
}


int proc_procsinfo(struct global_data * g_data, int pid, int index, struct data * p)
{
	FILE *fp;
	char filename[64];
	char buf[1024*4];
	int size=0;
	int ret=0;
	int count=0;
	int i;

	sprintf(filename,"/proc/%d/stat",pid);
	if( (fp = fopen(filename,"r")) == NULL) {
		sprintf(buf,"failed to open file %s",filename);
		error(buf);
		return 0;
	}
	size = fread(buf, 1, 1024-1, fp);
	fclose(fp);
	if(size == -1) {
#ifdef DEBUG
		fprintf(stderr,"procsinfo read returned = %d assuming process stopped pid=%d\n", ret,pid);
#endif /*DEBUG*/
		return 0;
	}
	ret = sscanf(buf, "%d (%s)",
			&p->procs[index].pi_pid,
			&p->procs[index].pi_comm[0]);
	if(ret != 2) {
		fprintf(stderr,"procsinfo sscanf returned = %d line=%s\n", ret,buf);
		return 0;
	}
	p->procs[index].pi_comm[strlen(p->procs[index].pi_comm)-1] = 0;

	for(count=0; count<size;count++)        /* now look for ") " as dumb Infiniban driver includes "()" */
		if(buf[count] == ')' && buf[count+1] == ' ' ) break;

	if(count == size) {
#ifdef DEBUG
		fprintf(stderr,"procsinfo failed to find end of command buf=%s\n", buf);
#endif /*DEBUG*/
		return 0;
	}
	count++; count++;

	ret = sscanf(&buf[count],
#ifndef KERNEL_2_6_18
			"%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d",
#else
			"%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %lu %lu %llu",
#endif
			&p->procs[index].pi_state,
			&p->procs[index].pi_ppid,
			&p->procs[index].pi_pgrp,
			&p->procs[index].pi_session,
			&p->procs[index].pi_tty_nr,
			&p->procs[index].pi_tty_pgrp,
			&p->procs[index].pi_flags,
			&p->procs[index].pi_minflt,
			&p->procs[index].pi_cmin_flt,
			&p->procs[index].pi_majflt,
			&p->procs[index].pi_cmaj_flt,
			&p->procs[index].pi_utime,
			&p->procs[index].pi_stime,
			&p->procs[index].pi_cutime,
			&p->procs[index].pi_cstime,
			&p->procs[index].pi_pri,
			&p->procs[index].pi_nice,
#ifndef KERNEL_2_6_18
			&p->procs[index].junk,
#else
			&p->procs[index].pi_num_threads,
#endif
			&p->procs[index].pi_it_real_value,
			&p->procs[index].pi_start_time,
			&p->procs[index].pi_vsize,
			&p->procs[index].pi_rss,
			&p->procs[index].pi_rlim_cur,
			&p->procs[index].pi_start_code,
			&p->procs[index].pi_end_code,
			&p->procs[index].pi_start_stack,
			&p->procs[index].pi_esp,
			&p->procs[index].pi_eip,
			&p->procs[index].pi_pending_signal,
			&p->procs[index].pi_blocked_sig,
			&p->procs[index].pi_sigign,
			&p->procs[index].pi_sigcatch,
			&p->procs[index].pi_wchan,
			&p->procs[index].pi_nswap,
			&p->procs[index].pi_cnswap,
			&p->procs[index].pi_exit_signal,
			&p->procs[index].pi_cpu
#ifdef KERNEL_2_6_18
				,
			&p->procs[index].pi_rt_priority,
			&p->procs[index].pi_policy,
			&p->procs[index].pi_delayacct_blkio_ticks
#endif

				);
#ifndef KERNEL_2_6_18
	if(ret != 37) {
		fprintf(stderr,"procsinfo2 sscanf wanted 37 returned = %d pid=%d line=%s\n", ret,pid,buf);
#else
		if(ret != 40) {
			fprintf(stderr,"procsinfo2 sscanf wanted 40 returned = %d pid=%d line=%s\n", ret,pid,buf);
#endif
			return 0;
		}

		sprintf(filename,"/proc/%d/statm",pid);
		if( (fp = fopen(filename,"r")) == NULL) {
			sprintf(buf,"failed to open file %s",filename);
			error(buf);
			return 0;
		}
		size = fread(buf, 1, 1024*4-1, fp);
		fclose(fp); /* close it even if the read failed, the file could have been removed
					   between open & read i.e. the device driver does not behave like a file */
		if(size == -1) {
			sprintf(buf,"failed to read file %s",filename);
			error(buf);
			return 0;
		}

		ret = sscanf(&buf[0], "%lu %lu %lu %lu %lu %lu %lu",
				&p->procs[index].statm_size,
				&p->procs[index].statm_resident,
				&p->procs[index].statm_share,
				&p->procs[index].statm_trs,
				&p->procs[index].statm_lrs,
				&p->procs[index].statm_drs,
				&p->procs[index].statm_dt
				);
		if(ret != 7) {
			fprintf(stderr,"sscanf wanted 7 returned = %d line=%s\n", ret,buf);
			return 0;
		}
		if(g_data->isroot) {
			p->procs[index].read_io = 0;
			p->procs[index].write_io = 0;
			sprintf(filename,"/proc/%d/io",pid);
			if( (fp = fopen(filename,"r")) != NULL) {
				for(i=0;i<6;i++) {
					if(fgets(buf,1024,fp) == NULL) {
						break;
					}
					if(strncmp("read_bytes:",  buf, 11) == 0 )
						sscanf(&buf[12], "%lld", &p->procs[index].read_io);
					if(strncmp("write_bytes:", buf, 12) == 0 )
						sscanf(&buf[13], "%lld", &p->procs[index].write_io);
				}
			}

			if (fp != NULL)
				fclose(fp);
		}
		return 1;
	}


#ifdef DEBUGPROC
	void print_procs(int index)
	{
		printf("procs[%d].pid           =%d\n",index,procs[index].pi_pid);
		printf("procs[%d].comm[0]       =%s\n",index,&procs[index].pi_comm[0]);
		printf("procs[%d].state         =%c\n",index,procs[index].pi_state);
		printf("procs[%d].ppid          =%d\n",index,procs[index].pi_ppid);
		printf("procs[%d].pgrp          =%d\n",index,procs[index].pi_pgrp);
		printf("procs[%d].session       =%d\n",index,procs[index].pi_session);
		printf("procs[%d].tty_nr        =%d\n",index,procs[index].pi_tty_nr);
		printf("procs[%d].tty_pgrp      =%d\n",index,procs[index].pi_tty_pgrp);
		printf("procs[%d].flags         =%lu\n",index,procs[index].pi_flags);
		printf("procs[%d].minflt       =%lu\n",index,procs[index].pi_minflt);
		printf("procs[%d].cmin_flt     =%lu\n",index,procs[index].pi_cmin_flt);
		printf("procs[%d].majflt       =%lu\n",index,procs[index].pi_majflt);
		printf("procs[%d].cmaj_flt     =%lu\n",index,procs[index].pi_cmaj_flt);
		printf("procs[%d].utime        =%lu\n",index,procs[index].pi_utime);
		printf("procs[%d].stime        =%lu\n",index,procs[index].pi_stime);
		printf("procs[%d].cutime       =%ld\n",index,procs[index].pi_cutime);
		printf("procs[%d].cstime       =%ld\n",index,procs[index].pi_cstime);
		printf("procs[%d].pri           =%d\n",index,procs[index].pi_pri);
		printf("procs[%d].nice          =%d\n",index,procs[index].pi_nice);
#ifndef KERNEL_2_6_18
		printf("procs[%d].junk          =%d\n",index,procs[index].junk);
#else
		printf("procs[%d].num_threads   =%ld\n",index,procs[index].num_threads);
#endif
		printf("procs[%d].it_real_value =%lu\n",index,procs[index].pi_it_real_value);
		printf("procs[%d].start_time    =%lu\n",index,procs[index].pi_start_time);
		printf("procs[%d].vsize         =%lu\n",index,procs[index].pi_vsize);
		printf("procs[%d].rss           =%lu\n",index,procs[index].pi_rss);
		printf("procs[%d].rlim_cur      =%lu\n",index,procs[index].pi_rlim_cur);
		printf("procs[%d].start_code    =%lu\n",index,procs[index].pi_start_code);
		printf("procs[%d].end_code      =%lu\n",index,procs[index].pi_end_code);
		printf("procs[%d].start_stack   =%lu\n",index,procs[index].pi_start_stack);
		printf("procs[%d].esp           =%lu\n",index,procs[index].pi_esp);
		printf("procs[%d].eip           =%lu\n",index,procs[index].pi_eip);
		printf("procs[%d].pending_signal=%lu\n",index,procs[index].pi_pending_signal);
		printf("procs[%d].blocked_sig   =%lu\n",index,procs[index].pi_blocked_sig);
		printf("procs[%d].sigign        =%lu\n",index,procs[index].pi_sigign);
		printf("procs[%d].sigcatch      =%lu\n",index,procs[index].pi_sigcatch);
		printf("procs[%d].wchan         =%lu\n",index,procs[index].pi_wchan);
		printf("procs[%d].nswap         =%lu\n",index,procs[index].pi_nswap);
		printf("procs[%d].cnswap        =%lu\n",index,procs[index].pi_cnswap);
		printf("procs[%d].exit_signal   =%d\n",index,procs[index].pi_exit_signal);
		printf("procs[%d].cpu           =%d\n",index,procs[index].pi_cpu);
#ifdef KERNEL_2_6_18
		printf("procs[%d].rt_priority   =%lu\n",index,procs[index].pi_rt_priority);
		printf("procs[%d].policy        =%lu\n",index,procs[index].pi_policy);
		printf("procs[%d].delayacct_blkio_ticks=%llu\n",index,procs[index].pi_delayacct_blkio_ticks);
#endif
		printf("OK\n");
	}
#endif /*DEBUG*/
	/* --- */


	int getprocs(struct global_data * g_data, int details, struct data * p)
	{
		struct dirent *dent;
		DIR *procdir;
		int count =0;

		if((char *)(procdir = opendir("/proc")) == NULL) {
			printf("opendir(/proc) failed");
			return 0;
		}
		while( (char *)(dent = readdir(procdir)) != NULL ) {
			if(dent->d_type == 4) { /* is this a directlory */
				/* mainframes report 0 = unknown every time !!!!  */
				/*
				   printf("inode=%d type=%d name=%s\n",
				   dent->d_ino,
				   dent->d_type,
				   dent->d_name);
				   */
				if(isnumbers(dent->d_name)) {
					/*                      printf("%s pid\n",dent->d_name); */
					if(details) {
						count=count + proc_procsinfo(g_data, atoi(dent->d_name),count, p);
					} else {
						count++;
					}
				}
				/*
				   else
				   printf("NOT numbers\n");
				   */
			}
		}
		closedir(procdir);
		return count;
	}
	/* --- */


	void proc_cpu(struct cpuinfo_brk * cpuinfo_brk)
	{
		if (cpuinfo_brk == NULL)
			return;

		struct data * q = cpuinfo_brk->ext->q;
		struct data * p = cpuinfo_brk->ext->p;
		int i;
		int row;
		int intr_line = 0;
		int ctxt_line = 0;
		int btime_line= 0;
		int proc_line = 0;
		int run_line  = 0;
		int block_line= 0;
		static int proc_cpu_first_time = 1;
		long long user;
		long long nice;
		long long sys;
		long long idle;
		long long iowait;
		long long hardirq;
		long long softirq;
		long long steal;

		get_cpu_cnt(cpuinfo_brk);

		/* Only read data once per interval */
		if( proc_cpu_done == 1)
			return;

		/* If number of CPUs changed, then we need to find the index of intr_line, ... again */
		if( cpuinfo_brk->old_cpus != cpuinfo_brk->cpus)
			intr_line = 0;

		if(proc_cpu_first_time) {
			cpuinfo_brk->ext->stat8 = sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[0][5], "%lld %lld %lld %lld %lld %lld %lld %lld",
					&user,
					&nice,
					&sys,
					&idle,
					&iowait,
					&hardirq,
					&softirq,
					&steal);
			proc_cpu_first_time = 0;
		}

		user = nice = sys = idle = iowait = hardirq = softirq = steal = 0;
		if(cpuinfo_brk->ext->stat8 == 8) {
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[0][5], "%lld %lld %lld %lld %lld %lld %lld %lld",
					&user,
					&nice,
					&sys,
					&idle,
					&iowait,
					&hardirq,
					&softirq,
					&steal);
		} else { /* stat 4 variables here as older Linux proc */
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[0][5], "%lld %lld %lld %lld",
					&user,
					&nice,
					&sys,
					&idle);
		}
		p->cpu_total.user = user + nice;
		p->cpu_total.wait = iowait; /* in the case of 4 variables = 0 */
		p->cpu_total.sys  = sys;
		/* p->cpu_total.sys  = sys + hardirq + softirq + steal;*/
		p->cpu_total.idle = idle;

		p->cpu_total.irq     = hardirq;
		p->cpu_total.softirq = softirq;
		p->cpu_total.steal   = steal;
		p->cpu_total.nice    = nice;

#ifdef DEBUG
		if (debug)
			fprintf(stderr,"XX user=%lld wait=%lld sys=%lld idle=%lld\n",
					p->cpu_total.user,
					p->cpu_total.wait,
					p->cpu_total.sys,
					p->cpu_total.idle);
#endif /*DEBUG*/

		for(i = 0; i < cpuinfo_brk->cpus; i++ ) {
			user = nice = sys = idle = iowait = hardirq = softirq = steal = 0;

			/* allow for large CPU numbers */
			if(i+1 > 1000)
				row = 8;
			else if(i+1 > 100)
				row = 7;
			else if(i+1 > 10)
				row = 6;
			else 
				row = 5;

			if(cpuinfo_brk->ext->stat8 == 8) {
				sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[i+1][row],
						"%lld %lld %lld %lld %lld %lld %lld %lld",
						&user,
						&nice,
						&sys,
						&idle,
						&iowait,
						&hardirq,
						&softirq,
						&steal);
			} else {
				sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[i+1][row], "%lld %lld %lld %lld",
						&user,
						&nice,
						&sys,
						&idle);
			}
			p->cpuN[i].user = user + nice;
			p->cpuN[i].wait = iowait;
			p->cpuN[i].sys  = sys;
			/*p->cpuN[i].sys  = sys + hardirq + softirq + steal;*/
			p->cpuN[i].idle = idle;

			p->cpuN[i].irq     = hardirq;
			p->cpuN[i].softirq = softirq;
			p->cpuN[i].steal   = steal;
			p->cpuN[i].nice    = nice;
		}
		
		if(intr_line == 0) {
			if(cpuinfo_brk->ext->proc[P_STAT].line[i+1][0] == 'p' &&
					cpuinfo_brk->ext->proc[P_STAT].line[i+1][1] == 'a' &&
					cpuinfo_brk->ext->proc[P_STAT].line[i+1][2] == 'g' &&
					cpuinfo_brk->ext->proc[P_STAT].line[i+1][3] == 'e' ) {
				/* 2.4 kernel */
				intr_line = i + 3;
				ctxt_line = i + 5;
				btime_line= i + 6;
				proc_line = i + 7;
				run_line  = i + 8;
				block_line= i + 9;
			}else {
				/* 2.6 kernel */
				intr_line = i + 1;
				ctxt_line = i + 2;
				btime_line= i + 3;
				proc_line = i + 4;
				run_line  = i + 5;
				block_line= i + 6;
			}
		}
		p->cpu_total.intr = -1;
		p->cpu_total.ctxt = -1;
		p->cpu_total.btime = -1;
		p->cpu_total.procs = -1;
		p->cpu_total.running = -1;
		p->cpu_total.blocked = -1;
		if(cpuinfo_brk->ext->proc[P_STAT].lines >= intr_line)
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[intr_line][0], "intr %lld", &p->cpu_total.intr);
		if(cpuinfo_brk->ext->proc[P_STAT].lines >= ctxt_line)
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[ctxt_line][0], "ctxt %lld", &p->cpu_total.ctxt);
		if(cpuinfo_brk->ext->proc[P_STAT].lines >= btime_line)
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[btime_line][0], "btime %lld", &p->cpu_total.btime);
		if(cpuinfo_brk->ext->proc[P_STAT].lines >= proc_line)
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[proc_line][0], "processes %lld", &p->cpu_total.procs);
		if(cpuinfo_brk->ext->proc[P_STAT].lines >= run_line)
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[run_line][0], "procs_running %lld", &p->cpu_total.running);
		if(cpuinfo_brk->ext->proc[P_STAT].lines >= block_line)
			sscanf(&cpuinfo_brk->ext->proc[P_STAT].line[block_line][0], "procs_blocked %lld", &p->cpu_total.blocked);

		/* If we had a change in the number of CPUs, copy current interval data to the previous, so we
		 * get a "0" utilization interval, but better than negative or 100%.
		 * Heads-up - This effects POWER SMT changes too.
		 */
		if( cpuinfo_brk->old_cpus != cpuinfo_brk->cpus )        {
			memcpy((void *) &(q->cpu_total), (void *) &(p->cpu_total), sizeof(struct cpu_stat));
			memcpy((void *) q->cpuN, (void *) p->cpuN, sizeof(struct cpu_stat) * cpuinfo_brk->cpus );
		}

		/* Flag that we processed /proc/stat data; re-set in proc_read() when we re-read /proc/stat */
		proc_cpu_done = 1;
	}


	void proc_nfs(struct nfs_brk * nfs_brk)
	{
		if (nfs_brk == NULL)
			return;
		struct data * p = nfs_brk->ext->p;
		struct proc_file * proc = nfs_brk->ext->proc;
		int i;
		int j;
		int len;
		int lineno;

		/* sample /proc/net/rpc/nfs
		   net 0 0 0 0
		   rpc 70137 0 0
		   proc2 18 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
		   proc3 22 0 27364 0 32 828 22 40668 0 1 0 0 0 0 0 0 0 0 1212 6 2 1 0
		   proc4 35 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
		   */
		if(proc[P_NFS].fp != 0) {
			for(lineno=0;lineno<proc[P_NFS].lines;lineno++) {
				if(!strncmp("proc2 ",proc[P_NFS].line[lineno],6)) {
					/* client version 2 line readers "proc2 18 num num etc" */
					len=strlen(proc[P_NFS].line[lineno]);
					for(j=0,i=8;i<len && j<NFS_V2_NAMES_COUNT;i++) {
						if(proc[P_NFS].line[lineno][i] == ' ') {
							p->nfs.v2c[j] =atol(&proc[P_NFS].line[lineno][i+1]);
							nfs_brk->nfs_v2c_found=1;
							j++;
						}
					}
				}
				if(!strncmp("proc3 ",proc[P_NFS].line[lineno],6)) {
					/* client version 3 line readers "proc3 22 num num etc" */
					len=strlen(proc[P_NFS].line[lineno]);
					for(j=0,i=8;i<len && j<NFS_V3_NAMES_COUNT;i++) {
						if(proc[P_NFS].line[lineno][i] == ' ') {
							p->nfs.v3c[j] =atol(&proc[P_NFS].line[lineno][i+1]);
							nfs_brk->nfs_v3c_found=1;
							j++;
						}
					}
				}
				if(!strncmp("proc4 ",proc[P_NFS].line[lineno],6)) {
					/* client version 4 line readers "proc4 35 num num etc" */
					nfs_brk->nfs_v4c_names_count = atoi(&proc[P_NFS].line[lineno][6]);
					len=strlen(proc[P_NFS].line[lineno]);
					for(j=0,i=8; i<len && j<nfs_brk->nfs_v4c_names_count; i++) {
						if(proc[P_NFS].line[lineno][i] == ' ') {
							p->nfs.v4c[j] =atol(&proc[P_NFS].line[lineno][i+1]);
							nfs_brk->nfs_v4c_found=1;
							j++;
						}
					}
				}
			}
		}
		/* sample /proc/net/rpc/nfsd
		   rc 0 0 0
		   fh 0 0 0 0 0
		   io 0 0
		   th 4 0 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000
		   ra 32 0 0 0 0 0 0 0 0 0 0 0
		   net 0 0 0 0
		   rpc 0 0 0 0 0
		   proc2 18 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
		   proc3 22 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
		   proc4 2 0 0
		   proc4ops 40 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
		   */
		if(proc[P_NFSD].fp != 0) {
			for(lineno=0;lineno<proc[P_NFSD].lines;lineno++) {
				if(!strncmp("proc2 ",proc[P_NFSD].line[lineno],6)) {
					/* server version 2 line readers "proc2 18 num num etc" */
					len=strlen(proc[P_NFSD].line[lineno]);
					for(j=0,i=8;i<len && j<NFS_V2_NAMES_COUNT;i++) {
						if(proc[P_NFSD].line[lineno][i] == ' ') {
							p->nfs.v2s[j] =atol(&proc[P_NFSD].line[lineno][i+1]);
							nfs_brk->nfs_v2s_found=1;
							j++;
						}
					}
				}
				if(!strncmp("proc3 ",proc[P_NFSD].line[lineno],6)) {
					/* server version 3 line readers "proc3 22 num num etc" */
					len=strlen(proc[P_NFSD].line[lineno]);
					for(j=0,i=8;i<len && j<NFS_V2_NAMES_COUNT;i++) {
						if(proc[P_NFSD].line[lineno][i] == ' ') {
							p->nfs.v3s[j] =atol(&proc[P_NFSD].line[lineno][i+1]);
							nfs_brk->nfs_v3s_found=1;
							j++;
						}
					}
				}
				if(!strncmp("proc4ops ",proc[P_NFSD].line[lineno],9)) {
					/* server version 4 line readers "proc4ops 40 num num etc"
NOTE: the "ops" hence starting in column 9 */
					nfs_brk->nfs_v4s_names_count = atol(&proc[P_NFSD].line[lineno][9]);
					len=strlen(proc[P_NFSD].line[lineno]);
					for(j=0,i=11; i<len && j<nfs_brk->nfs_v4s_names_count; i++) {
						if(proc[P_NFSD].line[lineno][i] == ' ') {
							p->nfs.v4s[j] =atol(&proc[P_NFSD].line[lineno][i+1]);
							nfs_brk->nfs_v4s_found=1;
							j++;
						}
					}
				}
			}
		}
	}


	void proc_kernel (struct kernel_brk * kernel_brk)
	{
		if (kernel_brk == NULL)
			return;
		struct proc_file * proc = kernel_brk->ext->proc;
		struct data * p = kernel_brk->ext->p;
		int i;
		p->cpu_total.uptime=0.0;
		p->cpu_total.idletime=0.0;
		p->cpu_total.uptime=atof(proc[P_UPTIME].line[0]);
		for(i=0;i<strlen(proc[P_UPTIME].line[0]);i++) {
			if(proc[P_UPTIME].line[0][i] == ' ') {
				p->cpu_total.idletime=atof(&proc[P_UPTIME].line[0][i+1]);
				break;
			}
		}

		sscanf(&proc[P_LOADAVG].line[0][0], "%f %f %f",
				&p->cpu_total.mins1,
				&p->cpu_total.mins5,
				&p->cpu_total.mins15);

	}

	char *proc_find_sb(char * p)
	{
		for(; *p != 0;p++)
			if(*p == ' ' && *(p+1) == '(')
				return p;
		return 0;
	}


	void proc_disk_io(struct disk_brk * brk)
	{
		if (brk == NULL)
			return;
		int diskline;
		int i;
		int ret;
		char *str;
		int fudged_busy;
		struct proc_file * proc = brk->ext->proc;
		struct data * p = brk->ext->p;

		brk->disks = 0;
		for(diskline=0;diskline<proc[P_STAT].lines;diskline++) {
			if(strncmp("disk_io", proc[P_STAT].line[diskline],7) == 0)
				break;
		}
		for(i=8;i<strlen(proc[P_STAT].line[diskline]);i++) {
			if( proc[P_STAT].line[diskline][i] == ':')
				brk->disks++;
		}

		str=&proc[P_STAT].line[diskline][0];
		for(i=0;i < brk->disks;i++) {
			str=proc_find_sb(str);
			if(str == 0)
				break;
			ret = sscanf(str, " (%d,%d):(%ld,%ld,%ld,%ld,%ld",
					&p->dk[i].dk_major,
					&p->dk[i].dk_minor,
					&p->dk[i].dk_noinfo,
					&p->dk[i].dk_reads,
					&p->dk[i].dk_rkb,
					&p->dk[i].dk_writes,
					&p->dk[i].dk_wkb);
			if(ret != 7)
				exit(7);
			p->dk[i].dk_xfers = p->dk[i].dk_noinfo;
			/* blocks  are 512 bytes*/
			p->dk[i].dk_rkb = p->dk[i].dk_rkb/2;
			p->dk[i].dk_wkb = p->dk[i].dk_wkb/2;

			p->dk[i].dk_bsize = (p->dk[i].dk_rkb+p->dk[i].dk_wkb)/p->dk[i].dk_xfers*1024;

			/* assume a disk does 200 op per second */
			fudged_busy = (p->dk[i].dk_reads + p->dk[i].dk_writes)/2;
			if(fudged_busy > 100*brk->ext->elapsed)
				p->dk[i].dk_time += 100*brk->ext->elapsed;
			p->dk[i].dk_time = fudged_busy;

			sprintf(p->dk[i].dk_name,"dev-%d-%d",p->dk[i].dk_major,p->dk[i].dk_minor);
			/*      fprintf(stderr,"disk=%d name=\"%s\" major=%d minor=%d\n", i,p->dk[i].dk_name, p->dk[i].dk_major,p->dk[i].dk_minor); */
			str++;
		}
	}

	void proc_diskstats(struct disk_brk * brk)
	{
		static FILE *fp = (FILE *)-1;
		char buf[1024];
		int i;
		int ret;
		struct data * p = brk->ext->p;

		if( fp == (FILE *)-1) {
			if( (fp = fopen("/proc/diskstats","r")) == NULL) {
				/* DEBUG if( (fp = fopen("diskstats","r")) == NULL) { */
				error("failed to open - /proc/diskstats");
				brk->disks = 0;
				return;
			}
			}
			/*
			   2    0 fd0 1 0 2 13491 0 0 0 0 0 13491 13491
			   3    0 hda 41159 53633 1102978 620181 39342 67538 857108 4042631 0 289150 4668250
			   3    1 hda1 58209 58218 0 0
			   3    2 hda2 148 4794 10 20
			   3    3 hda3 65 520 0 0
			   3    4 hda4 35943 1036092 107136 857088
			   22    0 hdc 167 5394 22308 32250 0 0 0 0 0 22671 32250 <-- USB !!
			   8    0 sda 990 2325 4764 6860 9 3 12 417 0 6003 7277
			   8    1 sda1 3264 4356 12 12
			   */
			for(i=0;i < brk->diskmax; ) {
				if(fgets(buf,1024,fp) == NULL)
					break;
				/* zero the data ready for reading */
				p->dk[i].dk_major =
					p->dk[i].dk_minor =
					p->dk[i].dk_name[0] =
					p->dk[i].dk_reads =
					p->dk[i].dk_rmerge =
					p->dk[i].dk_rkb =
					p->dk[i].dk_rmsec =
					p->dk[i].dk_writes =
					p->dk[i].dk_wmerge =
					p->dk[i].dk_wkb =
					p->dk[i].dk_wmsec =
					p->dk[i].dk_inflight =
					p->dk[i].dk_time =
					p->dk[i].dk_backlog =0;

				ret = sscanf(&buf[0], "%d %d %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
						&p->dk[i].dk_major,
						&p->dk[i].dk_minor,
						&p->dk[i].dk_name[0],
						&p->dk[i].dk_reads,
						&p->dk[i].dk_rmerge,
						&p->dk[i].dk_rkb,
						&p->dk[i].dk_rmsec,
						&p->dk[i].dk_writes,
						&p->dk[i].dk_wmerge,
						&p->dk[i].dk_wkb,
						&p->dk[i].dk_wmsec,
						&p->dk[i].dk_inflight,
						&p->dk[i].dk_time,
						&p->dk[i].dk_backlog );
				if(ret == 7) { /* shuffle the data around due to missing columns for partitions */
					p->dk[i].dk_partition = 1;
					p->dk[i].dk_wkb = p->dk[i].dk_rmsec;
					p->dk[i].dk_writes = p->dk[i].dk_rkb;
					p->dk[i].dk_rkb = p->dk[i].dk_rmerge;
					p->dk[i].dk_rmsec=0;
					p->dk[i].dk_rmerge=0;

				}
				else if(ret == 14) p->dk[i].dk_partition = 0;
				else fprintf(stderr,"disk sscanf wanted 14 but returned=%d line=%s\n",
						ret,buf);

				p->dk[i].dk_rkb /= 2; /* sectors = 512 bytes */
				p->dk[i].dk_wkb /= 2;
				p->dk[i].dk_xfers = p->dk[i].dk_reads + p->dk[i].dk_writes;
				if(p->dk[i].dk_xfers == 0)
					p->dk[i].dk_bsize = 0;
				else
					p->dk[i].dk_bsize = ((p->dk[i].dk_rkb + p->dk[i].dk_wkb) / p->dk[i].dk_xfers)*1024;

				p->dk[i].dk_time /= 10.0; /* in milli-seconds to make it upto 100%, 1000/100 = 10 */

				if( p->dk[i].dk_xfers > 0)
					i++;
			}
			if(reread) {
				fclose(fp);
				fp = (FILE *)-1;
			} else rewind(fp);
			brk->disks = i;
		}



		void proc_partitions(struct disk_brk * brk)
		{
			static FILE *fp = (FILE *)-1;
			char buf[1024];
			int i = 0;
			int ret;
			struct data * p = brk->ext->p;

			if( fp == (FILE *)-1) {
				if( (fp = fopen("/proc/partitions","r")) == NULL) {
					error("failed to open - /proc/partitions");
					partitions=0;
					return;
				}
			}
			if(fgets(buf,1024,fp) == NULL) goto end; /* throw away the header lines */
			if(fgets(buf,1024,fp) == NULL) goto end;
			/*
			   major minor  #blocks  name     rio rmerge rsect ruse wio wmerge wsect wuse running use aveq

			   33     0    1052352 hde 2855 15 2890 4760 0 0 0 0 -4 7902400 11345292
			   33     1    1050304 hde1 2850 0 2850 3930 0 0 0 0 0 3930 3930
			   3     0   39070080 hda 9287 19942 226517 90620 8434 25707 235554 425790 -12 7954830 33997658
			   3     1   31744408 hda1 651 90 5297 2030 0 0 0 0 0 2030 2030
			   3     2    6138720 hda2 7808 19561 218922 79430 7299 20529 222872 241980 0 59950 321410
			   3     3     771120 hda3 13 41 168 80 0 0 0 0 0 80 80
			   3     4          1 hda4 0 0 0 0 0 0 0 0 0 0 0
			   3     5     408208 hda5 812 241 2106 9040 1135 5178 12682 183810 0 11230 192850
			   */
			for(i=0; i < brk->diskmax; i++) {
				if(fgets(buf,1024,fp) == NULL)
					break;
				strip_spaces(buf);
				ret = sscanf(&buf[0], "%d %d %lu %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
						&p->dk[i].dk_major,
						&p->dk[i].dk_minor,
						&p->dk[i].dk_blocks,
						(char *)&p->dk[i].dk_name,
						&p->dk[i].dk_reads,
						&p->dk[i].dk_rmerge,
						&p->dk[i].dk_rkb,
						&p->dk[i].dk_rmsec,
						&p->dk[i].dk_writes,
						&p->dk[i].dk_wmerge,
						&p->dk[i].dk_wkb,
						&p->dk[i].dk_wmsec,
						&p->dk[i].dk_inflight,
						&p->dk[i].dk_use,
						&p->dk[i].dk_aveq
						);
				p->dk[i].dk_rkb /= 2; /* sectors = 512 bytes */
				p->dk[i].dk_wkb /= 2;
				p->dk[i].dk_xfers = p->dk[i].dk_rkb + p->dk[i].dk_wkb;
				if(p->dk[i].dk_xfers == 0)
					p->dk[i].dk_bsize = 0;
				else
					p->dk[i].dk_bsize = (p->dk[i].dk_rkb+p->dk[i].dk_wkb)/p->dk[i].dk_xfers*1024;

				p->dk[i].dk_time /= 10.0; /* in milli-seconds to make it upto 100%, 1000/100 = 10 */

				if(ret != 15) {
#ifdef DEBUG
					if(debug)fprintf(stderr,"sscanf wanted 15 returned = %d line=%s\n", ret,buf);
#endif /*DEBUG*/
					partitions_short = 1;
				} else partitions_short = 0;
			}
end:
			if(reread) {
				fclose(fp);
				fp = (FILE *)-1;
			} else rewind(fp);
			brk->disks = i;
		}


		void proc_disk(struct disk_brk * brk)
		{
			if (brk == NULL)
				return;
			struct stat buf;
			int ret;

			if(brk->disk_mode == 0) {
				ret = stat("/proc/diskstats", &buf);
				if(ret == 0) {
					brk->disk_mode=DISK_MODE_DISKSTATS;
				} else {
					ret = stat("/proc/partitions", &buf);
					if(ret == 0) {
						brk->disk_mode=DISK_MODE_PARTITIONS;
					} else {
						brk->disk_mode=DISK_MODE_IO;
					}
				}
			}
			switch(brk->disk_mode){
				case DISK_MODE_IO:              proc_disk_io(brk);   break;
				case DISK_MODE_DISKSTATS:       proc_diskstats(brk); break;
				case DISK_MODE_PARTITIONS:      proc_partitions(brk); break;
			}
		}
#undef isdigit
#define isdigit(ch) ( ( '0' <= (ch)  &&  (ch) >= '9')? 0: 1 )

		long proc_mem_search(struct proc_file * proc, char *s)
		{
			int i;
			int j;
			int len;
			len=strlen(s);
			for(i=0;i<proc[P_MEMINFO].lines;i++ ) {
				if( !strncmp(s, proc[P_MEMINFO].line[i],len) ) {
					for(j=len;
							!isdigit(proc[P_MEMINFO].line[i][j]) &&
							proc[P_MEMINFO].line[i][j] != 0;
							j++)
						/* do nothing */ ;
					return atol( &proc[P_MEMINFO].line[i][j]);
				}
			}
			return -1;
		}

		void proc_mem (struct mem_brk * mem_brk)
		{
			if (mem_brk == NULL)
				return;

			struct proc_file * proc = mem_brk->ext->proc;
			struct data * p = mem_brk->ext->p;
			if(proc[P_MEMINFO].read_this_interval == 0)
				proc_read(proc, P_MEMINFO);

			p->mem.memtotal   = proc_mem_search(proc, "MemTotal");
			p->mem.memfree    = proc_mem_search(proc, "MemFree");
			p->mem.memshared  = proc_mem_search(proc, "MemShared");
			p->mem.buffers    = proc_mem_search(proc, "Buffers");
			p->mem.cached     = proc_mem_search(proc, "Cached");
			p->mem.swapcached = proc_mem_search(proc, "SwapCached");
			p->mem.active     = proc_mem_search(proc, "Active");
			p->mem.inactive   = proc_mem_search(proc, "Inactive");
			p->mem.hightotal  = proc_mem_search(proc, "HighTotal");
			p->mem.highfree   = proc_mem_search(proc, "HighFree");
			p->mem.lowtotal   = proc_mem_search(proc, "LowTotal");
			p->mem.lowfree    = proc_mem_search(proc, "LowFree");
			p->mem.swaptotal  = proc_mem_search(proc, "SwapTotal");
			p->mem.swapfree   = proc_mem_search(proc, "SwapFree");
#ifdef LARGEMEM
			p->mem.dirty         = proc_mem_search(proc, "Dirty");
			p->mem.writeback     = proc_mem_search(proc, "Writeback");
			p->mem.mapped        = proc_mem_search(proc, "Mapped");
			p->mem.slab          = proc_mem_search(proc, "Slab");
			p->mem.committed_as  = proc_mem_search(proc, "Committed_AS");
			p->mem.pagetables    = proc_mem_search(proc, "PageTables");
			p->mem.hugetotal     = proc_mem_search(proc, "HugePages_Total");
			p->mem.hugefree      = proc_mem_search(proc, "HugePages_Free");
			p->mem.hugesize      = proc_mem_search(proc, "Hugepagesize");
#else
			p->mem.bigfree       = proc_mem_search(proc, "BigFree");
#endif /*LARGEMEM*/
		}


