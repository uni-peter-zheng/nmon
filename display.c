#include "lmon.h"
#include "stat.h"
#include "proc.h"
#include "display.h"
#include "func.h"
#include "struct.h"

inline void display (WINDOW * pad, int * x, int rows)
{
	*x = *x + (rows) + 1;
}

void show_mem (struct mem_brk * mem_brk, struct lparcfg_brk * lparcfg_brk)
{
	if (mem_brk == NULL || lparcfg_brk == NULL)
		return;

	int * x = &mem_brk->ext->x;
	struct data * p = mem_brk->ext->p;
	double elapsed = mem_brk->ext->elapsed;
	struct global_data * g_data = mem_brk->ext;

	BANNER(g_data->pad,*x,"Memory Stats");
	mvwprintw(g_data->pad,*x+1, 1, "               RAM     High      Low     Swap    Page Size=%d KB", mem_brk->pagesize/1024);
	mvwprintw(g_data->pad,*x+2, 1, "Total MB    %8.1f %8.1f %8.1f %8.1f ",
			p->mem.memtotal/1024.0,
			p->mem.hightotal/1024.0,
			p->mem.lowtotal/1024.0,
			p->mem.swaptotal/1024.0);
	mvwprintw(g_data->pad,*x+3, 1, "Free  MB    %8.1f %8.1f %8.1f %8.1f ",
			p->mem.memfree/1024.0,
			p->mem.highfree/1024.0,
			p->mem.lowfree/1024.0,
			p->mem.swapfree/1024.0);
	mvwprintw(g_data->pad,*x+4, 1, "Free Percent %7.1f%% %7.1f%% %7.1f%% %7.1f%% ",
			p->mem.memfree  == 0 ? 0.0 : 100.0*(float)p->mem.memfree/(float)p->mem.memtotal,
			p->mem.highfree == 0 ? 0.0 : 100.0*(float)p->mem.highfree/(float)p->mem.hightotal,
			p->mem.lowfree  == 0 ? 0.0 : 100.0*(float)p->mem.lowfree/(float)p->mem.lowtotal,
			p->mem.swapfree == 0 ? 0.0 : 100.0*(float)p->mem.swapfree/(float)p->mem.swaptotal);
	mvwprintw(g_data->pad,*x+5, 1, "            MB                  MB                  MB");
#ifdef LARGEMEM
	mvwprintw(g_data->pad,*x+6, 1, "                     Cached=%8.1f     Active=%8.1f",
			p->mem.cached/1024.0,
			p->mem.active/1024.0);
#else
	mvwprintw(g_data->pad,*x+6, 1, " Shared=%8.1f     Cached=%8.1f     Active=%8.1f",
			p->mem.memshared/1024.0,
			p->mem.cached/1024.0,
			p->mem.active/1024.0);
	mvwprintw(g_data->pad,*x+5, 68, "MB");
	mvwprintw(g_data->pad,*x+6, 55, "bigfree=%8.1f",
			p->mem.bigfree/1024);
#endif /*LARGEMEM*/
	mvwprintw(g_data->pad,*x+7, 1, "Buffers=%8.1f Swapcached=%8.1f  Inactive =%8.1f",
			p->mem.buffers/1024.0,
			p->mem.swapcached/1024.0,
			p->mem.inactive/1024.0);

	mvwprintw(g_data->pad,*x+8, 1, "Dirty  =%8.1f Writeback =%8.1f  Mapped   =%8.1f",
			p->mem.dirty/1024.0,
			p->mem.writeback/1024.0,
			p->mem.mapped/1024.0);
	mvwprintw(g_data->pad,*x+9, 1, "Slab   =%8.1f Commit_AS =%8.1f PageTables=%8.1f",
			p->mem.slab/1024.0,
			p->mem.committed_as/1024.0,
			p->mem.pagetables/1024.0);
#ifdef POWER
	if(lparcfg_brk->lparcfg.cmo_enabled == 0)
		mvwprintw(g_data->pad,*x+10, 1, "AMS is not active");
	else
		mvwprintw(g_data->pad,*x+10, 1, "AMS id=%d Weight=%-3d pmem=%ldMB hpi=%.1f/s hpit=%.1f(sec) Pool=%ldMB Loan=%ldKB     ",
				(int)lparcfg_brk->lparcfg.entitled_memory_pool_number,
				(int)lparcfg_brk->lparcfg.entitled_memory_weight,
				(long)(lparcfg_brk->lparcfg.backing_memory)/1024/1024,
				(double)(lparcfg_brk->lparcfg.cmo_faults_diff)/elapsed,
				(double)(lparcfg_brk->lparcfg.cmo_fault_time_usec_diff)/1000/1000/elapsed,
				(long)lparcfg_brk->lparcfg.entitled_memory_pool_size/1024/1024,
				(long)lparcfg_brk->lparcfg.entitled_memory_loan_request/1024);

	display(g_data->pad, x, 11);
#else /* POWER */
	display(g_data->pad, x, 10);
#endif /* POWER */
	/* for testing large page
	   p->mem.hugefree = 250;
	   p->mem.hugetotal = 1000;
	   p->mem.hugesize = 16*1024;
	   */
}

void show_disk(struct disk_brk * brk)
{
	if (brk == NULL)
		return;
	int j = 0;
	int k = 0;
	int * x = &brk->ext->x;
	int colour = brk->ext->colour;
	struct data * p = brk->ext->p;
	struct data * q = brk->ext->q;
	double elapsed = brk->ext->elapsed;
	struct global_data * g_data = brk->ext;

	if(brk->show_disk) {
		BANNER(g_data->pad,*x,"Disk I/O");
		switch(brk->disk_mode) {
			case DISK_MODE_PARTITIONS: mvwprintw(g_data->pad, *x+0, 12, "/proc/partitions");break;
			case DISK_MODE_DISKSTATS:  mvwprintw(g_data->pad, *x+0, 12, "/proc/diskstats");break;
			case DISK_MODE_IO:         mvwprintw(g_data->pad, *x+0, 12, "/proc/stat+disk_io");break;
		}
		mvwprintw(g_data->pad,*x+0, 31, "mostly in KB/s");
		mvwprintw(g_data->pad,*x+0, 50, "Warning:contains duplicates");
		switch (brk->show_disk) {
			case SHOW_DISK_STATS:
				mvwprintw(g_data->pad,*x+1, 0, "DiskName Busy    Read    Write       Xfers   Size  Peak%%  Peak-RW    InFlight ");
				break;
			case SHOW_DISK_GRAPH:
				mvwprintw(g_data->pad,*x+1, 0, "DiskName Busy  ");
				if(colour) wattrset(g_data->pad,COLOR_PAIR(6));
				mvwprintw(g_data->pad,*x+1, 15, "Read ");
				if(colour) wattrset(g_data->pad,COLOR_PAIR(3));
				mvwprintw(g_data->pad,*x+1, 20, "Write");
				if(colour) wattrset(g_data->pad,COLOR_PAIR(0));
				mvwprintw(g_data->pad,*x+1, 25, "KB|0          |25         |50          |75       100|");
				break;
		}
	}

	if(brk->disk_first_time) {
		mvwprintw(g_data->pad,*x+2, 0, "Please wait - collecting disk data");
	} else {
		double total_disk_read  = 0.0;
		double total_disk_write = 0.0;
		double total_disk_xfers = 0.0;
		int disk_mb = 0;
		int i = 0;
		char * str_p;

		for (i = 0,k=0; i < brk->disks; i++) {
			brk->disk_read = DKDELTA(dk_rkb) / elapsed;
			brk->disk_write = DKDELTA(dk_wkb) / elapsed;
			if((brk->show_disk == SHOW_DISK_GRAPH) && (brk->disk_read > 9999.9 || brk->disk_write > 9999.9)) {
				disk_mb=1;
				if(colour) wattrset(g_data->pad, COLOR_PAIR(1));
				mvwprintw(g_data->pad,*x+1, 25, "MB");
				if(colour) wattrset(g_data->pad, COLOR_PAIR(0));
				break;
			}
		}

		for (i = 0,k=0; i < brk->disks; i++) {
			if(brk->disk_only_mode && is_dgroup_name(brk, p->dk[i].dk_name) == 0)
				continue;

			/*
			   if(p->dk[i].dk_name[0] == 'h')
			   continue;
			   */
			brk->disk_busy = DKDELTA(dk_time) / elapsed;
			brk->disk_read = DKDELTA(dk_rkb) / elapsed;
			brk->disk_write = DKDELTA(dk_wkb) / elapsed;
			brk->disk_xfers = DKDELTA(dk_xfers);

			total_disk_read  += brk->disk_read;
			total_disk_write += brk->disk_write;
			total_disk_xfers += brk->disk_xfers;

			if(brk->disk_busy_peak[i] < brk->disk_busy)
				brk->disk_busy_peak[i] = brk->disk_busy;
			if(brk->disk_rate_peak[i] < (brk->disk_read + brk->disk_write))
				brk->disk_rate_peak[i] = brk->disk_read + brk->disk_write;
			if(!brk->ext->show_all && brk->disk_busy < 1)
				continue;

			if(strlen(p->dk[i].dk_name) > 8)
				str_p = &p->dk[i].dk_name[strlen(p->dk[i].dk_name) -8];
			else
				str_p = &p->dk[i].dk_name[0];

			if(brk->show_disk == SHOW_DISK_STATS) {
				/* output disks stats */
				mvwprintw(g_data->pad,*x+2 + k, 0, "%-8s %3.0f%% %8.1f %8.1fKB/s %6.1f %5.1fKB  %3.0f%% %9.1fKB/s %3d",
						str_p,
						brk->disk_busy,
						brk->disk_read,
						brk->disk_write,
						brk->disk_xfers / elapsed,
						brk->disk_xfers == 0.0 ? 0.0 :
						(DKDELTA(dk_rkb) + DKDELTA(dk_wkb) ) / brk->disk_xfers,
						brk->disk_busy_peak[i],
						brk->disk_rate_peak[i],
						p->dk[i].dk_inflight);
				k++;
			}
			if(brk->show_disk == SHOW_DISK_GRAPH) {
				/* output disk bar graphs */
				if(disk_mb) mvwprintw(g_data->pad,*x+2 + k, 0, "%-8s %3.0f%% %6.1f %6.1f",
						str_p,
						brk->disk_busy,
						brk->disk_read / 1024.0,
						brk->disk_write / 1024.0);
				else mvwprintw(g_data->pad,*x+2 + k, 0, "%-8s %3.0f%% %6.1f %6.1f",
						str_p,
						brk->disk_busy,
						brk->disk_read,
						brk->disk_write);
				mvwprintw(g_data->pad,*x+2 + k, 27, "|                                                  ");
				wmove(g_data->pad,*x+2 + k, 28);
				if(brk->disk_busy >100)
					brk->disk_busy=100;
				if( brk->disk_busy > 0.0 && (brk->disk_write + brk->disk_read) > 0.1) {
					/* 50 columns in the disk graph area so divide % by two */
					brk->readers = brk->disk_busy * brk->disk_read / (brk->disk_write + brk->disk_read) / 2;
					brk->writers = brk->disk_busy * brk->disk_write / (brk->disk_write + brk->disk_read) /2;
					if(brk->readers + brk->writers > 50) {
						brk->readers=0;
						brk->writers=0;
					}
					/* don't go beyond row 78 i.e. j = 28 + 50 */
					for (j = 0; j < brk->readers && j<50; j++) {
						if(colour) wattrset(g_data->pad,COLOR_PAIR(12));
						wprintw(g_data->pad,"R");
						if(colour) wattrset(g_data->pad,COLOR_PAIR(0));
					}
					for (; j < brk->readers + brk->writers && j < 50; j++) {
						if(colour) wattrset(g_data->pad,COLOR_PAIR(11));
						wprintw(g_data->pad,"W");
						if(colour) wattrset(g_data->pad,COLOR_PAIR(0));
					}
					for (j = brk->disk_busy; j < 50; j++)
						wprintw(g_data->pad," ");
				} else {
					for (j = 0; j < 50; j++)
						wprintw(g_data->pad," ");
					if(p->dk[i].dk_time == 0.0)
						mvwprintw(g_data->pad,*x+2 + k, 27, "| disk busy not available");
				}
				if(brk->disk_busy_peak[i] >100)
					brk->disk_busy_peak[i]=100;

				mvwprintw(g_data->pad,*x+2 + i, 77, "|");
				/* check rounding has not got the peak ">" over the 100% */
				j = 28+(int)(brk->disk_busy_peak[i]/2);
				if(j>77)
					j=77;
				mvwprintw(g_data->pad,*x+2 + i, j, ">");
				k++;
			}
		}

		mvwprintw(g_data->pad,*x+2 + k, 0, "Totals Read-MB/s=%-8.1f Writes-MB/s=%-8.1f Transfers/sec=%-8.1f",
				total_disk_read  / 1024.0,
				total_disk_write / 1024.0,
				total_disk_xfers / elapsed);

	}

	display(g_data->pad, x, 3 + k);
}

