/*************************************************************************
  > File Name: collect.c
  > Author: ToLiMit
  > Mail: 348958453@qq.com
  > Created Time: Wed 28 Oct 2015 02:05:08 PM CST
 ************************************************************************/

#include "lmon.h"
#include "func.h"
#include "struct.h"
#include "stat.h"

void collect_disk (struct disk_brk * disk_brk)
{
	if (disk_brk == NULL)
		return;

	int loop = disk_brk->ext->loop;
	double elapsed = disk_brk->ext->elapsed;
	struct data * p = disk_brk->ext->p;
	struct data * q = disk_brk->ext->q;
	int show_rrd = disk_brk->ext->show_rrd;
	FILE * fp_ss = disk_brk->ext->fp_ss;
	int i = 0;
	double  ftmp = 0.0;

	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i,disk_brk))
			fprintf(fp_ss,show_rrd ? "%srrdtool update diskbusy%s.rrd %s" : "%sDISKBUSY%s,%s",i == 0 ? "": "\n", dskgrp(disk_brk,i), LOOP(loop,show_rrd));
		/* check percentage is correct */
		ftmp = DKDELTA(dk_time) / elapsed;
		if(ftmp > 100.0 || ftmp < 0.0)
			fprintf(fp_ss,show_rrd ? ":U" : ",101.00");
		else
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					DKDELTA(dk_time) / elapsed);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i,disk_brk))
			fprintf(fp_ss,show_rrd ? "\nrrdtool update diskread%s.rrd %s" : "\nDISKREAD%s,%s", dskgrp(disk_brk,i), LOOP(loop,show_rrd));
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
				DKDELTA(dk_rkb) / elapsed);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i,disk_brk))
			fprintf(fp_ss,show_rrd ? "\nrrdtool update diskwrite%s.rrd %s" : "\nDISKWRITE%s,%s", dskgrp(disk_brk,i), LOOP(loop,show_rrd));
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
				DKDELTA(dk_wkb) / elapsed);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i,disk_brk))
			fprintf(fp_ss,show_rrd ? "\nrrdtool update diskxfer%s.rrd %s" : "\nDISKXFER%s,%s", dskgrp(disk_brk,i),LOOP(loop,show_rrd));
		disk_brk->disk_xfers = DKDELTA(dk_xfers);
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
				disk_brk->disk_xfers / elapsed);
	}
	for (i = 0; i < disk_brk->disks; i++) {
		if(NEWDISKGROUP(i,disk_brk))
			fprintf(fp_ss,show_rrd ? "\nrrdtool update diskbsize%s.rrd %s" : "\nDISKBSIZE%s,%s", dskgrp(disk_brk,i),LOOP(loop,show_rrd));
		disk_brk->disk_xfers = DKDELTA(dk_xfers);
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
				(disk_brk->disk_xfers) == 0.0 ? 0.0 : (DKDELTA(dk_rkb) + DKDELTA(dk_wkb)) / (disk_brk->disk_xfers));
	}

	if( disk_brk->extended_disk == 1 && disk_brk->disk_mode == DISK_MODE_DISKSTATS )        {
		for (i = 0; i < disk_brk->disks; i++) {
			if(NEWDISKGROUP(i,disk_brk))    {
				fprintf(fp_ss,"\nDISKREADS%s,%s", dskgrp(disk_brk,i), LOOP(loop,show_rrd));
			}
			disk_brk->disk_read = DKDELTA(dk_reads);
			fprintf(fp_ss,",%.1f", disk_brk->disk_read / elapsed);
		}

		for (i = 0; i < disk_brk->disks; i++) {
			if(NEWDISKGROUP(i,disk_brk))    {
				fprintf(fp_ss,"\nDISKWRITES%s,%s", dskgrp(disk_brk,i), LOOP(loop,show_rrd));
			}
			disk_brk->disk_write = DKDELTA(dk_writes);
			fprintf(fp_ss,",%.1f", disk_brk->disk_write / elapsed);
		}
	}
	fprintf(fp_ss,"\n");
}

