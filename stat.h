/*************************************************************************
    > File Name: stat.h
    > Author: ToLiMit
    > Mail: 348958453@qq.com
    > Created Time: Wed 28 Oct 2015 04:39:41 PM CST
 ************************************************************************/

#ifndef __STAT_H
#define __STAT_H

#define DEFAULT_LOG_DIR "/var/log/nmon/"
#define DISK_MODE_IO 1
#define DISK_MODE_DISKSTATS 2
#define DISK_MODE_PARTITIONS 3

#define P_CPUINFO       0
#define P_STAT          1
#define P_VERSION       2
#define P_MEMINFO       3
#define P_UPTIME        4
#define P_LOADAVG       5
#define P_NFS           6
#define P_NFSD          7
#define P_VMSTAT        8 /* new in 13h */
#define P_NUMBER        9 /* one more than the max */

#define DISKMIN 256
#define PROC_MAXLINES (16*1024) /*MAGIC COOKIE WARNING */

#define DPL 150 /* Disks per line for file output to ensure it
                                   does not overflow the spreadsheet input line max */

#define SHOW_DISK_NONE  0
#define SHOW_DISK_STATS 1
#define SHOW_DISK_GRAPH 2

#define CMDMAX 64

#define ARGS_NONE 0
#define ARGS_ONLY 1

#define IFNAME 64

#define DGROUPS 64
#define DGROUPITEMS 512

#define WARNING "needs root permission or file not present"

#ifdef POWER

#define VM_UNKNOWN 0
#define VM_POWERVM 1
#define VM_POWERKVM_GUEST 2
#define VM_POWERKVM_HOST 3
#define VM_NATIVE 4
#define NUMBER_NOT_VALID -999

#endif
#define ARGSMAX 1024*8
#define CMDLEN 4096

#define JFSMAX 128
#define LOAD 1
#define UNLOAD 0
#define JFSNAMELEN 64
#define JFSTYPELEN 8

#define VERSION "15g"

#define CPUMAX (192 * 8) /* MAGIC COOKIE WARNING */
#define NFS_V2_NAMES_COUNT 18
#define NFS_V3_NAMES_COUNT 22
#define NFS_V4S_NAMES_COUNT 72
#define NFS_V4C_NAMES_COUNT 60

#define MAX_SNAPS 72
#define MAX_SNAP_ROWS 20
#define SNAP_OFFSET 6

#define CHLD_START 0
#define CHLD_SNAP 1
#define CHLD_END 2

#define LPAR_LINE_MAX   50
#define LPAR_LINE_WIDTH 80

#define MAXROWS 1024
#define MAXCOLS 150 /* changed to allow maximum column widths */

#define BUFFER_ROW 1024

#define SHOW (1)
#define NOTSHOW (0)

#endif