void show_cpu_info (struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk, struct utsname * uts)
{
	if (cpuinfo_brk == NULL || lparcfg_brk == NULL)
		return;

	int * x = &cpuinfo_brk->ext->x;
	struct proc_file * proc = &cpuinfo_brk->ext->proc;
	struct global_data * g_data = cpuinfo_brk->ext;

	BANNER(g_data->pad,*x, "Linux and Processor Details");
	mvwprintw(g_data->pad,*x + 1, 4, "Linux: %s", proc[P_VERSION].line[0]);
	mvwprintw(g_data->pad,*x+2, 4, "Build: %s", proc[P_VERSION].line[1]);
	mvwprintw(g_data->pad,*x+3, 4, "Release  : %s", uts->release);
	mvwprintw(g_data->pad,*x+4, 4, "Version  : %s", uts->version);
#ifdef POWER
	mvwprintw(g_data->pad,*x+5, 4, "cpuinfo: %s", proc[P_CPUINFO].line[1]);
	mvwprintw(g_data->pad,*x+6, 4, "cpuinfo: %s", proc[P_CPUINFO].line[2]);
	mvwprintw(g_data->pad,*x+7, 4, "cpuinfo: %s", proc[P_CPUINFO].line[3]);
	mvwprintw(g_data->pad,*x+8, 4, "cpuinfo: %s", proc[P_CPUINFO].line[proc[P_CPUINFO].lines-1]);
	/* needs lparcfg to be already processed */
	switch (cpuinfo_brk->power_vm_type) {
		case VM_POWERKVM_GUEST:
			mvwprintw(g_data->pad,*x+9, 4, "PowerKVM Guest Physcal CPU:%d & Virtual CPU (SMT):%d  %s", lparcfg_brk->lparcfg.partition_active_processors, cpuinfo_brk->cpus, cpuinfo_brk->endian);
			break;
		case VM_POWERKVM_HOST:
			mvwprintw(g_data->pad,*x+9, 4, "PowerKVM Host Physical CPU:%d  %s", cpuinfo_brk->cpus, cpuinfo_brk->endian);
			break;
		case VM_POWERVM:
			mvwprintw(g_data->pad,*x+9, 4, "PowerVM Physcal CPU:%d & Logical CPU:%d  %s", lparcfg_brk->lparcfg.partition_active_processors, cpuinfo_brk->cpus, cpuinfo_brk->endian);
			break;
		case VM_NATIVE:
			mvwprintw(g_data->pad,*x+9, 4, "Native Mode Physical CPU:%d  %s", cpuinfo_brk->cpus, cpuinfo_brk->endian);
			break;
	}
#else
#ifdef MAINFRAME
	mvwprintw(g_data->pad,*x+5, 4, "cpuinfo: %s", proc[P_CPUINFO].line[1]);
	mvwprintw(g_data->pad,*x+6, 4, "cpuinfo: %s", proc[P_CPUINFO].line[2]);
	mvwprintw(g_data->pad,*x+7, 4, "cpuinfo: %s", proc[P_CPUINFO].line[3]);
	mvwprintw(g_data->pad,*x+8, 4, "cpuinfo: %s", proc[P_CPUINFO].line[4]);
#else /* Intel is the default */
	mvwprintw(g_data->pad,*x+5, 4, "cpuinfo: %s %s", cpuinfo_brk->vendor_ptr, cpuinfo_brk->model_ptr);
	mvwprintw(g_data->pad,*x+6, 4, "cpuinfo: Hz=%s bogomips=%s", cpuinfo_brk->mhz_ptr, cpuinfo_brk->bogo_ptr);
	if(cpuinfo_brk->processorchips || cpuinfo_brk->cores || cpuinfo_brk->hyperthreads || cpuinfo_brk->cpus) {
		mvwprintw(g_data->pad,*x+7, 4, "cpuinfo: ProcessorChips=%d PhyscalCores=%d", cpuinfo_brk->processorchips, cpuinfo_brk->cores);
		mvwprintw(g_data->pad,*x+8, 4, "cpuinfo: Hyperthreads  =%d VirtualCPUs =%d", cpuinfo_brk->hyperthreads, cpuinfo_brk->cpus);
	}
	/*
	   mvwprintw(brk->padcpu,5, 4, "cpuinfo: %s", proc[P_CPUINFO].line[4]);
	   mvwprintw(brk->padcpu,6, 4, "cpuinfo: %s", proc[P_CPUINFO].line[1]);
	   mvwprintw(brk->padcpu,7, 4, "cpuinfo: %s", proc[P_CPUINFO].line[6]);
	   mvwprintw(brk->padcpu,8, 4, "cpuinfo: %s", proc[P_CPUINFO].line[17]);
	   */
#endif /*MAINFRAME*/
	mvwprintw(g_data->pad,*x+9, 4, "# of CPUs: %d", cpuinfo_brk->cpus);
#endif /*POWER*/
	mvwprintw(g_data->pad,*x+10, 4,"Machine  : %s", uts->machine);
	mvwprintw(g_data->pad,*x+11, 4,"Nodename : %s", uts->nodename);
	mvwprintw(g_data->pad,*x+12, 4,"/etc/*ease[1]: %s", cpuinfo_brk->easy[0]);
	mvwprintw(g_data->pad,*x+13, 4,"/etc/*ease[2]: %s", cpuinfo_brk->easy[1]);
	mvwprintw(g_data->pad,*x+14, 4,"/etc/*ease[3]: %s", cpuinfo_brk->easy[2]);
	mvwprintw(g_data->pad,*x+15, 4,"/etc/*ease[4]: %s", cpuinfo_brk->easy[3]);
	mvwprintw(g_data->pad,*x+16, 4,"lsb_release: %s", cpuinfo_brk->lsb_release[0]);
	mvwprintw(g_data->pad,*x+17, 4,"lsb_release: %s", cpuinfo_brk->lsb_release[1]);
	mvwprintw(g_data->pad,*x+18, 4,"lsb_release: %s", cpuinfo_brk->lsb_release[2]);
	mvwprintw(g_data->pad,*x+19, 4,"lsb_release: %s", cpuinfo_brk->lsb_release[3]);
	display(g_data->pad, x, 20);
}