void collect_lpar(struct lparcfg_brk * lparcfg_brk, struct cpuinfo_brk * cpuinfo_brk)
{
	if (lparcfg_brk == NULL)
		return;

	int ret = lparcfg_brk->result;
	int loop = lparcfg_brk->ext->loop;
	int show_rrd = lparcfg_brk->ext->show_rrd;
	double elapsed = lparcfg_brk->ext->elapsed;
	FILE * fp_ss = lparcfg_brk->ext->fp_ss;
	/* Only print LPAR info to spreadsheet if in shared processor mode */
	if(ret != 0 && lparcfg_brk->lparcfg.shared_processor_mode > 0 && cpuinfo_brk->power_vm_type == VM_POWERVM )
		fprintf(fp_ss,"LPAR,%s,%9.6f,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%d\n",
				LOOP(loop,show_rrd),
				(double)lparcfg_brk->lparcfg.purr_diff/(double)lparcfg_brk->lparcfg.timebase/elapsed,
				lparcfg_brk->lparcfg.capped,
				lparcfg_brk->lparcfg.shared_processor_mode,
				lparcfg_brk->lparcfg.system_potential_processors,
				lparcfg_brk->lparcfg.system_active_processors,
				lparcfg_brk->lparcfg.pool_capacity,
				lparcfg_brk->lparcfg.MinEntCap/100.0,
				lparcfg_brk->lparcfg.partition_entitled_capacity/100.0,
				lparcfg_brk->lparcfg.partition_max_entitled_capacity/100.0,
				lparcfg_brk->lparcfg.MinProcs,
				cpuinfo_brk->cpus,              /* report logical CPU here so analyser graph CPU% vs VPs reports correctly */
				lparcfg_brk->lparcfg.partition_active_processors,
				lparcfg_brk->lparcfg.partition_potential_processors,
				lparcfg_brk->lparcfg.capacity_weight,
				lparcfg_brk->lparcfg.unallocated_capacity_weight,
				lparcfg_brk->lparcfg.BoundThrds,
				lparcfg_brk->lparcfg.MinMem,
				lparcfg_brk->lparcfg.unallocated_capacity,
				(double)lparcfg_brk->lparcfg.pool_idle_diff/(double)lparcfg_brk->lparcfg.timebase/elapsed,
				lparcfg_brk->lparcfg.smt_mode);
}

void collect_large(struct large_brk * large_brk)
{
	if (large_brk == NULL)
		return;

	struct data * p = large_brk->ext->p;
	int show_rrd = large_brk->ext->show_rrd;
	int loop = large_brk->ext->loop;
	FILE * fp_ss = large_brk->ext->fp_ss;

	if(p->mem.hugetotal > 0) {
		if(large_brk->first_huge == 1){
			large_brk->first_huge = 0;
			fprintf(fp_ss,"HUGEPAGES,Huge Page Use %s,HugeTotal,HugeFree,HugeSizeMB\n", large_brk->ext->run_name);
		}
		fprintf(fp_ss,"HUGEPAGES,%s,%ld,%ld,%.1f\n",
				LOOP(loop, show_rrd),
				p->mem.hugetotal,
				p->mem.hugefree,
				p->mem.hugesize/1024.0);
	}
}

void collect_jfs(struct jfs_brk * jfs_brk)
{
	if (jfs_brk == NULL)
		return;

	int loop = jfs_brk->ext->loop;
	int show_rrd = jfs_brk->ext->show_rrd;
	FILE * fp_ss = jfs_brk->ext->fp_ss;
	float fs_size;
	float fs_bsize;
	float fs_free;
	float fs_size_used;
	int k;

	jfs_load(jfs_brk, LOAD);
	fprintf(fp_ss,show_rrd ? "rrdtool update jfsfile.rrd %s" : "JFSFILE,%s", LOOP(loop,show_rrd));
	for (k = 0; k < jfs_brk->jfses; k++) {
		if(jfs_brk->jfs[k].mounted && strncmp(jfs_brk->jfs[k].name,"/proc",5)
				&& strncmp(jfs_brk->jfs[k].name,"/sys",4)
				&& strncmp(jfs_brk->jfs[k].name,"/dev/",5)
				&& strncmp(jfs_brk->jfs[k].name,"/run/",5)
				&& strncmp(jfs_brk->jfs[k].name,"/var/lib/nfs/rpc",16)
		  )   { /* /proc gives invalid/insane values */
			if(fstatfs( jfs_brk->jfs[k].fd, &jfs_brk->statfs_buffer) != -1) {
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

				if(fs_size <= 0.0 || fs_bsize <= 0.0) /* some pseudo filesystems have zero size but we get a -nan result */
					fs_size_used = 0.0;
				else
					fs_size_used = (fs_size - (float)jfs_brk->statfs_buffer.f_bfree  * fs_bsize/1024.0/1024.0)/fs_size * 100.0;

				if (fs_size_used >100.0)
					fs_size_used = 100.0;

				fprintf(fp_ss, show_rrd ? ":%.1f" : ",%.1f", fs_size_used );
			}
			else
				fprintf(fp_ss, show_rrd? ":U" : ",0.0");
		}
	}
	fprintf(fp_ss, "\n");
	jfs_load(jfs_brk, UNLOAD);
}