void show_top_info (struct top_brk * top_brk, unsigned long pagesize, double ignore_procdisk_threshold)
{
	if (top_brk == NULL)
		return;
	char * formatstring;
	int i;
	int j;
	int n;
	int * x = &top_brk->ext->x;
	int y = top_brk->ext->y;
	struct data * p = top_brk->ext->p;
	struct data * q = top_brk->ext->q;
	char pgrp[32];
	double elapsed = top_brk->ext->elapsed;
	struct global_data * g_data = top_brk->ext;
	int show_count = top_brk->show_count;

	wmove(g_data->pad,*x+1, 1);
	wclrtobot(g_data->pad);
	/* Get the details of the running processes */
	top_brk->skipped = 0;

	n = get_progress_num(p);
	if (top_brk->topper_size < n) {
		top_brk->topper = REALLOC(top_brk->topper, sizeof(struct topper ) * (n+1) ); /* add one to avoid overrun */
		top_brk->topper_size = n;
	}
	/* Sort the processes by CPU utilisation */
	for ( i = 0, top_brk->max_sorted = 0; i < n; i++) {
		/* move forward in the previous array to find a match*/
		for(j=0;j < q->nprocs;j++) {
			if (p->procs[i].pi_pid == q->procs[j].pi_pid) { /* found a match */
				top_brk->topper[top_brk->max_sorted].index = i;
				top_brk->topper[top_brk->max_sorted].other = j;
				top_brk->topper[top_brk->max_sorted].time =  TIMEDELTA(pi_utime,i,j) +
					TIMEDELTA(pi_stime,i,j);
				top_brk->topper[top_brk->max_sorted].size =  p->procs[i].statm_resident;
#define COUNTDELTA(brk,member) ( (q->procs[brk->topper[j].other].member > p->procs[i].member) ? 0 : (p->procs[i].member  - q->procs[brk->topper[j].other].member) )
				if(top_brk->ext->isroot)
					top_brk->topper[top_brk->max_sorted].io =  COUNTDELTA(top_brk,read_io) + COUNTDELTA(top_brk,write_io);
				top_brk->max_sorted++;
				break;
			}
		}
	}
	switch(top_brk->show_topmode) {
		default:
		case 3: qsort((void *) & top_brk->topper[0], top_brk->max_sorted, sizeof(struct topper ), &cpu_compare );
				break;
		case 4: qsort((void *) & top_brk->topper[0], top_brk->max_sorted, sizeof(struct topper ), &size_compare );
				break;
		case 5: qsort((void *) & top_brk->topper[0], top_brk->max_sorted, sizeof(struct topper ), &disk_compare );
				break;
	}
	BANNER(g_data->pad,*x,"Top Processes");
	if(top_brk->ext->isroot) {
		mvwprintw(g_data->pad,*x+0, 15, "Procs=%d mode=%d (1=Basic, 3=Perf 4=Size 5=I/O)", n, top_brk->show_topmode);
	} else {
		mvwprintw(g_data->pad,*x+0, 15, "Procs=%d mode=%d (1=Basic, 3=Perf 4=Size 5=(root-only))", n, top_brk->show_topmode);
	}
	if(top_brk->top_first_time) {
		mvwprintw(g_data->pad,*x+1, 1, "Please wait - information being collected");
	}
	else {
		switch (top_brk->show_topmode) {
			case 1:
				mvwprintw(g_data->pad,*x+1, 1, "  PID      PPID  Pgrp Nice Prior Status    proc-Flag Command");
				for (j = 0; j < show_count; j++) {
					i = top_brk->topper[j].index;
					if (p->procs[i].pi_pgrp == p->procs[i].pi_pid)
						strcpy(pgrp, "none");
					else
						sprintf(&pgrp[0], "%d", p->procs[i].pi_pgrp);
					/* skip over processes with 0 CPU */
					if(!top_brk->ext->show_all && (top_brk->topper[j].time/elapsed < ignore_procdisk_threshold) && !top_brk->cmdfound)
						break;
					//if( *x + j + 2 - top_brk->skipped > LINES+2) /* +2 to for safety :-) */
					//	break;
					mvwprintw(g_data->pad,*x+j + 2 - top_brk->skipped, 1, "%7d %7d %6s %4d %4d %9s 0x%08x %1s %-32s",
							p->procs[i].pi_pid,
							p->procs[i].pi_ppid,
							pgrp,
							p->procs[i].pi_nice,
							p->procs[i].pi_pri,

							(top_brk->topper[j].time * 100 / elapsed) ? "Running "
							: get_state(p->procs[i].pi_state),
							p->procs[i].pi_flags,
							(p->procs[i].pi_tty_nr ? "F" : " "),
							p->procs[i].pi_comm);
				}
				break;
			case 3:
			case 4:
			case 5:
				if(top_brk->show_args == ARGS_ONLY)  {
					formatstring = "  PID    %%CPU ResSize    Command                                            ";
				} else if(COLS > 119) {
					if(top_brk->show_topmode == 5)
						formatstring = "  PID       %%CPU    Size     Res    Res     Res     Res    Shared   StorageKB Command";
					else
						formatstring = "  PID       %%CPU    Size     Res    Res     Res     Res    Shared    Faults   Command";
				} else {
					if(top_brk->show_topmode == 5)
						formatstring = "  PID    %%CPU  Size   Res   Res   Res   Res Shared StorageKB Command";
					else
						formatstring = "  PID    %%CPU  Size   Res   Res   Res   Res Shared   Faults  Command";
				}
				mvwprintw(g_data->pad,*x+1, y, formatstring);

				if(top_brk->show_args == ARGS_ONLY) {
					formatstring = "         Used      KB                                                        ";
				} else if(COLS > 119) {
					if(top_brk->show_topmode == 5)
						formatstring = "            Used      KB     Set    Text    Data     Lib    KB    Read Write";
					else
						formatstring = "            Used      KB     Set    Text    Data     Lib    KB     Min   Maj";
				} else {
					if(top_brk->show_topmode == 5)
						formatstring = "         Used    KB   Set  Text  Data   Lib    KB ReadWrite ";
					else
						formatstring = "         Used    KB   Set  Text  Data   Lib    KB  Min  Maj ";
				}
				mvwprintw(g_data->pad,*x+2,1, formatstring);
				for (j = 0; j < show_count; j++) {
					i = top_brk->topper[j].index;
					if(!top_brk->ext->show_all) {
						/* skip processes with zero CPU/io */
						if(top_brk->show_topmode == 3 && (top_brk->topper[j].time/elapsed) < ignore_procdisk_threshold && !top_brk->cmdfound)
							break;
						if(top_brk->show_topmode == 5 && (top_brk->topper[j].io < 0.1 && !top_brk->cmdfound))
							break;
					}
					//if( *x + j + 3 - top_brk->skipped > LINES + 2) /* +2 to for safety :-) XYZXYZ*/
					//	break;
					if(top_brk->cmdfound && !cmdcheck(p->procs[i].pi_comm)) {
						top_brk->skipped++;
						show_count++;
						continue;
					}
					if(top_brk->show_args == ARGS_ONLY){
						mvwprintw(g_data->pad,*x+j + 3 - top_brk->skipped, 1,
								"%7d %5.1f %7lu %-120s",
								p->procs[i].pi_pid,
								top_brk->topper[j].time / elapsed,
								p->procs[i].statm_resident*pagesize/1024, /* in KB */
								args_lookup(p->procs[i].pi_pid,
									p->procs[i].pi_comm));
					}
					else {
						if(COLS > 119)
							formatstring = "%8d %7.1f %7lu %7lu %7lu %7lu %7lu %5lu %6d %6d %-32s";
						else
							formatstring = "%7d %5.1f %5lu %5lu %5lu %5lu %5lu %5lu %4d %4d %-32s";

						mvwprintw(g_data->pad,*x+j + 3 - top_brk->skipped, 1, formatstring,
								p->procs[i].pi_pid,
								top_brk->topper[j].time/elapsed,
								/* top_brk->topper[j].time /1000.0 / elapsed,*/
								p->procs[i].statm_size*pagesize/1024UL, /* in KB */
								p->procs[i].statm_resident*pagesize/1024UL, /* in KB */
								p->procs[i].statm_trs*pagesize/1024UL, /* in KB */
								p->procs[i].statm_drs*pagesize/1024UL, /* in KB */
								p->procs[i].statm_lrs*pagesize/1024UL, /* in KB */
								p->procs[i].statm_share*pagesize/1024UL, /* in KB */
								top_brk->show_topmode == 5 ? (int)(COUNTDELTA(top_brk,read_io)  / elapsed / 1024) : (int)(COUNTDELTA(top_brk,pi_minflt) / elapsed),
								top_brk->show_topmode == 5 ? (int)(COUNTDELTA(top_brk,write_io) / elapsed / 1024) : (int)(COUNTDELTA(top_brk,pi_majflt) / elapsed),
								p->procs[i].pi_comm);
					}
				}
				break;
		}
	}
	display(g_data->pad, x, 3 + j);
}

void show_kernel(struct kernel_brk * kernel_brk)
{
	if (kernel_brk == NULL)
		return;

	int updays, uphours, upmins;
	float average;
	int * x = &kernel_brk->ext->x;
	struct data * p = kernel_brk->ext->p;
	struct data * q = kernel_brk->ext->q;
	double elapsed = kernel_brk->ext->elapsed;
	struct global_data * g_data = kernel_brk->ext;

	BANNER(g_data->pad,*x,"Kernel Stats");
	mvwprintw(g_data->pad,*x+1, 1, "RunQueue       %8lld   Load Average    CPU use since boot time",
			p->cpu_total.running);
	updays=p->cpu_total.uptime/60/60/24;
	uphours=(p->cpu_total.uptime-updays*60*60*24)/60/60;
	upmins=(p->cpu_total.uptime-updays*60*60*24-uphours*60*60)/60;
	mvwprintw(g_data->pad,*x+2, 1, "ContextSwitch  %8.1f    1 mins %5.2f    Uptime Days=%3d Hours=%2d Mins=%2d",
			(float)(p->cpu_total.ctxt - q->cpu_total.ctxt) / elapsed,
			(float)p->cpu_total.mins1,
			updays, uphours, upmins);
	updays=p->cpu_total.idletime/60/60/24;
	uphours=(p->cpu_total.idletime-updays*60*60*24)/60/60;
	upmins=(p->cpu_total.idletime-updays*60*60*24-uphours*60*60)/60;
	mvwprintw(g_data->pad,*x+3, 1, "Forks          %8.1f    5 mins %5.2f    Idle   Days=%3d Hours=%2d Mins=%2d",
			(float)(p->cpu_total.procs - q->cpu_total.procs)/elapsed,
			(float)p->cpu_total.mins5,
			updays, uphours, upmins);

	mvwprintw(g_data->pad,*x+4, 1, "Interrupts     %8.1f   15 mins %5.2f",
			(float)(p->cpu_total.intr - q->cpu_total.intr)/elapsed,
			(float)p->cpu_total.mins15);
	average = (p->cpu_total.uptime - p->cpu_total.idletime)/ p->cpu_total.uptime * 100.0;
	if( average > 0.0)
		mvwprintw(g_data->pad,*x+4, 46, "Average CPU use=%6.2f%%", average);
	else
		mvwprintw(g_data->pad,*x+4, 46, "Uptime has overflowed");
	display(g_data->pad, x, 5);
}

void show_lpar(struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk)
{
	if (lparcfg_brk == NULL)
		return;

	int ret = lparcfg_brk->result;
	int * x = &lparcfg_brk->ext->x;
	struct global_data * g_data = lparcfg_brk->ext;
	double elapsed = lparcfg_brk->ext->elapsed;

	BANNER(g_data->pad,*x,"LPAR Stats");
	if(ret == 0) {
		mvwprintw(g_data->pad,*x+2, 0, "Reading data from /proc/ppc64/lparcfg failed");
		mvwprintw(g_data->pad,*x+3, 0, "This is probably a Native Virtual Machine");
	} else
		if(cpuinfo_brk->power_vm_type == VM_POWERKVM_HOST || cpuinfo_brk->power_vm_type == VM_POWERKVM_GUEST) {
			mvwprintw(g_data->pad,*x+2, 0, "Reading data from /proc/ppc64/lparcfg mostly failed");
			mvwprintw(g_data->pad,*x+3, 0, "PowerKVM does not many of these stats");
		} else {
			mvwprintw(g_data->pad,*x+1, 0, "LPAR=%d  SerialNumber=%s  Type=%s",
					lparcfg_brk->lparcfg.partition_id, lparcfg_brk->lparcfg.serial_number, lparcfg_brk->lparcfg.system_type);
			mvwprintw(g_data->pad,*x+2, 0, "Flags:      Shared-CPU=%-5s  Capped=%-5s   SMT-mode=%d",
					lparcfg_brk->lparcfg.shared_processor_mode?"true":"false",
					lparcfg_brk->lparcfg.capped?"true":"false",
					lparcfg_brk->lparcfg.smt_mode);
			mvwprintw(g_data->pad,*x+3, 0, "Systems CPU Pool=%8.2f          Active=%8.2f    Total=%8.2f",
					(float)lparcfg_brk->lparcfg.pool_capacity,
					(float)lparcfg_brk->lparcfg.system_active_processors,
					(float)lparcfg_brk->lparcfg.system_potential_processors);
			mvwprintw(g_data->pad,*x+4, 0, "LPARs CPU    Min=%8.2f     Entitlement=%8.2f      Max=%8.2f",
					lparcfg_brk->lparcfg.MinEntCap/100.0,
					lparcfg_brk->lparcfg.partition_entitled_capacity/100.0,
					lparcfg_brk->lparcfg.partition_max_entitled_capacity/100.0);
			mvwprintw(g_data->pad,*x+5, 0, "Virtual CPU  Min=%8.2f          VP Now=%8.2f      Max=%8.2f",
					(float)lparcfg_brk->lparcfg.MinProcs,
					(float)lparcfg_brk->lparcfg.partition_active_processors,
					(float)lparcfg_brk->lparcfg.partition_potential_processors);
			mvwprintw(g_data->pad,*x+6, 0, "Memory       Min= unknown             Now=%8.2f      Max=%8.2f",
					(float)lparcfg_brk->lparcfg.MinMem,
					(float)lparcfg_brk->lparcfg.DesMem);
			mvwprintw(g_data->pad,*x+7, 0, "Other     Weight=%8.2f   UnallocWeight=%8.2f Capacity=%8.2f",
					(float)lparcfg_brk->lparcfg.capacity_weight,
					(float)lparcfg_brk->lparcfg.unallocated_capacity_weight,
					(float)lparcfg_brk->lparcfg.CapInc/100.0);

			mvwprintw(g_data->pad,*x+8, 0, "      BoundThrds=%8.2f UnallocCapacity=%8.2f  Increment",
					(float)lparcfg_brk->lparcfg.BoundThrds,
					(float)lparcfg_brk->lparcfg.unallocated_capacity);
			if(lparcfg_brk->lparcfg.purr_diff == 0 || lparcfg_brk->lparcfg.timebase <1) {
				mvwprintw(g_data->pad,*x+9, 0, "lparcfg: purr field always zero, upgrade to SLES9+sp1 or RHEL4+u1");
			} else {
				if(lparcfg_brk->lpar_first_time) {
					mvwprintw(g_data->pad,*x+9, 0, "Please wait gathering data");

				} else {
					mvwprintw(g_data->pad,*x+9, 0, "Physical CPU use=%8.3f ",
							(double)lparcfg_brk->lparcfg.purr_diff/(double)lparcfg_brk->lparcfg.timebase/elapsed);
					if( lparcfg_brk->lparcfg.pool_idle_time != NUMBER_NOT_VALID && lparcfg_brk->lparcfg.pool_idle_saved != 0)
						mvwprintw(g_data->pad,*x+9, 29, "PoolIdleTime=%8.2f",
								(double)lparcfg_brk->lparcfg.pool_idle_diff/(double)lparcfg_brk->lparcfg.timebase/elapsed);
					mvwprintw(g_data->pad,*x+9, 54, "[timebase=%lld]", lparcfg_brk->lparcfg.timebase);
				}
			}
		}
	display(g_data->pad, x, 10);
}


void show_large(struct large_brk * large_brk)
{
	if (large_brk == NULL)
		return;

	int * x = &large_brk->ext->x;
	struct data * p = large_brk->ext->p;
	struct global_data * g_data = large_brk->ext;

	BANNER(g_data->pad,*x,"Large (Huge) Page Stats");
	if(p->mem.hugetotal > 0) {
		if(p->mem.hugetotal - p->mem.hugefree > large_brk->huge_peak)
			large_brk->huge_peak = p->mem.hugetotal - p->mem.hugefree;
		mvwprintw(g_data->pad,*x+1, 1, "Total Pages=%7ld   100.0%%   Huge Page Size =%ld KB",    p->mem.hugetotal, p->mem.hugesize);
		mvwprintw(g_data->pad,*x+2, 1, "Used  Pages=%7ld   %5.1f%%   Used Pages Peak=%-8ld",
				(long)(p->mem.hugetotal - p->mem.hugefree),
				(p->mem.hugetotal - p->mem.hugefree)/(float)p->mem.hugetotal*100.0,
				large_brk->huge_peak);
		mvwprintw(g_data->pad,*x+3, 1, "Free  Pages=%7ld   %5.1f%%",    p->mem.hugefree, p->mem.hugefree/(float)p->mem.hugetotal*100.0);
	} else {
		mvwprintw(g_data->pad,*x+1, 1, " There are no Huge Pages");
		mvwprintw(g_data->pad,*x+2, 1, " - see /proc/meminfo");
	}
	display(g_data->pad, x, 4);
}

void show_diskmap(struct disk_brk * disk_brk)
{
	if (disk_brk == NULL)
		return;

	int i;
	int * x = &disk_brk->ext->x;
	struct data * p = disk_brk->ext->p;
	struct data * q = disk_brk->ext->q;
	double elapsed = disk_brk->ext->elapsed;
	struct global_data * g_data = disk_brk->ext;

	BANNER(g_data->pad,*x,"Disk %%Busy Map");
	mvwprintw(g_data->pad,*x+0, 18,"Key: @=90 #=80 X=70 8=60 O=50 0=40 o=30 +=20 -=10 .=5 _=0%%");
	mvwprintw(g_data->pad,*x+1, 0,"             Disk No.  1         2         3         4         5         6   ");
	if(disk_brk->disk_first_time) {
		mvwprintw(g_data->pad,*x+2, 0,"Please wait - collecting disk data");
	} else {
		mvwprintw(g_data->pad,*x+2, 0,"Disks=%-4d   0123456789012345678901234567890123456789012345678901234567890123", disk_brk->disks);
		mvwprintw(g_data->pad,*x+3, 0,"disk 0 to 63 ");
		for (i = 0; i < disk_brk->disks; i++) {
			disk_brk->disk_busy = DKDELTA(dk_time) / elapsed;
			disk_brk->disk_read = DKDELTA(dk_rkb) / elapsed;
			disk_brk->disk_write = DKDELTA(dk_wkb) / elapsed;
			/* ensure boundaries */
			if (disk_brk->disk_busy <  0)
				disk_brk->disk_busy=0;
			else
				if (disk_brk->disk_busy > 99) disk_brk->disk_busy=99;

#define MAPWRAP 64
			mvwprintw(g_data->pad,*x+3 + (int)(i/MAPWRAP), 13+ (i%MAPWRAP), "%c", disk_brk->disk_busy_map_ch[(int)disk_brk->disk_busy]);
		}
	}
	display(g_data->pad, x, 4 + disk_brk->disks/MAPWRAP);

}

void show_jfs(struct jfs_brk * jfs_brk)
{
	if (jfs_brk == NULL)
		return;

	int k;
	int i;
	int ret;
	int * x = &jfs_brk->ext->x;
	struct global_data * g_data = jfs_brk->ext;
	BANNER(g_data->pad,*x,"Filesystems");
	mvwprintw(g_data->pad,*x+1, 0, "Filesystem            SizeMB  FreeMB  Use%% Type     MountPoint");

	for (k = 0; k < jfs_brk->jfses; k++) {
		float fs_size=0;
		float fs_bsize=0;
		float fs_free=0;
		float fs_size_used=100.0;
		char * str_p;

		if(jfs_brk->jfs[k].mounted) {
			if(!strncmp(jfs_brk->jfs[k].name,"/proc/",6)       /* sub directorys have to be fake too */
					|| !strncmp(jfs_brk->jfs[k].name,"/sys/",5)
					|| !strncmp(jfs_brk->jfs[k].name,"/dev/",5)
					|| !strncmp(jfs_brk->jfs[k].name,"/proc",6) /* one more than the string to ensure the NULL */
					|| !strncmp(jfs_brk->jfs[k].name,"/sys",5)
					|| !strncmp(jfs_brk->jfs[k].name,"/dev",5)
					|| !strncmp(jfs_brk->jfs[k].name,"/rpc_pipe",10)
			  ) { /* /proc gives invalid/insane values */
				mvwprintw(g_data->pad,*x+2+k, 0, "%-14s", jfs_brk->jfs[k].name);
				mvwprintw(g_data->pad,*x+2+k, 27, "-");
				mvwprintw(g_data->pad,*x+2+k, 35, "-");
				mvwprintw(g_data->pad,*x+2+k, 41, "-");
				mvwprintw(g_data->pad,*x+2+k, 43, "%-8s not a real filesystem",jfs_brk->jfs[k].type);
			} else {
				jfs_brk->statfs_buffer.f_blocks = 0;
				if((ret = fstatfs(jfs_brk->jfs[k].fd, &(jfs_brk->statfs_buffer))) != -1) {
					if(jfs_brk->statfs_buffer.f_blocks != 0) {
						/* older Linux seemed to always report in 4KB blocks but
						   newer Linux release use the f_bsize block sizes but
						   the man statfs docs the field as the natural I/O size so
						   the blocks reported here are ambigous in size */
						if(jfs_brk->statfs_buffer.f_bsize == 0)
							fs_bsize = 4.0 * 1024.0;
						else
							fs_bsize = jfs_brk->statfs_buffer.f_bsize;
						/* convert blocks to MB */
						fs_size = (float)jfs_brk->statfs_buffer.f_blocks * fs_bsize/1024.0/1024.0;

						/* find the best size info available f_bavail is like df reports
						   otherwise use f_bsize (this includes inode blocks) */
						if(jfs_brk->statfs_buffer.f_bavail == 0)
							fs_free = (float)jfs_brk->statfs_buffer.f_bfree  * fs_bsize/1024.0/1024.0;
						else
							fs_free = (float)jfs_brk->statfs_buffer.f_bavail  * fs_bsize/1024.0/1024.0;

						/* this is a percentage */
						fs_size_used = (fs_size - (float)jfs_brk->statfs_buffer.f_bfree  * fs_bsize/1024.0/1024.0)/fs_size * 100.0;
						/* try to get the same number as df using kludge */
						/*fs_size_used += 1.0; */
						if (fs_size_used >100.0)
							fs_size_used = 100.0;

						if( (i=strlen(jfs_brk->jfs[k].device)) <20)
							str_p=&jfs_brk->jfs[k].device[0];
						else {
							str_p=&jfs_brk->jfs[k].device[i-20];
						}
						mvwprintw(g_data->pad,*x+2+k, 0, "%-20s %7.0f %7.0f %4.0f%% %-8s %s",
								str_p,
								fs_size,
								fs_free,
								ceil(fs_size_used),
								jfs_brk->jfs[k].type,
								jfs_brk->jfs[k].name
								);

					} else {
						mvwprintw(g_data->pad,*x+2+k, 0, "%s", jfs_brk->jfs[k].name);
						mvwprintw(g_data->pad,*x+2+k, 43, "%-8s size=zero blocks!", jfs_brk->jfs[k].type);
					}
				}
				else {
					mvwprintw(g_data->pad,*x+2+k, 0, "%s", jfs_brk->jfs[k].name);
					mvwprintw(g_data->pad,*x+2+k, 43, "%-8s statfs failed", jfs_brk->jfs[k].type);
				}
			}
		} else {
			mvwprintw(g_data->pad,*x+2+k, 0, "%-14s", jfs_brk->jfs[k].name);
			mvwprintw(g_data->pad,*x+2+k, 43, "%-8s not mounted",jfs_brk->jfs[k].type);
		}
	}
	display(g_data->pad, x, 2 + jfs_brk->jfses);
}