void collect_net(struct net_brk * net_brk)
{
	if (net_brk == NULL)
		return;

	struct data * q = net_brk->ext->q;
	struct data * p = net_brk->ext->p;
	FILE * fp_ss = net_brk->ext->fp_ss;
	int show_rrd = net_brk->ext->show_rrd;
	int i;
	int loop = net_brk->ext->loop;
	double elapsed = net_brk->ext->elapsed;

#define IFDELTA(member) ((float)( (q->ifnets[i].member > p->ifnets[i].member) ? 0 : (p->ifnets[i].member - q->ifnets[i].member)/elapsed) )
#define IFDELTA_ZERO(member1,member2) ((IFDELTA(member1) == 0) || (IFDELTA(member2)== 0)? 0.0 : IFDELTA(member1)/IFDELTA(member2) )
	fprintf(fp_ss,show_rrd ? "rrdtool update net.rrd %s" : "NET,%s", LOOP(loop,show_rrd));
	/*for (i = 0; i < net_brk->networks; i++) {
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f", IFDELTA(if_ibytes) / 1024.0);
	}
	for (i = 0; i < net_brk->networks; i++) {
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f", IFDELTA(if_obytes) / 1024.0);
	}*/
	for (i = 0; i < net_brk->networks; i++) {
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f", IFDELTA(if_ibits) / 1024.0);
	}
	for (i = 0; i < net_brk->networks; i++) {
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f", IFDELTA(if_obits) / 1024.0);
	}
	fprintf(fp_ss,"\n");
	fprintf(fp_ss,show_rrd ? "rrdtool update netpacket.rrd %s" : "NETPACKET,%s", LOOP(loop,show_rrd));
	for (i = 0; i < net_brk->networks; i++) {
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f", IFDELTA(if_ipackets) );
	}
	for (i = 0; i < net_brk->networks; i++) {
		fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f", IFDELTA(if_opackets) );
	}
	fprintf(fp_ss,"\n");
}

void collect_nfs(struct nfs_brk * nfs_brk)
{
	if (nfs_brk == NULL)
		return;

	int show_rrd = nfs_brk->ext->show_rrd;
	int loop = nfs_brk->ext->loop;
	double elapsed = nfs_brk->ext->elapsed;
	int i;
	FILE * fp_ss = nfs_brk->ext->fp_ss;
	struct data * p = nfs_brk->ext->p;
	struct data * q = nfs_brk->ext->q;

	if(nfs_brk->nfs_first_time && ! show_rrd) {
		if(nfs_brk->nfs_v2c_found) {
			fprintf(fp_ss,"NFSCLIV2,NFS Client v2");
			for(i=0;i<18;i++)
				fprintf(fp_ss,",%s",nfs_brk->nfs_v2_names[i]);
			fprintf(fp_ss,"\n");
		}
		if(nfs_brk->nfs_v2s_found) {
			fprintf(fp_ss,"NFSSVRV2,NFS Server v2");
			for(i=0;i<18;i++)
				fprintf(fp_ss,",%s",nfs_brk->nfs_v2_names[i]);
			fprintf(fp_ss,"\n");
		}

		if(nfs_brk->nfs_v3c_found) {
			fprintf(fp_ss,"NFSCLIV3,NFS Client v3");
			for(i=0;i<22;i++)
				fprintf(fp_ss,",%s",nfs_brk->nfs_v3_names[i]);
			fprintf(fp_ss,"\n");
		}
		if(nfs_brk->nfs_v3s_found) {
			fprintf(fp_ss,"NFSSVRV3,NFS Server v3");
			for(i=0;i<22;i++)
				fprintf(fp_ss,",%s",nfs_brk->nfs_v3_names[i]);
			fprintf(fp_ss,"\n");
		}

		if(nfs_brk->nfs_v4c_found) {
			fprintf(fp_ss,"NFSCLIV4,NFS Client v4");
			for(i=0;i<nfs_brk->nfs_v4c_names_count;i++)
				fprintf(fp_ss,",%s",nfs_brk->nfs_v4c_names[i]);
			fprintf(fp_ss,"\n");
		}
		if(nfs_brk->nfs_v4s_found) {
			fprintf(fp_ss,"NFSSVRV4,NFS Server v4");
			for(i=0;i<nfs_brk->nfs_v4s_names_count;i++)
				fprintf(fp_ss,",%s",nfs_brk->nfs_v4s_names[i]);
			fprintf(fp_ss,"\n");
		}
		memcpy(&q->nfs,&p->nfs,sizeof(struct nfs_stat));
		nfs_brk->nfs_first_time=0;
	}
	if(nfs_brk->nfs_v2c_found) {
		fprintf(fp_ss,show_rrd ? "rrdtool update nfscliv2.rrd %s" : "NFSCLIV2,%s", LOOP(loop,show_rrd));
		for(i=0;i<18;i++) {
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					(double)NFS_DELTA(nfs.v2c[i]));
		}
		fprintf(fp_ss,"\n");
	}
	if(nfs_brk->nfs_v2s_found) {
		fprintf(fp_ss,show_rrd ? "rrdtool update nfsvrv2.rrd %s" : "NFSSVRV2,%s", LOOP(loop,show_rrd));
		for(i=0;i<18;i++) {
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					(double)NFS_DELTA(nfs.v2s[i]));
		}
		fprintf(fp_ss,"\n");
	}
	if(nfs_brk->nfs_v3c_found) {
		fprintf(fp_ss,show_rrd ? "rrdtool update nfscliv3.rrd %s" : "NFSCLIV3,%s", LOOP(loop,show_rrd));
		for(i=0;i<22;i++) {
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					(double)NFS_DELTA(nfs.v3c[i]));
		}
		fprintf(fp_ss,"\n");
	}
	if(nfs_brk->nfs_v3s_found) {
		fprintf(fp_ss,show_rrd ? "rrdtool update nfsvrv3.rrd %s" : "NFSSVRV3,%s", LOOP(loop,show_rrd));
		for(i=0;i<22;i++) {
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					(double)NFS_DELTA(nfs.v3s[i]));
		}
		fprintf(fp_ss,"\n");
	}

	if(nfs_brk->nfs_v4c_found) {
		fprintf(fp_ss,show_rrd ? "rrdtool update nfscliv4.rrd %s" : "NFSCLIV4,%s", LOOP(loop,show_rrd));
		for(i=0;i<nfs_brk->nfs_v4c_names_count;i++) {
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					(double)NFS_DELTA(nfs.v4c[i]));
		}
		fprintf(fp_ss,"\n");
	}
	if(nfs_brk->nfs_v4s_found) {
		fprintf(fp_ss,show_rrd ? "rrdtool update nfsvrv4.rrd %s" : "NFSSVRV4,%s", LOOP(loop,show_rrd));
		for(i=0;i<nfs_brk->nfs_v4c_names_count;i++) {
			fprintf(fp_ss,show_rrd ? ":%.1f" : ",%.1f",
					(double)NFS_DELTA(nfs.v4s[i]));
		}
		fprintf(fp_ss,"\n");
	}
}