void show_net(struct net_brk * net_brk)
{
	if (net_brk == NULL)
		return;

	int * x = &net_brk->ext->x;
	int i;
	struct data * p = net_brk->ext->p;
	struct data * q = net_brk->ext->q;
	double elapsed = net_brk->ext->elapsed;
	struct global_data * g_data = net_brk->ext;

	BANNER(g_data->pad,*x,"Network I/O");
	mvwprintw(g_data->pad,*x+1, 0, "I/F Name\tRecv=Kb/s   Trans=Kb/s   packin   packout   insize=KB   outsize=KB   Peak->Recv=Kb/s   Trans=Kb/s");

#define IFDELTA(member) ((float)( (q->ifnets[i].member > p->ifnets[i].member) ? 0 : (p->ifnets[i].member - q->ifnets[i].member)/elapsed) )
#define IFDELTA_ZERO(member1,member2) ((IFDELTA(member1) == 0) || (IFDELTA(member2)== 0)? 0.0 : IFDELTA(member1)/IFDELTA(member2) )
	for (i = 0; i < net_brk->networks; i++) {
		mvwprintw(g_data->pad,*x+2 + i, 0, "%8s\t%8.1f     %8.1f   %7.1f   %7.1f    %7.1f     %7.1f        %8.1f     %8.1f   ",
				&p->ifnets[i].if_name[0],
				IFDELTA(if_ibits) / 1024.0,
				IFDELTA(if_obits) / 1024.0,
				//IFDELTA(if_ibytes) / 1024.0,
				//IFDELTA(if_obytes) / 1024.0,
				IFDELTA(if_ipackets),
				IFDELTA(if_opackets),
				IFDELTA_ZERO(if_ibytes, if_ipackets),
				IFDELTA_ZERO(if_obytes, if_opackets),
			//	net_brk->net_read_peak[i],
			//	net_brk->net_write_peak[i]
				net_brk->net_read_peak_by_bits[i],
				net_brk->net_write_peak_by_bits[i]
				);
	}
	display(g_data->pad, x, net_brk->networks + 2);

	if(net_brk->errors)
		net_brk->show_neterror = 3;
	if(net_brk->show_neterror) {
		BANNER(g_data->pad,*x,"Network Error Counters");
		mvwprintw(g_data->pad,*x+1, 0, "I/F Name\tiErrors   iDrop  iOverrun  iFrame  oErrors   oDrop oOverrun oCarrier oColls ");
		for (i = 0; i < net_brk->networks; i++) {
			mvwprintw(g_data->pad,*x+2 + i, 0, "%8s\t%7lu %7lu %7lu %7lu %7lu %7lu %7lu %7lu %7lu",
					&p->ifnets[i].if_name[0],
					p->ifnets[i].if_ierrs,
					p->ifnets[i].if_idrop,
					p->ifnets[i].if_ififo,
					p->ifnets[i].if_iframe,
					p->ifnets[i].if_oerrs,
					p->ifnets[i].if_odrop,
					p->ifnets[i].if_ofifo,
					p->ifnets[i].if_ocarrier,
					p->ifnets[i].if_ocolls);
		}
		display(g_data->pad, x, net_brk->networks + 2);
		if(net_brk->show_neterror > 0)
			net_brk->show_neterror--;
	}
}

void show_welcome(struct welcome_brk * welcome_brk, struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk)
{
	if (welcome_brk == NULL || cpuinfo_brk == NULL || lparcfg_brk == NULL)
		return;

	struct proc_file * proc = cpuinfo_brk->ext->proc;
	int * x = &welcome_brk->ext->x;
	int colour = welcome_brk->ext->colour;
	WINDOW * pad = welcome_brk->ext->pad;
	COLOUR wattrset(pad,COLOR_PAIR(2));
	*x = *x + 3;
	mvwprintw(pad,*x+1, 3, "------------------------------");
	mvwprintw(pad,*x+2, 3, "#    #  #    #   ####   #    #");
	mvwprintw(pad,*x+3, 3, "##   #  ##  ##  #    #  ##   #");
	mvwprintw(pad,*x+4, 3, "# #  #  # ## #  #    #  # #  #");
	mvwprintw(pad,*x+5, 3, "#  # #  #    #  #    #  #  # #");
	mvwprintw(pad,*x+6, 3, "#   ##  #    #  #    #  #   ##");
	mvwprintw(pad,*x+7, 3, "#    #  #    #   ####   #    #");
	mvwprintw(pad,*x+8, 3, "------------------------------");
	COLOUR wattrset(pad,COLOR_PAIR(0));
	mvwprintw(pad,*x+1, 40, "For help type H or ...");
	mvwprintw(pad,*x+2, 40, " nmon -?  - hint");
	mvwprintw(pad,*x+3, 40, " nmon -h  - full");
	mvwprintw(pad,*x+5, 40, "To start the same way every time");
	mvwprintw(pad,*x+6, 40, " set the NMON ksh variable");
	mvwprintw(pad,*x+7, 40, "If you want to collect data in background, you can use:");
	mvwprintw(pad,*x+8, 40, " nmon -s2 -c100 -f  (interval:2s count:100)");
	COLOUR wattrset(pad,COLOR_PAIR(1));
#ifdef POWER
	get_cpu_cnt(cpuinfo_brk);
	proc_read (proc, P_CPUINFO);

	get_endian(cpuinfo_brk);
	switch(cpuinfo_brk->power_vm_type) {
		case VM_POWERKVM_GUEST:
			get_cpu_cnt(cpuinfo_brk);
#ifdef RHEL7
			mvwprintw(pad, *x + 9, 3, "%s %s", cpuinfo_brk->easy[0], cpuinfo_brk->easy[1]);
#else
#ifdef SLES113
			mvwprintw(pad, *x + 9, 3, "%s", cpuinfo_brk->easy[2]);
#else
			mvwprintw(pad, *x + 9, 3, "%s", cpuinfo_brk->easy[3]);
#endif /* SLES113 */
#endif /* RHEL7 */
			mvwprintw(pad,*x+10, 3, "PowerKVM Guest %s", &proc[P_CPUINFO].line[1][7]);
			mvwprintw(pad,*x+11, 3, "PowerKVM Guest VirtualCPUs=%d LogicalCPUs=%d", (int)lparcfg_brk->lparcfg.partition_active_processors, cpuinfo_brk->cpus);
			mvwprintw(pad,*x+12, 3, "PowerKVM Guest SMT=%d", lparcfg_brk->lparcfg.smt_mode);
			break;
		case VM_POWERKVM_HOST:
			mvwprintw(pad,*x+ 9, 3, "%s", cpuinfo_brk->easy[0]);
			mvwprintw(pad,*x+10, 3, "PowerKVM Host %s", &proc[P_CPUINFO].line[1][7]);
			mvwprintw(pad,*x+11, 3, "PowerKVM Host owns all %d CPUs & SMT=off in the Hosting OS", cpuinfo_brk->cpus);
			mvwprintw(pad,*x+12, 3, "PowerKVM Host %s", proc[P_CPUINFO].line[proc[P_CPUINFO].lines-2]);
			break;
		case VM_NATIVE:
			mvwprintw(pad,*x+ 9, 3, "%s", cpuinfo_brk->easy[0]);
			mvwprintw(pad,*x+10, 3, "Native %s", &proc[P_CPUINFO].line[1][7]);
			mvwprintw(pad,*x+11, 3, "Native owns all %d CPUs & SMT=off in the Hosting OS", cpuinfo_brk->cpus);
			mvwprintw(pad,*x+12, 3, "Native %s", proc[P_CPUINFO].line[proc[P_CPUINFO].lines-2]);
			break;
		default:
		case VM_POWERVM:
			mvwprintw(pad,*x+ 9, 3, "%s", cpuinfo_brk->easy[3]);
			mvwprintw(pad,*x+10, 3, "PowerVM %s %s", &proc[P_CPUINFO].line[1][7], &proc[P_CPUINFO].line[proc[P_CPUINFO].lines-1][11]);
			mvwprintw(pad,*x+11, 3, "PowerVM Entitlement=%-6.2f VirtualCPUs=%d LogicalCPUs=%d",
					(double)lparcfg_brk->lparcfg.partition_entitled_capacity/100.0, (int)lparcfg_brk->lparcfg.partition_active_processors, cpuinfo_brk->cpus);
			mvwprintw(pad,*x+12, 3, "PowerVM SMT=%d Capped=%d", lparcfg_brk->lparcfg.smt_mode, lparcfg_brk->lparcfg.capped);
			break;
	}

	mvwprintw(pad,*x+13, 3, "Processor Clock=%s             %s", &proc[P_CPUINFO].line[2][9], cpuinfo_brk->endian);

#endif
#ifdef X86
	get_cpu_cnt(cpuinfo_brk);
	mvwprintw(pad,*x+10, 3, "x86 %s %s", vendor_ptr, model_ptr);
	mvwprintw(pad,*x+11, 3, "x86 MHz=%s bogomips=%s", mhz_ptr,bogo_ptr);
	if(processorchips || cores || hyperthreads || cpuinfo_brk->cpus) {
		mvwprintw(pad,*x+12, 3, "x86 ProcessorChips=%d PhyscalCores=%d", processorchips, cores);
		mvwprintw(pad,*x+13, 3, "x86 Hyperthreads  =%d VirtualCPUs =%d", hyperthreads, cpuinfo_brk->cpus);
	}
#endif
	mvwprintw(pad,*x+14, 3, "Spreadsheet file= %s", cpuinfo_brk->ext->log_path);
	COLOUR wattrset(pad,COLOR_PAIR(0));
	mvwprintw(pad,*x+16, 3, "Use these keys to toggle statistics on/off:");
	mvwprintw(pad,*x+17, 3, "   c = CPU        l = CPU Long-term   - = Faster screen updates");
	mvwprintw(pad,*x+18, 3, "   m = Memory     j = Filesystems     + = Slower screen updates");
	mvwprintw(pad,*x+19, 3, "   d = Disks      n = Network         V = Virtual Memory");
	mvwprintw(pad,*x+20, 3, "   r = Resource   N = NFS             v = Verbose hints");
	mvwprintw(pad,*x+21, 3, "   k = kernel     t = Top-processes   . = only busy disks/procs");
	mvwprintw(pad,*x+22, 3, "   h = more options                   q = Quit");
	*x = *x + 23;
}