void collect_kernel(struct kernel_brk * kernel_brk)
{
	if (kernel_brk == NULL)
		return;

	int loop = kernel_brk->ext->loop;
	int show_rrd = kernel_brk->ext->show_rrd;
	struct data * p = kernel_brk->ext->p;
	struct data * q = kernel_brk->ext->q;
	double elapsed = kernel_brk->ext->elapsed;
	FILE * fp_ss = kernel_brk->ext->fp_ss;
	char * str_p;

	if(kernel_brk->ext->proc_first_time) {
		q->cpu_total.ctxt = p->cpu_total.ctxt;
		q->cpu_total.procs= p->cpu_total.procs;
		kernel_brk->ext->proc_first_time=0;
	}
	if(show_rrd)
		str_p = "rrdtool update proc.rrd %s:%.0f:%.0f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f\n";
	else
		str_p = "PROC,%s,%.0f,%.0f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n";

	fprintf(fp_ss,str_p, LOOP(loop,show_rrd), (float)p->cpu_total.running, (float)p->cpu_total.blocked, (float)(p->cpu_total.ctxt - q->cpu_total.ctxt)/elapsed, -1.0, -1.0, -1.0, (float)(p->cpu_total.procs - q->cpu_total.procs)/elapsed, -1.0, -1.0, -1.0);

}

void collect_vm (struct mem_brk * mem_brk)
{
	if (mem_brk == NULL)
		return;

	int ret = mem_brk->result;
	struct data * p = mem_brk->ext->p;
	struct data * q = mem_brk->ext->q;
	FILE * fp_ss = mem_brk->ext->fp_ss;
	char * str_p;
	int show_rrd = mem_brk->ext->show_rrd;
	int loop = mem_brk->ext->loop;

	if( ret < 0) {
		mem_brk->show_vm=0;
	} else if(mem_brk->vm_first_time) {
		mem_brk->vm_first_time = 0;
		fprintf(fp_ss,"VM,Paging and Virtual Memory,nr_dirty,nr_writeback,nr_unstable,nr_page_table_pages,nr_mapped,nr_slab,pgpgin,pgpgout,pswpin,pswpout,pgfree,pgactivate,pgdeactivate,pgfault,pgmajfault,pginodesteal,slabs_scanned,kswapd_steal,kswapd_inodesteal,pageoutrun,allocstall,pgrotated,pgalloc_high,pgalloc_normal,pgalloc_dma,pgrefill_high,pgrefill_normal,pgrefill_dma,pgsteal_high,pgsteal_normal,pgsteal_dma,pgscan_kswapd_high,pgscan_kswapd_normal,pgscan_kswapd_dma,pgscan_direct_high,pgscan_direct_normal,pgscan_direct_dma\n");
	}
	if(show_rrd)
		str_p = "rrdtool update vm.rrd %s"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld:%lld:%lld:%lld"
			":%lld:%lld\n";
	else
		str_p = "VM,%s"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld,%lld,%lld,%lld"
			",%lld,%lld\n";

	fprintf(fp_ss, str_p,
			LOOP(loop,show_rrd),
			VMCOUNT(nr_dirty),
			VMCOUNT(nr_writeback),
			VMCOUNT(nr_unstable),
			VMCOUNT(nr_page_table_pages),
			VMCOUNT(nr_mapped),
			VMCOUNT(nr_slab),
			VMDELTA(pgpgin),
			VMDELTA(pgpgout),
			VMDELTA(pswpin),
			VMDELTA(pswpout),
			VMDELTA(pgfree),
			VMDELTA(pgactivate),
			VMDELTA(pgdeactivate),
			VMDELTA(pgfault),
			VMDELTA(pgmajfault),
			VMDELTA(pginodesteal),
			VMDELTA(slabs_scanned),
			VMDELTA(kswapd_steal),
			VMDELTA(kswapd_inodesteal),
			VMDELTA(pageoutrun),
			VMDELTA(allocstall),
			VMDELTA(pgrotated),
			VMDELTA(pgalloc_high),
			VMDELTA(pgalloc_normal),
			VMDELTA(pgalloc_dma),
			VMDELTA(pgrefill_high),
			VMDELTA(pgrefill_normal),
			VMDELTA(pgrefill_dma),
			VMDELTA(pgsteal_high),
			VMDELTA(pgsteal_normal),
			VMDELTA(pgsteal_dma),
			VMDELTA(pgscan_kswapd_high),
			VMDELTA(pgscan_kswapd_normal),
			VMDELTA(pgscan_kswapd_dma),
			VMDELTA(pgscan_direct_high),
			VMDELTA(pgscan_direct_normal),
			VMDELTA(pgscan_direct_dma));
}