void show_help (struct help_brk * help_brk)
{
	if (help_brk == NULL)
		return;

	struct global_data * g_data = help_brk->ext;
	int * x = &help_brk->ext->x;
	BANNER(g_data->pad,*x,"HELP");
	mvwprintw(g_data->pad, *x+1, 5, "key  --- statistics which toggle on/off ---");
	mvwprintw(g_data->pad, *x+2, 5, "h = This help information");
	mvwprintw(g_data->pad, *x+3, 5, "r = RS6000/pSeries CPU/cache/OS/kernel/hostname details + LPAR");
	mvwprintw(g_data->pad, *x+4, 5, "t = Top Process Stats 1=basic 3=CPU");
	mvwprintw(g_data->pad, *x+5, 5, "    u = shows command arguments (hit twice to refresh)");
	mvwprintw(g_data->pad, *x+6, 5, "c = CPU by processor             l = longer term CPU averages");
	mvwprintw(g_data->pad, *x+7, 5, "m = Memory & Swap stats L=Huge   j = JFS Usage Stats");
	mvwprintw(g_data->pad, *x+8, 5, "n = Network stats                N = NFS");
	mvwprintw(g_data->pad, *x+9, 5, "d = Disk I/O Graphs D=Stats      o = Disks %%Busy Map");
	mvwprintw(g_data->pad,*x+10, 5, "k = Kernel stats & loadavg       V = Virtual Memory");
	mvwprintw(g_data->pad,*x+11, 5, "g = User Defined Disk Groups [start nmon with -g <filename>]");
	mvwprintw(g_data->pad,*x+12, 5, "v = Verbose Simple Checks - OK/Warnings/Danger");
	mvwprintw(g_data->pad,*x+13, 5, "b = black & white mode");
	mvwprintw(g_data->pad,*x+14, 5, "--- controls ---");
	mvwprintw(g_data->pad,*x+15, 5, "+ and - = double or half the screen refresh time");
	mvwprintw(g_data->pad,*x+16, 5, "q = quit                     space = refresh screen now");
	mvwprintw(g_data->pad,*x+17, 5, ". = Minimum Mode =display only busy disks and processes");
	mvwprintw(g_data->pad,*x+18, 5, "0 = reset peak counts to zero (peak = \">\")");
	mvwprintw(g_data->pad,*x+19, 5, "Developer Nigel Griffiths see http://nmon.sourceforge.net");
	display(g_data->pad, x, 20);
}

void show_vm(struct mem_brk * mem_brk)
{
	if (mem_brk == NULL)
		return;

	int ret = mem_brk->result;
	int * x = &mem_brk->ext->x;
	struct data * p = mem_brk->ext->p;
	struct data * q = mem_brk->ext->q;
	struct global_data * g_data = mem_brk->ext;
	BANNER(g_data->pad,*x,"Virtual-Memory");
	if(ret < 0 ) {
		mvwprintw(g_data->pad,*x+2, 2, "Virtual Memory stats not supported with this kernel");
		mvwprintw(g_data->pad,*x+3, 2, "/proc/vmstat only seems to appear in 2.6 onwards");

	} else {
		if(mem_brk->vm_first_time) {
			mvwprintw(g_data->pad,*x+2, 2, "Please wait - collecting data");
		} else {
			mvwprintw(g_data->pad,*x+1, 0, "nr_dirty    =%9lld pgpgin      =%8lld",
					VMCOUNT(nr_dirty),
					VMDELTA(pgpgin));
			mvwprintw(g_data->pad,*x+2, 0, "nr_writeback=%9lld pgpgout     =%8lld",
					VMCOUNT(nr_writeback),
					VMDELTA(pgpgout));
			mvwprintw(g_data->pad,*x+3, 0, "nr_unstable =%9lld pgpswpin    =%8lld",
					VMCOUNT(nr_unstable),
					VMDELTA(pswpin));
			mvwprintw(g_data->pad,*x+4, 0, "nr_table_pgs=%9lld pgpswpout   =%8lld",
					VMCOUNT(nr_page_table_pages),
					VMDELTA(pswpout));
			mvwprintw(g_data->pad,*x+5, 0, "nr_mapped   =%9lld pgfree      =%8lld",
					VMCOUNT(nr_mapped),
					VMDELTA(pgfree));
			mvwprintw(g_data->pad,*x+6, 0, "nr_slab     =%9lld pgactivate  =%8lld",
					VMCOUNT(nr_slab),
					VMDELTA(pgactivate));
			mvwprintw(g_data->pad,*x+7, 0, "                       pgdeactivate=%8lld",
					VMDELTA(pgdeactivate));
			mvwprintw(g_data->pad,*x+8, 0, "allocstall  =%9lld pgfault     =%8lld  kswapd_steal     =%7lld",
					VMDELTA(allocstall),
					VMDELTA(pgfault),
					VMDELTA(kswapd_steal));
			mvwprintw(g_data->pad,*x+9, 0, "pageoutrun  =%9lld pgmajfault  =%8lld  kswapd_inodesteal=%7lld",
					VMDELTA(pageoutrun),
					VMDELTA(pgmajfault),
					VMDELTA(kswapd_inodesteal));
			mvwprintw(g_data->pad,*x+10, 0,"slabs_scanned=%8lld pgrotated   =%8lld  pginodesteal     =%7lld",
					VMDELTA(slabs_scanned),
					VMDELTA(pgrotated),
					VMDELTA(pginodesteal));



			mvwprintw(g_data->pad,*x+1, 46, "              High Normal    DMA");
			mvwprintw(g_data->pad,*x+2, 46, "alloc      %7lld%7lld%7lld",
					VMDELTA(pgalloc_high),
					VMDELTA(pgalloc_normal),
					VMDELTA(pgalloc_dma));
			mvwprintw(g_data->pad,*x+3, 46, "refill     %7lld%7lld%7lld",
					VMDELTA(pgrefill_high),
					VMDELTA(pgrefill_normal),
					VMDELTA(pgrefill_dma));
			mvwprintw(g_data->pad,*x+4, 46, "steal      %7lld%7lld%7lld",
					VMDELTA(pgsteal_high),
					VMDELTA(pgsteal_normal),
					VMDELTA(pgsteal_dma));
			mvwprintw(g_data->pad,*x+5, 46, "scan_kswapd%7lld%7lld%7lld",
					VMDELTA(pgscan_kswapd_high),
					VMDELTA(pgscan_kswapd_normal),
					VMDELTA(pgscan_kswapd_dma));
			mvwprintw(g_data->pad,*x+6, 46, "scan_direct%7lld%7lld%7lld",
					VMDELTA(pgscan_direct_high),
					VMDELTA(pgscan_direct_normal),
					VMDELTA(pgscan_direct_dma));
		}
	}
	display(g_data->pad, x, 11);
}

void show_nfs(struct nfs_brk * nfs_brk)
{
	if (nfs_brk == NULL)
		return;

	int show_nfs = nfs_brk->show_nfs;
	int i;
	int * x = &nfs_brk->ext->x;
	struct data * p = nfs_brk->ext->p;
	struct data * q = nfs_brk->ext->q;
	double elapsed = nfs_brk->ext->elapsed;
	struct global_data * g_data = nfs_brk->ext;

	if(nfs_brk->nfs_first_time) {
		memcpy(&q->nfs,&p->nfs,sizeof(struct nfs_stat));
	}
	if(nfs_brk->nfs_clear) {
		nfs_brk->nfs_clear = 0;
		for(i = 0; i < 25; i++)
			mvwprintw(g_data->pad,*x+i, 0, "                                                                                ");
	}
	BANNER(g_data->pad,*x,"Network Filesystem (NFS) I/O Operations per second");
	if(show_nfs == 1) {
		if(nfs_brk->nfs_v2c_found || nfs_brk->nfs_v2s_found)
			mvwprintw(g_data->pad,*x+1, 0, " Version 2        Client   Server");
		else
			mvwprintw(g_data->pad,*x+1, 0, " Version 2 not active");

		if(nfs_brk->nfs_v3c_found || nfs_brk->nfs_v3s_found)
			mvwprintw(g_data->pad,*x+1, 41, "Version 3      Client   Server");
		else
			mvwprintw(g_data->pad,*x+1, 41, " Version 3 not active");
	}
	if(show_nfs == 2 ) {
		if(nfs_brk->nfs_v4c_found)
			mvwprintw(g_data->pad,*x+1, 0, " Version 4 Client (%d Stats found)", nfs_brk->nfs_v4c_names_count);
		else
			mvwprintw(g_data->pad,*x+1, 0, " Version 4 Client side not active");
	}
	if(show_nfs == 3 ) {
		if(nfs_brk->nfs_v4s_found)
			mvwprintw(g_data->pad,*x+1, 0, " Version 4 Server (%d Stats found)", nfs_brk->nfs_v4s_names_count);
		else
			mvwprintw(g_data->pad,*x+1, 0, " Version 4 Server side not active");
	}
	float v2c_total =0;
	float v2s_total =0;
	float v3c_total =0;
	float v3s_total =0;
	float v4c_total =0;
	float v4s_total =0;
	if(nfs_brk->nfs_v3c_found || nfs_brk->nfs_v3s_found) {
		for(i=0;i<18;i++) {     /* NFS V2 Client & Server */
			if(show_nfs == 1)
				mvwprintw(g_data->pad,*x+2+i,  3, "%12s %8.1f %8.1f",
						nfs_brk->nfs_v2_names[i],
						NFS_DELTA(nfs.v2c[i]),
						NFS_DELTA(nfs.v2s[i]));
			v2c_total += NFS_DELTA(nfs.v2c[i]);
			v2s_total += NFS_DELTA(nfs.v2s[i]);
		}
	}
	if(nfs_brk->nfs_v3c_found || nfs_brk->nfs_v3s_found) {
		for(i=0;i<22;i++) {     /* NFS V3 Client & Server */
			if (show_nfs == 1)
				mvwprintw(g_data->pad,*x+2+i, 41, "%12s %8.1f %8.1f",
						nfs_brk->nfs_v3_names[i],
						NFS_DELTA(nfs.v3c[i]),
						NFS_DELTA(nfs.v3s[i]));
			v3c_total +=NFS_DELTA(nfs.v3c[i]);
			v3s_total +=NFS_DELTA(nfs.v3s[i]);
		}
	}

	if(nfs_brk->nfs_v4c_found) {
		for(i=0;i<18;i++) {     /* NFS V4 Client */
			if(show_nfs == 2) {
				mvwprintw(g_data->pad,*x+2+i, 0, "%12s%7.1f",
						nfs_brk->nfs_v4c_names[i],
						NFS_DELTA(nfs.v4c[i]));
			}
			v4c_total +=NFS_DELTA(nfs.v4c[i]);
		}
		for(i=18;i<35;i++) {    /* NFS V4 Client */
			if(show_nfs == 2) {
				mvwprintw(g_data->pad,*x+2+i-18, 20, "%12s%7.1f",
						nfs_brk->nfs_v4c_names[i],
						NFS_DELTA(nfs.v4c[i]));
			}
			v4c_total +=NFS_DELTA(nfs.v4c[i]);
		}
	}

	if(nfs_brk->nfs_v4s_found) {
		for(i=0;i<18;i++) {     /* NFS V4 Server */
			if(show_nfs == 3) {
				mvwprintw(g_data->pad,*x+2+i, 0, "%12s%7.1f",
						nfs_brk->nfs_v4s_names[i],
						NFS_DELTA(nfs.v4s[i]));
			}
			v4s_total +=NFS_DELTA(nfs.v4s[i]);
		}
		for(i=18;i<36;i++) {    /* NFS V4 Server */
			if(show_nfs == 3) {
				mvwprintw(g_data->pad,*x+2+i-18, 19, "%12s%7.1f",
						nfs_brk->nfs_v4s_names[i],
						NFS_DELTA(nfs.v4s[i]));
			}
			v4s_total +=NFS_DELTA(nfs.v4s[i]);
		}
		for(i=36;i<54 && i<nfs_brk->nfs_v4s_names_count;i++) {  /* NFS V4 Server */
			if(show_nfs == 3) {
				mvwprintw(g_data->pad,*x+2+i-36, 39, "%12s%7.1f",
						nfs_brk->nfs_v4s_names[i],
						NFS_DELTA(nfs.v4s[i]));
			}
			v4s_total +=NFS_DELTA(nfs.v4s[i]);
		}
		for(i=54;i<=70 && i<nfs_brk->nfs_v4s_names_count;i++) { /* NFS V4 Server */
			if(show_nfs == 3) {
				mvwprintw(g_data->pad,*x+2+i-54, 59, "%12s%7.1f",
						nfs_brk->nfs_v4s_names[i],
						NFS_DELTA(nfs.v4s[i]));
			}
			v4s_total +=NFS_DELTA(nfs.v4s[i]);
		}
	}
	mvwprintw(g_data->pad,*x+2+18,  1, "--NFS-Totals->---Client----Server--");
	/* if(nfs_brk->nfs_v2c_found || nfs_brk->nfs_v2s_found) */
	mvwprintw(g_data->pad,*x+2+19,  1, "NFSv2 Totals->%9.1f %9.1f", v2c_total,v2s_total);
	/* if(nfs_brk->nfs_v3c_found || nfs_brk->nfs_v3s_found)*/
	mvwprintw(g_data->pad,*x+2+20,  1, "NFSv3 Totals->%9.1f %9.1f", v3c_total,v3s_total);
	/* if(nfs_brk->nfs_v4c_found || nfs_brk->nfs_v4s_found)*/
	mvwprintw(g_data->pad,*x+2+21,  1, "NFSv4 Totals->%9.1f %9.1f", v4c_total,v4s_total);

	display(g_data->pad, x, 24);
}