void collect_dgroup(struct disk_brk * disk_brk)
{
	if (disk_brk == NULL)
		return;

	int k,j,i;
	double elapsed = disk_brk->ext->elapsed;
	FILE* fp_ss = disk_brk->ext->fp_ss;
	int loop = disk_brk->ext->loop;
	int show_rrd = disk_brk->ext->show_rrd;
	struct data * p = disk_brk->ext->p;
	struct data * q = disk_brk->ext->q;

	if (disk_brk->dgroup_loaded == 2) {
		fprintf(fp_ss, show_rrd ? "rrdtool update dgbusy.rdd %s" : "DGBUSY,%s", LOOP(loop,show_rrd));
		for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
			if (disk_brk->dgroup_name[k] != 0) {
				disk_brk->disk_total = 0.0;
				for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
					i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
					if (i != -1) {
						disk_brk->disk_total += DKDELTA(dk_time) / elapsed;
					}
				}
				fprintf(fp_ss, show_rrd ? ":%.1f" : ",%.1f", (float)(disk_brk->disk_total / disk_brk->dgroup_disks[k]));
			}
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, show_rrd ? "rrdtool update dgread.rdd %s" : "DGREAD,%s", LOOP(loop,show_rrd));
		for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
			if (disk_brk->dgroup_name[k] != 0) {
				disk_brk->disk_total = 0.0;
				for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
					i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
					if (i != -1) {
						disk_brk->disk_total += DKDELTA(dk_rkb);
					}
				}
				fprintf(fp_ss, show_rrd ? ":%.1f" : ",%.1f", disk_brk->disk_total / elapsed);
			}
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, show_rrd ? "rrdtool update dgwrite.rdd %s" : "DGWRITE,%s", LOOP(loop,show_rrd));
		for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
			if (disk_brk->dgroup_name[k] != 0) {
				disk_brk->disk_total = 0.0;
				for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
					i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
					if (i != -1) {
						/*
						   disk_brk->disk_total += DKDELTA(dk_writes) * p->dk[i].dk_bsize / 1024.0;
						   */
						disk_brk->disk_total += DKDELTA(dk_wkb);
					}
				}
				fprintf(fp_ss, show_rrd ? ":%.1f" : ",%.1f", disk_brk->disk_total / elapsed);
			}
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, show_rrd ? "rrdtool update dgbsize.rdd %s" : "DGSIZE,%s", LOOP(loop,show_rrd));
		for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
			if (disk_brk->dgroup_name[k] != 0) {
				disk_brk->disk_write = 0.0;
				disk_brk->disk_xfers  = 0.0;
				for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
					i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
					if (i != -1) {
						/*
						   disk_brk->disk_write += (DKDELTA(dk_reads) + DKDELTA(dk_writes) ) * p->dk[i].dk_bsize / 1024.0;
						   */
						disk_brk->disk_write += (DKDELTA(dk_rkb) + DKDELTA(dk_wkb) );
						disk_brk->disk_xfers  += DKDELTA(dk_xfers);
					}
				}
				if ( disk_brk->disk_write == 0.0 || disk_brk->disk_xfers == 0.0)
					disk_brk->disk_size = 0.0;
				else
					disk_brk->disk_size = disk_brk->disk_write / disk_brk->disk_xfers;
				fprintf(fp_ss, show_rrd ? ":%.1f" : ",%.1f", disk_brk->disk_size);
			}
		}
		fprintf(fp_ss, "\n");
		fprintf(fp_ss, show_rrd ? "rrdtool update dgxfer.rdd %s" : "DGXFER,%s", LOOP(loop,show_rrd));
		for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
			if (disk_brk->dgroup_name[k] != 0) {
				disk_brk->disk_total = 0.0;
				for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
					i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
					if (i != -1) {
						disk_brk->disk_total  += DKDELTA(dk_xfers);
					}
				}
				fprintf(fp_ss, show_rrd ? ":%.1f" : ",%.1f", disk_brk->disk_total / elapsed);
			}
		}
		fprintf(fp_ss, "\n");

		if( disk_brk->extended_disk == 1 && disk_brk->disk_mode == DISK_MODE_DISKSTATS )        {
			fprintf(fp_ss,"DGREADS,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_reads);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total / elapsed);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGREADMERGE,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_rmerge);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total / elapsed);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGREADSERV,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_rmsec);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGWRITES,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_writes);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total / elapsed);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGWRITEMERGE,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_wmerge);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total / elapsed);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGWRITESERV,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_wmsec);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGINFLIGHT,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += p->dk[i].dk_inflight;
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGIOTIME,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_time);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total);
				}
			}
			fprintf(fp_ss, "\n");
			fprintf(fp_ss,"DGBACKLOG,%s", LOOP(loop,show_rrd));
			for (k = 0; k < disk_brk->dgroup_total_groups; k++) {
				if (disk_brk->dgroup_name[k] != 0) {
					disk_brk->disk_total = 0.0;
					for (j = 0; j < disk_brk->dgroup_disks[k]; j++) {
						i = disk_brk->dgroup_data[k*DGROUPITEMS+j];
						if (i != -1) {
							disk_brk->disk_total  += DKDELTA(dk_backlog);
						}
					}
					fprintf(fp_ss,",%.1f", disk_brk->disk_total);
				}
			}
			fprintf(fp_ss, "\n");
		} /* if( disk_brk->extended_disk == 1 && disk_mode == DISK_MODE_DISKSTATS */
	}       /* if (disk_brk->dgroup_loaded == 2) */
}

void collect_smp (struct smp_brk * smp_brk, struct cpuinfo_brk * cpuinfo_brk)
{
	if (smp_brk == NULL || cpuinfo_brk == NULL)
		return;

	FILE * fp_ss = smp_brk->ext->fp_ss;
	int loop = smp_brk->ext->loop;
	int show_rrd = smp_brk->ext->show_rrd;
	struct data * p = smp_brk->ext->p;
	struct data * q = smp_brk->ext->q;
	int i;

	for (i = 0; i < cpuinfo_brk->cpus; i++) {
		cpuinfo_brk->cpu_user = p->cpuN[i].user - q->cpuN[i].user;
		cpuinfo_brk->cpu_sys  = p->cpuN[i].sys  - q->cpuN[i].sys;
		cpuinfo_brk->cpu_wait = p->cpuN[i].wait - q->cpuN[i].wait;
		cpuinfo_brk->cpu_idle = p->cpuN[i].idle - q->cpuN[i].idle;
		cpuinfo_brk->cpu_steal= p->cpuN[i].steal- q->cpuN[i].steal;
		cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle + cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys + cpuinfo_brk->cpu_wait + cpuinfo_brk->cpu_steal;
		if (cpuinfo_brk->cpu_sum == 0)
			cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle = 100.0;
		if(!smp_brk->smp_first_time) {
			if(!smp_brk->show_raw) {
				plot_smp_save(smp_brk, cpuinfo_brk, i + 1, 3 + i,
						(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_steal/ (double)cpuinfo_brk->cpu_sum * 100.0);
			}
			else {
				save_smp_save(smp_brk, i + 1, 4 + i,
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
		smp_brk->smp_first_time = 0;
	}
	RRD	fprintf(fp_ss,"rrdtool update cpu%02d.rrd %s:%.1f:%.1f:%.1f:%.1f\n",i,LOOP(loop,show_rrd),
			(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0);

	cpuinfo_brk->cpu_user = p->cpu_total.user - q->cpu_total.user;
	cpuinfo_brk->cpu_sys  = p->cpu_total.sys  - q->cpu_total.sys;
	cpuinfo_brk->cpu_wait = p->cpu_total.wait - q->cpu_total.wait;
	cpuinfo_brk->cpu_idle = p->cpu_total.idle - q->cpu_total.idle;
	cpuinfo_brk->cpu_steal= p->cpu_total.steal- q->cpu_total.steal;
	cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle + cpuinfo_brk->cpu_user + cpuinfo_brk->cpu_sys + cpuinfo_brk->cpu_wait + cpuinfo_brk->cpu_steal;
	if(cpuinfo_brk->cpu_sum == 0)
		cpuinfo_brk->cpu_sum = cpuinfo_brk->cpu_idle = 100.0;

	RRD fprintf(fp_ss,"rrdtool update cpu.rrd %s:%.1f:%.1f:%.1f:%.1f%.1f\n", LOOP(loop,show_rrd),
			(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0,
			(double)cpuinfo_brk->cpu_steal/ (double)cpuinfo_brk->cpu_sum * 100.0);
	if (cpuinfo_brk->cpus > 1) {
		if(!smp_brk->smp_first_time) {
			if(!smp_brk->show_raw) {
				plot_smp_save(smp_brk, cpuinfo_brk, 0, 4 + i,
						(double)cpuinfo_brk->cpu_user / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_sys  / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_wait / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_idle / (double)cpuinfo_brk->cpu_sum * 100.0,
						(double)cpuinfo_brk->cpu_steal/ (double)cpuinfo_brk->cpu_sum * 100.0);
			} else {
				save_smp_save(smp_brk, 0, 4 + i,
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
	}
}

void collect_top_info (struct top_brk * top_brk, unsigned long pagesize, double ignore_procdisk_threshold)
{
	if (top_brk == NULL)
		return;

	FILE * fp_ss = top_brk->ext->fp_ss;
	int j = 0;
	int i = 0;
	struct data * p = top_brk->ext->p;
	struct data * q = top_brk->ext->q;
	int loop = top_brk->ext->loop;
	int show_rrd = top_brk->ext->show_rrd;
	double elapsed = top_brk->ext->elapsed;

#define COUNTDELTA(brk,member) ( (q->procs[brk->topper[j].other].member > p->procs[i].member) ? 0 : (p->procs[i].member  - q->procs[brk->topper[j].other].member) )
	for (j = 0; j < top_brk->max_sorted; j++) {
		i = top_brk->topper[j].index;
		if((top_brk->cmdfound && cmdcheck(p->procs[i].pi_comm)) ||
				(!top_brk->cmdfound && ((top_brk->topper[j].time / elapsed) > ignore_procdisk_threshold)) )
		{
#ifndef KERNEL_2_6_18
			fprintf(fp_ss,"TOP,%07d,%s,%.2f,%.2f,%.2f,%lu,%lu,%lu,%lu,%lu,%d,%d,%s\n",
#else
					fprintf(fp_ss,"TOP,%07d,%s,%.2f,%.2f,%.2f,%lu,%lu,%lu,%lu,%lu,%d,%d,%s,%ld,%llu\n",
#endif
						/* 1 */ p->procs[i].pi_pid,
						/* 2 */ LOOP(loop,show_rrd),
						/* 3 */ top_brk->topper[j].time / elapsed,
						/* 4 */ TIMEDELTA(pi_utime,i,top_brk->topper[j].other) / elapsed,
						/* 5 */ TIMEDELTA(pi_stime,i,top_brk->topper[j].other) / elapsed,
						/* 6 */ p->procs[i].statm_size*pagesize/1024UL, /* in KB */
						/* 7 */ p->procs[i].statm_resident*pagesize/1024UL, /* in KB */
						/* 8 */ p->procs[i].statm_trs*pagesize/1024UL, /* in KB */
						/* 9 */ p->procs[i].statm_drs*pagesize/1024UL, /* in KB */
						/* 10*/ p->procs[i].statm_share*pagesize/1024UL, /* in KB */
						/* 11*/ (int)(COUNTDELTA(top_brk,pi_minflt) / elapsed),
						/* 12*/ (int)(COUNTDELTA(top_brk,pi_majflt) / elapsed),
						/* 13*/ p->procs[i].pi_comm

#ifndef KERNEL_2_6_18
						);
#else
			,
				p->procs[i].pi_num_threads,
				COUNTDELTA(top_brk,pi_delayacct_blkio_ticks)
					);
#endif

			if(top_brk->show_args)
				args_output(p->procs[i].pi_pid,loop, p->procs[i].pi_comm);
		} else
			top_brk->skipped++;
	}
}

void collect_mem (struct mem_brk * mem_brk, struct lparcfg_brk * lparcfg_brk)
{
	char * str_p;
	int loop = mem_brk->ext->loop;
	struct data * p = mem_brk->ext->p;
	FILE * fp_ss = mem_brk->ext->fp_ss;
	int show_rrd = mem_brk->ext->show_rrd;
	double elapsed = mem_brk->ext->elapsed;

	if(show_rrd)
		str_p = "rrdtool update mem.rrd %s:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f:%.1f\n";
	else
		str_p = "MEM,%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n";
	fprintf(fp_ss,str_p,
			LOOP(loop,show_rrd),
			p->mem.memtotal/1024.0,
			p->mem.hightotal/1024.0,
			p->mem.lowtotal/1024.0,
			p->mem.swaptotal/1024.0,
			p->mem.memfree/1024.0,
			p->mem.highfree/1024.0,
			p->mem.lowfree/1024.0,
			p->mem.swapfree/1024.0,
			p->mem.memshared/1024.0,
			p->mem.cached/1024.0,
			p->mem.active/1024.0,
#ifdef LARGEMEM
			-1.0,
#else
			p->mem.bigfree/1024.0,
#endif /*LARGEMEM*/
			p->mem.buffers/1024.0,
			p->mem.swapcached/1024.0,
			p->mem.inactive/1024.0);
#ifdef POWER
	if(lparcfg_brk->lparcfg.cmo_enabled != 0) {
		if(!show_rrd)
			fprintf(fp_ss,"MEMAMS,%s,%d,%d,%.1f,%.3lf,0,0,0,%.1f,%ld,%ld,%ld\n",
					LOOP(loop,show_rrd),
					(int)lparcfg_brk->lparcfg.entitled_memory_pool_number,
					(int)lparcfg_brk->lparcfg.entitled_memory_weight,
					(float)(lparcfg_brk->lparcfg.cmo_faults_diff)/elapsed,
					(float)(lparcfg_brk->lparcfg.cmo_fault_time_usec_diff)/1000/1000/elapsed,
					/* three zeros here */
					(float)(lparcfg_brk->lparcfg.backing_memory)/1024/1024,
					lparcfg_brk->lparcfg.cmo_page_size/1024,
					lparcfg_brk->lparcfg.entitled_memory_pool_size/1024/1024,
					lparcfg_brk->lparcfg.entitled_memory_loan_request/1024);
	}
#ifdef EXPERIMENTAL
	if(!show_rrd)fprintf(fp_ss,"MEMEXPERIMENTAL,%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",
			LOOP(loop,show_rrd),
			(long)lparcfg_brk->lparcfg.DesEntCap,
			(long)lparcfg_brk->lparcfg.DesProcs,
			(long)lparcfg_brk->lparcfg.DesVarCapWt,
			(long)lparcfg_brk->lparcfg.DedDonMode,
			(long)lparcfg_brk->lparcfg.group,
			(long)lparcfg_brk->lparcfg.pool,
			(long)lparcfg_brk->lparcfg.entitled_memory,
			(long)lparcfg_brk->lparcfg.entitled_memory_group_number,
			(long)lparcfg_brk->lparcfg.unallocated_entitled_memory_weight,
			(long)lparcfg_brk->lparcfg.unallocated_io_mapping_entitlement);
#endif /* EXPERIMENTAL */
#endif /* POWER */
}

void collect_verbose (struct cpuinfo_brk * cpuinfo_brk)
{
	if (cpuinfo_brk == NULL)
		return;

	int i = 0;
	struct global_data * g_data = cpuinfo_brk->ext;

	if(cpuinfo_brk->cpus > cpuinfo_brk->max_cpus) {
		for (i = cpuinfo_brk->max_cpus+1; i <= cpuinfo_brk->cpus; i++)
			fprintf(g_data->fp_ss,"CPU%03d,CPU %d %s,User%%,Sys%%,Wait%%,Idle%%\n", i, i, g_data->run_name);
		cpuinfo_brk->max_cpus= cpuinfo_brk->cpus;
	}

	if( g_data->bbbr_line == 0)     {
		fprintf(g_data->fp_ss,"BBBR,0,Reconfig,action,old,new\n");
		g_data->bbbr_line++;
	}
	fprintf(g_data->fp_ss,"BBBR,%03d,%s,cpuchg,%d,%d\n",g_data->bbbr_line++,LOOP(g_data->loop,g_data->show_rrd),cpuinfo_brk->old_cpus,cpuinfo_brk->cpus);
}