void show_smp(struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk, struct lparcfg_brk * lparcfg_brk)
{
	if (smp_brk == NULL || cpuinfo_brk == NULL || lparcfg_brk == NULL)
		return;

	int i;
	int * x = &smp_brk->ext->x;
	int colour = smp_brk->ext->colour;
	struct data * p = smp_brk->ext->p;
	struct data * q = smp_brk->ext->q;
	double elapsed = smp_brk->ext->elapsed;
	struct global_data * g_data = smp_brk->ext;
	struct proc_file * proc = smp_brk->ext->proc;
	BANNER(g_data->pad,*x,"CPU Utilisation");

	/* mvwprintw(g_data->pad,1, 0, cpu_line);*/
	/*
	 *mvwprintw(g_data->pad,2, 0, "CPU  User%%  Sys%% Wait%% Idle|0          |25         |50          |75       100|");
	 */
	mvwprintw(g_data->pad,*x+1, 0, smp_brk->cpu_line);
	mvwprintw(g_data->pad,*x+2, 0, "CPU  ");
	COLOUR wattrset(g_data->pad, COLOR_PAIR(2));
	mvwprintw(g_data->pad,*x+2, 4, "User%%");
	COLOUR wattrset(g_data->pad, COLOR_PAIR(1));
	mvwprintw(g_data->pad,*x+2, 9, "  Sys%%");
	COLOUR wattrset(g_data->pad, COLOR_PAIR(4));
	mvwprintw(g_data->pad,*x+2, 15, " Wait%%");
	if(p->cpu_total.steal != q->cpu_total.steal){
		COLOUR wattrset(g_data->pad, COLOR_PAIR(5));
		mvwprintw(g_data->pad,*x+2, 22, "Steal");
	} else {
		COLOUR wattrset(g_data->pad, COLOR_PAIR(0));
		mvwprintw(g_data->pad,*x+2, 22, " Idle");
	}
	COLOUR wattrset(g_data->pad, COLOR_PAIR(0));
	mvwprintw(g_data->pad,*x+2, 27, "|0          |25         |50          |75       100|");
#ifdef POWER
	/* Always get lpar info as well so we can report physical CPU usage
	 * to make data more meaningful
	 * This assumes that LPAR info is available in q and p !
	 */
	if( lparcfg_brk->result > 0 )       {
		if( lparcfg_brk->lparcfg.shared_processor_mode == 1)    {
			if(lparcfg_brk->lparcfg.timebase == -1) {
				lparcfg_brk->lparcfg.timebase=0;
				for(i=0;i < smp_brk->ext->proc[P_CPUINFO].lines-1;i++) {
					if(!strncmp("timebase",smp_brk->ext->proc[P_CPUINFO].line[i],8)) {
						sscanf(smp_brk->ext->proc[P_CPUINFO].line[i],"timebase : %lld",&lparcfg_brk->lparcfg.timebase);
						break;
					}
				}
			}
			else {
				/* PowerKVM Host or Guest or Native have not Entitlement stats */
				if(cpuinfo_brk->power_vm_type == VM_POWERVM)
					mvwprintw(g_data->pad,*x+1,30,"EntitledCPU=% 6.3f",
							(double)lparcfg_brk->lparcfg.partition_entitled_capacity/100.0);
				/* Only if the calculation is working */
				if(lparcfg_brk->lparcfg.purr_diff != 0 )
					mvwprintw(g_data->pad,*x+1,50,"PhysicalCPUused=% 7.3f",
							(double)lparcfg_brk->lparcfg.purr_diff/(double)lparcfg_brk->lparcfg.timebase/elapsed);
			}
		}
	}
#endif
	for (i = 0; i < cpuinfo_brk->cpus; i++) {
		cpuinfo_brk->cpu_user = p->cpuN[i].user - q->cpuN[i].user;
		cpuinfo_brk->cpu_sys  = p->cpuN[i].sys  - q->cpuN[i].sys;
		cpuinfo_brk->cpu_wait = p->cpuN[i].wait - q->cpuN[i].wait;
		cpuinfo_brk->cpu_idle = p->cpuN[i].idle - q->cpuN[i].idle;
		cpuinfo_brk->cpu_steal= p->cpuN[i].steal- q->cpuN[i].steal;
		/* DEBUG inject steal       cpuinfo_brk->cpu_steal = cpuinfo_brk->cpu_sys; */
		cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle + cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys + cpuinfo_brk->cpu_wait + cpuinfo_brk->cpu_steal;
		/* Check if we had a CPU # change and have to set idle to 100 */
		if( cpuinfo_brk->cpu_sum == 0)
			cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle = 100.0;
		if(smp_brk->smp_first_time) {
			if(i == 0)
				mvwprintw(g_data->pad,*x+3 + i, 27, "| Please wait gathering CPU statistics");
			else
				mvwprintw(g_data->pad,*x+3 + i, 27, "|");
			mvwprintw(g_data->pad,*x+3 + i, 77, "|");
		} else {
#ifdef POWER
			/* lparcfg gathered above */
			if( lparcfg_brk->lparcfg.smt_mode > 1 &&  i % lparcfg_brk->lparcfg.smt_mode == 0) {
				mvwprintw(g_data->pad,*x+3 + i, 27, "*");
				mvwprintw(g_data->pad,*x+3 + i, 77, "*");
			}
#endif
			if(!smp_brk->show_raw)
				plot_smp_show(smp_brk, cpuinfo_brk, lparcfg_brk, i+1, *x+3 + i,
						(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_steal / (double)cpuinfo_brk->cpu_sum * 100.0);
			else
				save_smp_show(smp_brk,i+1, *x+3+i,
						RAW(user) - RAW(nice),
						RAW(sys),
						RAW(wait),
						RAW(idle),
						RAW(nice),
						RAW(irq),
						RAW(softirq),
						RAW(steal));
#ifdef POWER
			/* lparcfg gathered above */
			if( lparcfg_brk->lparcfg.smt_mode > 1 &&  i % lparcfg_brk->lparcfg.smt_mode == 0) {
				mvwprintw(g_data->pad,*x+3 + i, 27, "*");
				mvwprintw(g_data->pad,*x+3 + i, 77, "*");
			}
#endif

		}
	}       /* for (i = 0; i < cpuinfo_brk->cpus; i++) */
	mvwprintw(g_data->pad,*x+i + 3, 0, smp_brk->cpu_line);

#ifdef POWER
	/* proc_lparcfg called above in previous ifdef
	*/
	if( lparcfg_brk->lparcfg.shared_processor_mode == 1)    {
		if(lparcfg_brk->lparcfg.timebase == -1) {
			lparcfg_brk->lparcfg.timebase=0;
			for(i=0; i < smp_brk->ext->proc[P_CPUINFO].lines-1; i++) {
				if(!strncmp("timebase",smp_brk->ext->proc[P_CPUINFO].line[i],8)) {
					sscanf(smp_brk->ext->proc[P_CPUINFO].line[i],"timebase : %lld",&lparcfg_brk->lparcfg.timebase);
					break;
				}
			}
		}
		else {
			mvwprintw(g_data->pad,*x+i+3,29,"%s", lparcfg_brk->lparcfg.shared_processor_mode ? "Shared": "Dedicsted");
			mvwprintw(g_data->pad,*x+i+3,39,"|");
			/* PowerKVM has no Capped concept */
			if(cpuinfo_brk->power_vm_type == VM_POWERVM)
				mvwprintw(g_data->pad,*x+i+3,41,"%s", lparcfg_brk->lparcfg.capped ? "--Capped": "Uncapped");
			mvwprintw(g_data->pad,*x+i+3,51,"|");
			mvwprintw(g_data->pad,*x+i+3,54,"SMT=%d", lparcfg_brk->lparcfg.smt_mode);
			mvwprintw(g_data->pad,*x+i+3,64,"|");
			mvwprintw(g_data->pad,*x+i+3,67,"VP=%.0f", (float)lparcfg_brk->lparcfg.partition_active_processors);
		}
	}
#endif
	cpuinfo_brk->cpu_user = p->cpu_total.user - q->cpu_total.user;
	cpuinfo_brk->cpu_sys  = p->cpu_total.sys  - q->cpu_total.sys;
	cpuinfo_brk->cpu_wait = p->cpu_total.wait - q->cpu_total.wait;
	cpuinfo_brk->cpu_idle = p->cpu_total.idle - q->cpu_total.idle;
	cpuinfo_brk->cpu_steal= p->cpu_total.steal- q->cpu_total.steal;
	/* DEBUG inject steal       cpuinfo_brk->cpu_steal = cpuinfo_brk->cpu_sys; */
	cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle + cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys + cpuinfo_brk->cpu_wait + cpuinfo_brk->cpu_steal;

	/* Check if we had a CPU # change and have to set idle to 100 */
	if( cpuinfo_brk->cpu_sum == 0)
		cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle = 100.0;

	if (cpuinfo_brk->cpus > 1) {
		if(!smp_brk->smp_first_time) {
			if(!smp_brk->show_raw) {
				plot_smp_show(smp_brk, cpuinfo_brk, lparcfg_brk, 0, *x+4 + i,
						(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_steal/ (double)cpuinfo_brk->cpu_sum * 100.0);
			} else {
				save_smp_show(smp_brk,0, *x+4 + i,
						RAWTOTAL(user) - RAWTOTAL(nice),
						RAWTOTAL(sys),
						RAWTOTAL(wait),
						RAWTOTAL(idle),
						RAWTOTAL(nice),
						RAWTOTAL(irq),
						RAWTOTAL(softirq),
						RAWTOTAL(steal));
			}
		}
		mvwprintw(g_data->pad, *x+i + 5, 0, smp_brk->cpu_line);
		i = i + 2;
	} /* if (cpuinfo_brk->cpus > 1) */

#ifdef POWER
	if( lparcfg_brk->lparcfg.shared_processor_mode == 1)	{
		if(lparcfg_brk->lparcfg.timebase == -1) {
			lparcfg_brk->lparcfg.timebase=0;
			for(i=0; i < proc[P_CPUINFO].lines-1;i++) {
				if(!strncmp("timebase",proc[P_CPUINFO].line[i],8)) {
					sscanf(proc[P_CPUINFO].line[i],"timebase : %lld",&lparcfg_brk->lparcfg.timebase);
					break;
				}
			}
		}
		else {
			mvwprintw(g_data->pad,*x+i+3,29,"%s", lparcfg_brk->lparcfg.shared_processor_mode ? "Shared": "Dedicsted");
			mvwprintw(g_data->pad,*x+i+3,39,"|");
			/* PowerKVM has no Capped concept */
			if(cpuinfo_brk->power_vm_type == VM_POWERVM)
				mvwprintw(g_data->pad,*x+i+3,41,"%s", lparcfg_brk->lparcfg.capped ? "--Capped": "Uncapped");
			mvwprintw(g_data->pad,*x+i+3,51,"|");
			mvwprintw(g_data->pad,*x+i+3,54,"SMT=%d", lparcfg_brk->lparcfg.smt_mode);
			mvwprintw(g_data->pad,*x+i+3,64,"|");
			mvwprintw(g_data->pad,*x+i+3,67,"VP=%.0f", (float)lparcfg_brk->lparcfg.partition_active_processors);
		}
	}
#endif

	display(g_data->pad, x, i + 4);
}

void show_longterm(struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;

	int * x = &cpuinfo_brk->ext->x;
	struct data * p = cpuinfo_brk->ext->p;
	struct data * q = cpuinfo_brk->ext->q;
	struct global_data * g_data = cpuinfo_brk->ext;
	cpuinfo_brk->cpu_user = p->cpu_total.user - q->cpu_total.user;
	cpuinfo_brk->cpu_sys  = p->cpu_total.sys  - q->cpu_total.sys;
	cpuinfo_brk->cpu_wait = p->cpu_total.wait - q->cpu_total.wait;
	cpuinfo_brk->cpu_idle = p->cpu_total.idle - q->cpu_total.idle;
	cpuinfo_brk->cpu_steal= p->cpu_total.steal- q->cpu_total.steal;
	/* DEBUG inject steal       cpuinfo_brk->cpu_steal = cpuinfo_brk->cpu_sys; */
	cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle + cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys + cpuinfo_brk->cpu_wait + cpuinfo_brk->cpu_steal;

	if(cpuinfo_brk->next_cpu_snap >= MAX_SNAPS) {
		cpuinfo_brk->next_cpu_snap = 0;
		cpuinfo_brk->cpu_snap_all = 1;
	}
	save_snap(cpuinfo_brk,
			(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_steal/ (double)cpuinfo_brk->cpu_sum * 100.0);
	plot_snap(cpuinfo_brk);
	display(g_data->pad, x, MAX_SNAP_ROWS + 2);
}

void show_dgroup(struct disk_brk * disk_brk)
{
	if (disk_brk == NULL)
		return;

	int n;
	int k;
	int i;
	int j;
	int * x = &disk_brk->ext->x;
	struct data * p = disk_brk->ext->p;
	struct data * q = disk_brk->ext->q;
	double elapsed = disk_brk->ext->elapsed;
	struct global_data * g_data = disk_brk->ext;

	BANNER(g_data->pad,*x,"Disk-Group-I/O");
	if (disk_brk->dgroup_loaded != 2 || disk_brk->dgroup_total_disks == 0) {
		mvwprintw(g_data->pad, *x+1, 1, "No Disk Groups found use -g groupfile when starting nmon");
		n = 0;
	} else if (disk_brk->disk_first_time) {
		mvwprintw(g_data->pad, *x+1, 1, "Please wait - collecting disk data");
	} else {
		mvwprintw(g_data->pad, *x+1, 1, "Name          Disks AvgBusy Read-KB/s|Write  TotalMB/s   xfers/s BlockSizeKB");
		float total_busy   = 0.0;
		float total_rbytes = 0.0;
		float total_wbytes = 0.0;
		float total_xfers  = 0.0;
		for(k = n = 0; k < disk_brk->dgroup_total_groups; k++) {
			disk_brk->disk_busy = 0.0;
			disk_brk->disk_read = 0.0;
			disk_brk->disk_write = 0.0;
			disk_brk->disk_xfers  = 0.0;
			for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
				i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
				if (i != -1) {
					disk_brk->disk_busy   += DKDELTA(dk_time) / elapsed;
					disk_brk->disk_read += DKDELTA(dk_rkb) / elapsed;
					disk_brk->disk_write += DKDELTA(dk_wkb) / elapsed;
					disk_brk->disk_xfers  += DKDELTA(dk_xfers) / elapsed;
				}
			}
			if (disk_brk->dgroup_disks[k] == 0)
				disk_brk->disk_busy = 0.0;
			else
				disk_brk->disk_busy = disk_brk->disk_busy / disk_brk->dgroup_disks[k];
			total_busy += disk_brk->disk_busy;
			total_rbytes += disk_brk->disk_read;
			total_wbytes += disk_brk->disk_write;
			total_xfers  += disk_brk->disk_xfers;
			if ((disk_brk->disk_read + disk_brk->disk_write) == 0 || disk_brk->disk_xfers == 0)
				disk_brk->disk_size = 0.0;
			else
				disk_brk->disk_size = ((float)disk_brk->disk_read + (float)disk_brk->disk_write) / (float)disk_brk->disk_xfers;
			mvwprintw(g_data->pad, *x+n + 2, 1, "%-14s   %3d %5.1f%% %9.1f|%-9.1f %6.1f %9.1f %6.1f ",
					disk_brk->dgroup_name[k],
					disk_brk->dgroup_disks[k],
					disk_brk->disk_busy,
					disk_brk->disk_read,
					disk_brk->disk_write,
					(disk_brk->disk_read + disk_brk->disk_write) / 1024, /* in MB */
					disk_brk->disk_xfers,
					disk_brk->disk_size
					);
			n++;
		}
		mvwprintw(g_data->pad, *x+n + 2, 1, "Groups=%2d TOTALS %3d %5.1f%% %9.1f|%-9.1f %6.1f %9.1f",
				n,
				disk_brk->dgroup_total_disks,
				total_busy / disk_brk->dgroup_total_disks,
				total_rbytes,
				total_wbytes,
				(((double)total_rbytes + (double)total_wbytes)) / 1024, /* in MB */
				total_xfers
				);
	}
	display(g_data->pad, x, 3 + disk_brk->dgroup_total_groups);
}

void show_verbose(struct cpuinfo_brk * cpuinfo_brk, struct disk_brk * disk_brk)
{
	if (cpuinfo_brk == NULL || disk_brk == NULL)
		return;

	struct global_data * g_data = cpuinfo_brk->ext;
	int * x = &g_data->x;
	double top_disk_busy = 0.0;
	char *top_disk_name = "";
	int i,k;

	BANNER(g_data->pad, *x, "Verbose Mode");
	mvwprintw(g_data->pad, *x + 1, 0, " Code    Resource            Stats   Now\tWarn\tDanger ");
	cpuinfo_brk->cpu_user = g_data->p->cpu_total.user - g_data->q->cpu_total.user;
	cpuinfo_brk->cpu_sys  = g_data->p->cpu_total.sys  - g_data->q->cpu_total.sys;
	cpuinfo_brk->cpu_wait = g_data->p->cpu_total.wait - g_data->q->cpu_total.wait;
	cpuinfo_brk->cpu_idle = g_data->p->cpu_total.idle - g_data->q->cpu_total.idle;
	cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle + cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys + cpuinfo_brk->cpu_wait;

	cpuinfo_brk->cpu_busy= (double)(cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys)/ (double)cpuinfo_brk->cpu_sum * 100.0;
	mvwprintw(g_data->pad, *x + 2, 0, "        -> CPU               %%busy %5.1f%%\t>80%%\t>90%%          ",cpuinfo_brk->cpu_busy);
	if(cpuinfo_brk->cpu_busy > 90.0){
		if(g_data->colour)  wattrset(g_data->pad,COLOR_PAIR(1));
		mvwprintw(g_data->pad,*x + 2, 0, " DANGER");
	}
	else if(cpuinfo_brk->cpu_busy > 80.0) {
		if(g_data->colour)  wattrset(g_data->pad,COLOR_PAIR(4));
		mvwprintw(g_data->pad,*x + 2, 0, "Warning");
	}
	else  {
		if(g_data->colour)  wattrset(g_data->pad,COLOR_PAIR(2));
		mvwprintw(g_data->pad,*x + 2, 0, "     OK");
	}
	if(g_data->colour)  
		wattrset(g_data->pad,COLOR_PAIR(0));

	for (i = 0, k = 0; i < disk_brk->disks; i++) {
		disk_brk->disk_busy = (g_data->q->dk[i].dk_time > g_data->p->dk[i].dk_time) ? 0 : (g_data->p->dk[i].dk_time - g_data->q->dk[i].dk_time) / g_data->elapsed;
		if( disk_brk->disk_busy > top_disk_busy) {
			top_disk_busy = disk_brk->disk_busy;
			top_disk_name = g_data->p->dk[i].dk_name;
		}
	}
	if(top_disk_busy > 80.0) {
		if(g_data->colour)  wattrset(g_data->pad,COLOR_PAIR(1));
		mvwprintw(g_data->pad, *x + 3, 0, " DANGER");
	}
	else if(top_disk_busy > 60.0) {
		if(g_data->colour)  wattrset(g_data->pad,COLOR_PAIR(4));
		mvwprintw(g_data->pad, *x + 3, 0, "Warning");
	}
	else  {
		if(g_data->colour)  wattrset(g_data->pad,COLOR_PAIR(2));
		mvwprintw(g_data->pad,*x + 3, 0, "     OK");
	}
	if(g_data->colour)  
		wattrset(g_data->pad,COLOR_PAIR(0));
	mvwprintw(g_data->pad,*x + 3, 8, "-> Top Disk %8s %%busy %5.1f%%\t>40%%\t>60%%          ",top_disk_name,top_disk_busy);
	display(g_data->pad, x, 6);
}


