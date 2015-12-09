/*
 * lmon.c -- Curses based Performance Monitor for Linux
 * Developer: Nigel Griffiths.
 */

/*
 * Use the following Makefile (for Linux on POWER)
 CFLAGS=-g -D JFS -D GETUSER -Wall -D LARGEMEM -D POWER
 LDFLAGS=-lcurses
nmon: lnmon.o
 * end of Makefile
 */
/* #define POWER 1 */
/* #define KERNEL_2_6_18 1 */
/* This adds the following to the disk stats
   pi_num_threads,
   pi_rt_priority,
   pi_policy,
   pi_delayacct_blkio_ticks
   */

#include "lmon.h"
#include "display.h"
#include "proc.h"
#include "func.h"
#include "collect.h"
#include "stat.h"
#include "struct.h"

char version[] = VERSION;
static char *SccsId = "nmon " VERSION;
pthread_t collect_tid;
sem_t sem;

struct global_data g_data = {
	.pad = NULL,
	.q = NULL,
	.p = NULL,
	.seconds  = -1,         /* pause interval */
	.maxloops = -1,  /* stop after this number of updates */
	.stat8 = 0,
	.show_aaa = 1,
	.show_all = 1,
	.x = 0,
	.y = 0,
	.user_filename = {'\0'},
	.user_filename_set = 0,
	.log_path = {'\0'},
	.proc_first_time = 1,
	.run_name_set = 0,
	.show_rrd = 0,
	.bbbr_line = 0,
	.cur_line = 0,
	.colour = 1,
	.update_data = 1,
	.change_show = 1,
};

struct help_brk help_brk = {
	.show_help = 0,
};

struct welcome_brk welcome_brk = {
	.welcome = 1,
};

struct disk_brk disk_brk = {
	.disk_mode = 0,
	.show_disk = 0,
	.show_diskmap = 0,
	.show_dgroup = 0,
	.extended_disk = 0,
	.disks_per_line = DPL,
	.disks = 0,
	.diskmax = DISKMIN,
	.dgroup_loaded = 0,
	.auto_dgroup = 0,
	.disk_busy_map_ch = "_____.....----------++++++++++oooooooooo0000000000OOOOOOOOOO8888888888XXXXXXXXXX##########@@@@@@@@@@*",
};

struct jfs_brk jfs_brk = {
	.show_jfs = 0,
	.jfses = 0,
};

struct cpuinfo_brk cpuinfo_brk = {
	.show_cpu = 0,
	.show_longterm = 0,
	.show_verbose = 0,
#ifdef POWER
	.endian = "Unknown Endian",
	.power_vm_type = VM_UNKNOWN,
#endif
	.easy = {"not found",0,0,0,0},
	.lsb_release = {"not found",0,0,0,0},
#ifdef X86
	.cores = 0,
	.siblings = 0,
	.processorchips = 0,
	.hyperthreads = 0,
	.vendor_ptr = "-",
	.model_ptr = "-",
	.mhz_ptr = "-",
	.bogo_ptr = "-",
#endif
	.old_cpus = 1,
	.max_cpus = 1,
	.next_cpu_snap = 0,
	.cpu_snap_all = 0,
	.dotline = 0,
};

#ifdef POWER
struct lparcfg_brk lparcfg_brk = {
	.show_lpar = 0,
	.lparcfg_processed = 0,
	.lparcfg_reread = 1,
	.lpar_sanity = 55,
	.lpar_count = 0,
	.lpar_first_time = 1,
	.result = 0,
};
#endif

struct top_brk top_brk = {
	.show_top = 0,
	.show_topmode = 3,
	.collect_top = 0,
	.collect_topmode = 3,
	.topper_size = 200,
	.max_sorted = 0,
	.top_first_time = 1,
	.show_args = 0,
	.topper = NULL,
	.cmdfound = 0,
	.show_count = 100,
	.cur_ps = 0,
	.cmdlist = {NULL},
};

struct mem_brk mem_brk = {
	.show_memory = 0,
	.show_vm = 0,
	.result = 0,
	.pagesize = 1024 * 4,
};

struct kernel_brk kernel_brk = {
	.show_kernel = 0,
};

struct large_brk large_brk = {
	.show_large = 0,
	.first_huge = 1,
	.huge_peak = 0,
};

struct net_brk net_brk = {
	.show_net = 0,
	.show_neterror = 0,
	.networks = 0,
	.errors = 0,
};

struct nfs_brk nfs_brk = {
	.show_nfs = 0,
	.nfs_v2c_found = 0,
	.nfs_v2s_found = 0,
	.nfs_v3c_found = 0,
	.nfs_v3s_found = 0,
	.nfs_v4c_found = 0,
	.nfs_v4s_found = 0,
	.nfs_clear = 0,
	.nfs_v4s_names_count = NFS_V4S_NAMES_COUNT,
	.nfs_v4c_names_count = NFS_V4C_NAMES_COUNT,
	.nfs_v4c_names = {  /* get these names from nfsstat as they are NOT documented */
		"null",         "read",         "write",        "commit",       "open",         "open_conf",    /* 1 - 6 */
		"open_noat",    "open_dgrd",    "close",        "setattr",      "fsinfo",       "renew",        /* 7 - 12 */
		"setclntid",    "confirm",      "lock",         "lockt",        "locku",        "access",       /* 13 - 18 */
		"getattr",      "lookup",       "lookup_root",  "remove",       "rename",       "link",         /* 19 - 24 */
		"symlink",      "create",       "pathconf",     "statfs",       "readlink",     "readdir",      /* 25 - 30 */
		"server_caps",  "delegreturn",  "getacl",       "setacl",       "fs_locations", "rel_lkowner",  /* 31 - 36 */
		"secinfo",      "exchange_id",  "create_ses",   "destroy_ses",  "sequence",     "get_lease_t",  /* 37 - 42 */
		"reclaim_comp", "layoutget",    "getdevinfo",   "layoutcommit", "layoutreturn", "getdevlist",   /* 43 - 48 */
		"stat49",       "stat50",       "stat51",       "stat52",       "start53",      "stat54",       /* 49 - 54 */
		"stat55",       "stat56",       "stat57",       "stat58",       "start59",      "stat60"        /* 55 - 60 */
	},

	.nfs_v4s_names = {  /* get these names from nfsstat as they are NOT documented */
		"op0-unused",   "op1-unused",   "op2-future",   "access",       "close",        "commit",       /* 1 - 6 */
		"create",       "delegpurge",   "delegreturn",  "getattr",      "getfh",        "link",         /* 7 - 12 */
		"lock",         "lockt",        "locku",        "lookup",       "lookup_root",  "nverify",      /* 13 - 18 */
		"open",         "openattr",     "open_conf",    "open_dgrd",    "putfh",        "putpubfh",     /* 19 - 24 */
		"putrootfh",    "read",         "readdir",      "readlink",     "remove",       "rename",       /* 25 - 30 */
		"renew",        "restorefh",    "savefh",       "secinfo",      "setattr",      "setcltid",     /* 31 - 36 */
		"setcltidconf", "verify",       "write",        "rellockowner", "bc_ctl",       "blind_conn",   /* 37 - 42 */
		"exchange_id",  "create_ses",   "destroy_ses",  "free_statid",  "getdirdelag",  "getdevinfo",   /* 43 - 48 */
		"getdevlist",   "layoutcommit", "layoutget",    "layoutreturn", "secunfononam", "sequence",     /* 49 - 54 */
		"set_ssv",      "test_stateid", "want_deleg",   "destory_clid", "reclaim_comp", "stat60",       /* 55 - 60 */
		"stat61",       "stat62",       "stat63",       "stat64",       "stat65",       "stat66",       /* 61 - 66 */
		"stat67",       "stat68",       "stat69",       "stat70",       "stat71",       "stat72"        /* 67 - 72 */
	},

	.nfs_v3_names ={
		"null", "getattr", "setattr", "lookup", "access", "readlink",
		"read", "write", "create", "mkdir", "symlink", "mknod",
		"remove", "rmdir", "rename", "link", "readdir", "readdirplus",
		"fsstat", "fsinfo", "pathconf", "commit"
	},

	.nfs_v2_names = {
		"null", "getattr", "setattr", "root", "lookup", "readlink",
		"read", "wrcache", "write", "create", "remove", "rename",
		"link", "symlink", "mkdir", "rmdir", "readdir", "fsstat"
	},
};

struct smp_brk smp_brk = {
	.show_smp = 0,
	.smp_first_time = 1,
	.show_raw = 0,
	.cpu_line = "---------------------------+-------------------------------------------------+",
};


/* for Disk Busy rain style output covering 100's of diskss on one screen */
/*"00000555551111111111222222222233333333334444444444555555555566666666667777777777888888888899999999991"*/
extern char * month[12];

int proc_cpu_done = 0;  /* Flag if we have run function proc_cpu() already in this interval */
int reread =0;
int time_stamp_type =0;
int nmon_one_in  = 1;
struct tm *tim; /* used to work out the hour/min/second */
int debug =0;
/* Counts of resources */
int     partitions = 0;         /* number of partitions in system  */
int     partitions_short = 0;   /* partitions file data short form (i.e. data missing) */
/* Mode of output variables */
int     show_para    = 1;
int     show_headings= 1;
int     flash_on     = 0;
double ignore_procdisk_threshold = 0.1;
double ignore_io_threshold      = 0.1;
/* Curses support */
int     cursed = 1;     /* 1 = using curses and
						   0 = loging output for a spreadsheet */
int     colour = 1;     /* 1 = using colour curses and
						   0 = using black and white curses  (see -b flag) */
char errorstr[70];
int error_on = 0;

int aiorunning;
int aiorunning_max = 0;
int aiocount;
int aiocount_max = 0;
float aiotime;
float aiotime_max =0.0;

struct {
	int pid;
	char *args;
} arglist[ARGSMAX];

/* Added variable to remember started children
 * 0 - start
 * 1 - snap
 * 2 - end
 */
int nmon_children[3] = {-1,-1,-1};
/* Cut of everything after the first space in callback
 * Delete any '&' just before the space
 */
char *check_call_string (char* callback, const char* name)
{
	char * tmp_ptr = callback;

	if (strlen(callback) > 256) {
		fprintf(stderr,"ERROR nmon: ignoring %s - too long\n", name);
		return (char *) NULL;
	}

	for( ; *tmp_ptr != '\0' && *tmp_ptr != ' ' && *tmp_ptr != '&'; ++tmp_ptr )
		;

	*tmp_ptr = '\0';

	if( tmp_ptr == callback )
		return (char *)NULL;
	else
		return callback;
}

/* Remove error output to this buffer and display it if NMONDEBUG=1 */
void error(char *err)
{
	strncpy(errorstr,err,69);
}

/* Maximum number of lines in /g_data.proc files */
/* Intel already has 26 (so here 30) per Hypterthread CPU (max 128*2 CPUs here) */
/* POWER has only 6 to 7 lines but gets  1536 SMT threads soon */
/* Erring on the saf side below */

#define MVPRINTW(row,col,string) {move((row),(col)); \
	attron(A_STANDOUT); \
	printw(string); \
	attroff(A_STANDOUT); }

/* Full Args Mode stuff here */


void args_output(int pid, int loop, char * progname)
{
	FILE *pop;
	int i,j,n;
	char tmpstr[CMDLEN];
	static int arg_first_time = 1;

	if(pid == 0)
		return; /* ignore init */
	for(i=0;i<ARGSMAX-1;i++ ) {   /* clear data out */
		if(arglist[i].pid == pid){
			return;
		}
		if(arglist[i].pid == 0) /* got to empty slot */
			break;
	}
	sprintf(tmpstr,"ps -p %d -o args 2>/dev/null", pid);
	pop = popen(tmpstr, "r");
	if(pop == NULL) {
		return;
	} else {
		if(fgets(tmpstr, CMDLEN, pop) == NULL) { /* throw away header */
			pclose(pop);
			return;
		}
		tmpstr[0]=0;
		if(fgets(tmpstr, CMDLEN, pop) == NULL) {
			pclose(pop);
			return;
		}
		tmpstr[strlen(tmpstr)-1]=0;
		if(tmpstr[strlen(tmpstr)-1]== ' ')
			tmpstr[strlen(tmpstr)-1]=0;
		arglist[i].pid = pid;
		if(arg_first_time) {
			fprintf(g_data.fp_ss,"UARG,+Time,PID,ProgName,FullCommand\n");
			arg_first_time = 0;
		}
		n=strlen(tmpstr);
		for(i=0;i<n;i++) {
			/*strip out stuff that confused Excel i.e. starting with maths symbol*/
			if(tmpstr[i] == ',' &&
					((tmpstr[i+1] == '-') || tmpstr[i+1] == '+')  )
				tmpstr[i+1] = '_';
			/*strip out double spaces */
			if(tmpstr[i] == ' ' && tmpstr[i+1] == ' ') {
				for(j=0;j<n-i;j++)
					tmpstr[i+j]=tmpstr[i+j+1];
				i--; /* rescan to remove triple space etc */
			}
		}

		fprintf(g_data.fp_ss,"UARG,%s,%07d,%s,%s\n",LOOP(g_data.loop,g_data.show_rrd),pid,progname,tmpstr);
		pclose(pop);
		return;
	}
}

void args_load ()
{
	FILE *pop;
	int i;
	char tmpstr[CMDLEN];

	for(i=0;i<ARGSMAX;i++ ) {   /* clear data out */
		if(arglist[i].pid == -1)
			break;
		if(arglist[i].pid != 0){
			arglist[i].pid = -1;
			free(arglist[i].args);
		}
	}
	pop = popen("ps -eo pid,args 2>/dev/null", "r");
	if(pop == NULL) {
		return;
	} else {
		if(fgets(tmpstr, CMDLEN, pop) == NULL) { /* throw away header */
			pclose(pop);
			return;
		}
		for(i=0;i<ARGSMAX;i++ ) {
			tmpstr[0]=0;
			if(fgets(tmpstr, CMDLEN, pop) == NULL) {
				pclose(pop);
				return;
			}
			tmpstr[strlen(tmpstr)-1]=0;
			if(tmpstr[strlen(tmpstr)-1]== ' ')
				tmpstr[strlen(tmpstr)-1]=0;
			arglist[i].pid = atoi(tmpstr);
			arglist[i].args = malloc(strlen(tmpstr));
			strcpy(arglist[i].args,&tmpstr[6]);
		}
		pclose(pop);
	}
}

char *args_lookup (int pid, char * progname)
{
	int i;
	for(i=0;i<ARGSMAX;i++) {
		if(arglist[i].pid == pid)
			return arglist[i].args;
		if(arglist[i].pid == -1)
			return progname;
	}
	return progname;
}
/* end args mode stuff here */


/* Main data structure for collected stats.
 * Two versions are previous and current data.
 * Often its the difference that is printed.
 * The pointers are swaped i.e. current becomes the previous
 * and the previous over written rather than moving data around.
 */


/* Supports up to 780, but not POWER6 595 follow-up with POWER7 */
/* XXXX needs rework to cope to with fairly rare but interesting higher numbers of CPU machines */

int cmdcheck (char *cmd)
{
	int i;
#ifdef CMDDEBUG
	fprintf(stderr,"top_brk.cmdfound=%d\n",top_brk.cmdfound);
	for(i=0;i<top_brk.cmdfound;i++)
		fprintf(stderr,"top_brk.cmdlist[%d]=\"%s\"\n",i,top_brk.cmdlist[i]);
#endif /* CMDDEBUG */
	for(i=0;i<top_brk.cmdfound;i++) {
		if(strlen(top_brk.cmdlist[i]) == 0)
			continue;
		if( !strncmp(top_brk.cmdlist[i],cmd,strlen(top_brk.cmdlist[i])) )
			return 1;
	}
	return 0;
}

/* Convert secs + micro secs to a double */
double  doubletime (void)
{
	gettimeofday(&g_data.p->tv, 0);
	return((double)g_data.p->tv.tv_sec + g_data.p->tv.tv_usec * 1.0e-6);
}

void init_pairs()
{
	if(g_data.colour)  init_pair((short)0,(short)7,(short)0); /* White */
	if(g_data.colour)  init_pair((short)1,(short)1,(short)0); /* Red */
	if(g_data.colour)  init_pair((short)2,(short)2,(short)0); /* Green */
	if(g_data.colour)  init_pair((short)3,(short)3,(short)0); /* Yellow */
	if(g_data.colour)  init_pair((short)4,(short)4,(short)0); /* Blue */
	if(g_data.colour)  init_pair((short)5,(short)5,(short)0); /* Magenta */
	if(g_data.colour)  init_pair((short)6,(short)6,(short)0); /* Cyan */
	if(g_data.colour)  init_pair((short)7,(short)7,(short)0); /* White */
	if(g_data.colour)  init_pair((short)8,(short)0,(short)1); /* Red background, red text */
	if(g_data.colour)  init_pair((short)9,(short)0,(short)2); /* Green background, green text */
	if(g_data.colour)  init_pair((short)10,(short)0,(short)4); /* Blue background, blue text */
	if(g_data.colour)  init_pair((short)11,(short)0,(short)3); /* Yellow background, yellow text */
	if(g_data.colour)  init_pair((short)12,(short)0,(short)6); /* Cyan background, cyan text */
}

/* Signal handler
 * SIGUSR1 or 2 is used to stop nmon cleanly
 * SIGWINCH is used when the window size is changed
 */
void interrupt(int signum)
{
	int child_pid;
	int waitstatus;
	if (signum == SIGCHLD ) {
		while((child_pid = waitpid(0, &waitstatus, 0)) == -1 ) {
			if( errno == EINTR) /* retry */
				continue;
			return; /* ECHLD, EFAULT */
		}
		if(child_pid == nmon_children[CHLD_SNAP])
			nmon_children[CHLD_SNAP] = -1;
		signal(SIGCHLD, interrupt);
		return;
	}
	if (signum == SIGUSR1 || signum == SIGUSR2) {
		g_data.maxloops = g_data.loop;
		return;
	}
	if (signum == SIGWINCH) {
		CURSE endwin(); /* stop + start curses so it works out the # of row and cols */
		CURSE initscr();
		CURSE cbreak();
		CURSE keypad(stdscr, TRUE);
		signal(SIGWINCH, interrupt);
		if(g_data.colour)  g_data.colour = has_colors();
		if(g_data.colour)  start_color();
		if(g_data.colour)  init_pairs();
		CURSE clear();
		return;
	}
	CURSE endwin();
	exit(0);
}


/* only place the q=previous and p=currect pointers are modified */
void switcher(void)
{
	static int      which = 1;
	int i;

	if (which) {
		g_data.p = &(g_data.database[0]);
		g_data.q = &(g_data.database[1]);
		which = 0;
	} else {
		g_data.p = &(g_data.database[1]);
		g_data.q = &(g_data.database[0]);
		which = 1;
	}
	if(flash_on)
		flash_on = 0;
	else
		flash_on = 1;

	/* Reset flags so /g_data.proc/... is re-read in next interval */
	for(i = 0; i < P_NUMBER; i++) {
		g_data.proc[i].read_this_interval = 0;
	}
#ifdef POWER
	lparcfg_brk.lparcfg_processed=0;
#endif
}


/* Lookup the right string */
char * status(int n)
{
	switch (n) {
		case 0:
			return "Run  ";
		default:
			return "Sleep";
	}
}

/* Lookup the right g_data.process state string */
char    *get_state( char n)
{
	static char     duff[64];
	switch (n) {
		case 'R': return "Running  ";
		case 'S': return "Sleeping ";
		case 'D': return "DiskSleep";
		case 'Z': return "Zombie   ";
		case 'T': return "Traced   ";
		case 'W': return "Paging   ";
		default:
				  sprintf(duff, "%d", n);
				  return duff;
	}
}

#ifdef GETUSER
/* Convert User id (UID) to a name with caching for speed
 * getpwuid() should be NFS/yellow pages safe
 */
char    *getuser(uid_t uid)
{
#define NAMESIZE 16
	struct user_info {
		uid_t uid;
		char    name[NAMESIZE];
	};
	static struct user_info *u = NULL;
	static int      used = 0;
	int     i;
	struct passwd *pw;
	void * tmp_p = NULL;

	i = 0;
	if (u != NULL) {
		for (i = 0; i < used; i++) {
			if (u[i].uid == uid) {
				return u[i].name;
			}
		}
		while ((tmp_p = realloc(u, (sizeof(struct user_info ) * (i + 1)))) == NULL)
			;
		u = (struct user_info *)tmp_p;
	} else
		u = (struct user_info *)malloc(sizeof(struct user_info ));
	used++;

	/* failed to find a match so add it */
	u[i].uid = uid;
	pw = getpwuid(uid);

	if (pw != NULL)
		strncpy(u[i].name, pw->pw_name, NAMESIZE);
	else
		sprintf(u[i].name, "unknown%d",uid);
	return u[i].name;
}
#endif /* GETUSER */

/* User Defined Disk Groups */


void hint(void)
{
	printf("\nHint: %s [-h] [-s <seconds>] [-c <count>] [-f -d <disks> -t -r <name>] [-x]\n\n", g_data.progname);
	printf("\t-h            FULL help information\n");
	printf("\tInteractive-Mode:\n");
	printf("\tread startup banner and type: \"h\" once it is running\n");
	printf("\tFor Data-Collect-Mode (-f)\n");
	printf("\t-f            spreadsheet output format [note: default -s300 -c288]\n");
	printf("\toptional\n");
	printf("\t-s <g_data.seconds>  between refreshing the screen [default 2]\n");
	printf("\t-c <number>   of refreshes [default millions]\n");
	printf("\t-d <disks>    to increase the number of disks [default 256]\n");
	printf("\t-t            spreadsheet includes top processes\n");
	printf("\t-x            capacity planning (15 min for 1 day = -fdt -s 900 -c 96)\n");
	printf("\n");
}

void help(void)
{
	hint();
	printf("Version - %s\n\n",SccsId);
	printf("For Interactive-Mode\n");
	printf("\t-s <g_data.seconds>  time between refreshing the screen [default 2]\n");
	printf("\t-c <number>   count of screen refreshes [default millions]\n");
	printf("\t-g <filename> User Defined Disk Groups [hit g to show them]\n");
	printf("\t              - file = on each line: group_name <disks list> space separated\n");
	printf("\t              - like: database sdb sdc sdd sde\n");
	printf("\t              - upto 64 disk groups, 512 disks per line\n");
	printf("\t              - disks can appear more than once and in many groups\n");
	printf("\t-g auto       - will make a file called \"auto\" with just disks fron \"lsblk|grep disk\" output\n");
	printf("\t-b            black and white [default is colour]\n");
	printf("\texample: %s -s 1 -c 100\n",g_data.progname);
	printf("\n");
	printf("For Data-Collect-Mode = spreadsheet format (comma separated values)\n");
	printf("\tNote: use only one of f,F,z,x or X and make it the first argument\n");
	printf("\t-f            spreadsheet output format [note: default -s300 -c288]\n");
	printf("\t\t\t output file is <g_data.hostname>_YYYYMMDD_HHMM.nmon\n");
	printf("\t-F <filename> same as -f but user supplied filename\n");
	printf("\t-r <runname>  used in the spreadsheet file [default g_data.hostname]\n");
	printf("\t-t            include top processes in the output\n");
	printf("\t-T            as -t plus saves command line arguments in UARG section\n");
	printf("\t-s <g_data.seconds>  between snap shots\n");
	printf("\t-c <number>   of snapshots before nmon stops\n");
	printf("\t-d <disks>    to increase the number of disks [default 256]\n");
	printf("\t-l <dpl>      disks/line default 150 to avoid spreadsheet issues. EMC=64.\n");
	printf("\t-g <filename> User Defined Disk Groups (see above) - see BBBG & DG lines\n");
	printf("\t-g auto       As above but makes the file \"auto\" for you of just the disks like sda etc.\n");
	printf("\t-D            Use with -g to add the Disk wait/service time & inflight stats.\n");

	printf("\t-N            include NFS Network File System\n");
	printf("\t-I <percent>  Include process & disks busy threshold (default 0.1)\n");
	printf("\t              don't save or show proc/disk using less than this percent\n");
	printf("\t-m <directory> nmon changes to this directory before saving to file\n");
	printf("\texample: collect for 1 hour at 30 second intervals with top procs\n");
	printf("\t\t %s -f -t -r Test1 -s30 -c120\n",g_data.progname);
	printf("\n");
	printf("\tTo load into a spreadsheet:\n");
	printf("\tsort -A *nmon >stats.csv\n");
	printf("\ttransfer the stats.csv file to your PC\n");
	printf("\tStart spreadsheet & then Open type=comma-separated-value ASCII file\n");
	printf("\t The nmon analyser or consolidator does not need the file sorted.\n");
	printf("\n");
	printf("Capacity planning mode - use cron to run each day\n");
	printf("\t-x            sensible spreadsheet output for CP =  one day\n");
	printf("\t              every 15 mins for 1 day ( i.e. -ft -s 900 -c 96)\n");
	printf("\t-X            sensible spreadsheet output for CP = busy hour\n");
	printf("\t              every 30 secs for 1 hour ( i.e. -ft -s 30 -c 120)\n");
	printf("\n");

	printf("Interactive Mode Commands\n");
	printf("\tkey --- Toggles to control what is displayed ---\n");
	printf("\th   = Online help information\n");
	printf("\tr   = Machine type, machine name, cache details and OS version + LPAR\n");
	printf("\tc   = CPU by processor stats with bar graphs\n");
	printf("\tl   = long term CPU (over 75 snapshots) with bar graphs\n");
	printf("\tm   = Memory stats\n");
	printf("\tL   = Huge memory page stats\n");
	printf("\tV   = Virtual Memory and Swap stats\n");
	printf("\tk   = Kernel Internal stats\n");
	printf("\tn   = Network stats and errors\n");
	printf("\tN   = NFS Network File System\n");
	printf("\td   = Disk I/O Graphs\n");
	printf("\tD   = Disk I/O Stats\n");
	printf("\to   = Disk I/O Map (one character per disk showing how busy it is)\n");
	printf("\tg   = User Defined Disk Groups        (assumes -g <file> when nmon started)\n");
	printf("\tG   = Change Disk stats to just disks (assumes -g auto   when nmon started)\n");
	printf("\tj   = File Systems \n");
	printf("\tt   = Top Process stats: select the data & order 1=Basic, 3=Perf 4=Size 5=I/O=root only\n");
	printf("\tt   = Top Process stats use 1,3,4,5 to select the data & order\n");
	printf("\tu   = Top Process full command details\n");
	printf("\tv   = Verbose mode - tries to make recommendations\n");
#ifdef PARTITIONS
	printf("\tP   = Partitions Disk I/O Stats\n");
#endif
#ifdef POWER
	printf("\tp   = Logical Partitions Stats\n");
#endif
	printf("\tb   = black and white mode (or use -b option)\n");
	printf("\t.   = minimum mode i.e. only busy disks and processes\n");
	printf("\n");
	printf("\tkey --- Other Controls ---\n");
	printf("\t+   = double the screen refresh time\n");
	printf("\t-   = halves the screen refresh time\n");
	printf("\tq   = quit (also x, e or control-C)\n");
	printf("\t0   = reset peak counts to zero (peak = \">\")\n");
	printf("\tspace = refresh screen now\n");
	printf("\n");
	printf("Startup Control\n");
	printf("\tIf you find you always type the same toggles every time you start\n");
	printf("\tthen place them in the NMON shell variable. For example:\n");
	printf("\t export NMON=cmdrvtan\n");

	printf("\n");
	printf("Others:\n");
	printf("\ta) To you want to stop nmon - kill -USR2 <nmon-pid>\n");
	printf("\tb) Use -p and nmon outpg_data.uts the background process pid\n");
	printf("\tc) To limit the processes nmon lists (online and to a file)\n");
	printf("\t   Either set NMONCMD0 to NMONCMD63 to the program names\n");
	printf("\t   or use -C cmd:cmd:cmd etc. example: -C ksh:vi:syncd\n");
	printf("\td) If you want to pipe nmon output to other commands use a FIFO:\n");
	printf("\t   mkfifo /tmp/mypipe\n");
	printf("\t   nmon -F /tmp/mypipe &\n");
	printf("\t   grep /tmp/mypipe\n");
	printf("\te) If nmon fails please report it with:\n");
	printf("\t   1) nmon version like: %s\n",VERSION);
	printf("\t   2) the output of cat /proc/cpuinfo\n");
	printf("\t   3) some clue of what you were doing\n");
	printf("\t   4) I may ask you to run the debug version\n");
	printf("\tf) Default spreadsheet path is: /var/log/nmon/\n");
	printf("\t   The nmon_analyser_v47.xlsm is puted in /var/log/nmon/, You can copy to Win7 to use it.\n");
	printf("\t   nmon_analyser_v47.xlsm can analy spreadsheet file(In Excel)\n");
	printf("\n");
	printf("\tDeveloper Nigel Griffiths\n");
	printf("\tFeedback welcome - on the current release only and state exactly the problem\n");
	printf("\tNo warranty given or implied.\n");
	exit(0);
}

/* We order this array rather than the actual g_data.process tables
 * the index is the position in the g_data.process table and
 * the size is the memory used  in bytes
 * the io is the storge I/O performed in the the last period in bytes
 * the time is the CPU used in the last period in g_data.seconds
 */

/* checkinput is the subroutine to handle user input */
int checkinput(void)
{
	static int use_env = 1;
	char buf[1024];
	int bytes = 0;
	int chars;
	int i;
	char * p;

	if (!cursed) /* not user input so stop with control-C */
		return 0;
	//ioctl(fileno(stdin), FIONREAD, &bytes);

	if(use_env) {
		use_env = 0;
		p = getenv("NMON");
		if(p != 0){
			strcpy(buf,p);
			chars = strlen(buf);
		}
		else 
			chars = 0;
		if (chars > 0) {
			welcome_brk.welcome = 0;
			for (i = 0; i < chars; i++) {
				switch (buf[i]) {
					case 'x':
					case 'q':
						nocbreak();
						endwin();
						exit(0);
					case '6':
					case '7':
					case '8':
					case '9':
						cpuinfo_brk.dotline = buf[i] - '0';
						break;
					case '+':
						g_data.seconds = g_data.seconds * 2;
						break;
					case '-':
						g_data.seconds = g_data.seconds / 2;
						if (g_data.seconds < 1)
							g_data.seconds = 1;
						break;
					case '.':
						if (g_data.show_all)
							g_data.show_all = 0;
						else {
							g_data.show_all = 1;
							disk_brk.show_disk = SHOW_DISK_STATS;
							top_brk.show_top = 1;
							top_brk.show_topmode =3;
						}
						g_data.change_show = 1;
						break;
					case '?':
					case 'h':
					case 'H':
						if (help_brk.show_help)
							help_brk.show_help = 0;
						else {
							help_brk.show_help = 1;
							cpuinfo_brk.show_verbose = 0;
						}
						g_data.change_show = 1;
						break;
					case 'b':
					case 'B':
						FLIP(g_data.colour);
						g_data.change_show = 1;
						clear();
						break;
					case 'Z':
						FLIP(smp_brk.show_raw);
						smp_brk.show_smp=1;
						g_data.change_show = 1;
						break;
					case 'l':
						FLIP (cpuinfo_brk.show_longterm);
						g_data.change_show = 1;
						break;
#ifdef POWER
					case 'p':
						FLIP(lparcfg_brk.show_lpar);
						g_data.change_show = 1;
						break;
#endif
					case 'V':
						FLIP(mem_brk.show_vm);
						g_data.change_show = 1;
						break;
					case 'j':
					case 'J':
						FLIP(jfs_brk.show_jfs);
						g_data.change_show = 1;
						break;
					case 'k':
					case 'K':
						FLIP(kernel_brk.show_kernel);
						g_data.change_show = 1;
						break;
					case 'm':
					case 'M':
						FLIP(mem_brk.show_memory);
						g_data.change_show = 1;
						break;
					case 'L':
						FLIP(large_brk.show_large);
						g_data.change_show = 1;
						break;
					case 'D':
						switch (disk_brk.show_disk) {
							case SHOW_DISK_NONE:
								disk_brk.show_disk = SHOW_DISK_STATS;
								break;
							case SHOW_DISK_STATS:
								disk_brk.show_disk = SHOW_DISK_NONE;
								break;
							case SHOW_DISK_GRAPH:
								disk_brk.show_disk = SHOW_DISK_STATS;
								break;
						}
						g_data.change_show = 1;
						break;
					case 'd':
						switch (disk_brk.show_disk) {
							case SHOW_DISK_NONE:
								disk_brk.show_disk = SHOW_DISK_GRAPH;
								break;
							case SHOW_DISK_STATS:
								disk_brk.show_disk = SHOW_DISK_GRAPH;
								break;
							case SHOW_DISK_GRAPH:
								disk_brk.show_disk = 0;
								break;
						}
						g_data.change_show = 1;
						break;
					case 'o':
					case 'O':
						FLIP(disk_brk.show_diskmap);
						g_data.change_show = 1;
						break;
					case 'n':
						if (net_brk.show_net) {
							net_brk.show_net = 0;
							net_brk.show_neterror = 0;
						} else {
							net_brk.show_net = 1;
							net_brk.show_neterror = 3;
						}
						g_data.change_show = 1;
						break;
					case 'N':
						if(nfs_brk.show_nfs == 0)
							nfs_brk.show_nfs = 1;
						else if(nfs_brk.show_nfs == 1)
							nfs_brk.show_nfs = 2;
						else if(nfs_brk.show_nfs == 2)
							nfs_brk.show_nfs = 3;
						else if(nfs_brk.show_nfs == 3)
							nfs_brk.show_nfs = 0;
						nfs_brk.nfs_clear=1;
						g_data.change_show = 1;
						break;
					case 'c':
					case 'C':
						FLIP(smp_brk.show_smp);
						g_data.change_show = 1;
						break;
					case 'r':
					case 'R':
						FLIP(cpuinfo_brk.show_cpu);
						g_data.change_show = 1;
						break;
					case 't':
						top_brk.show_topmode = 3; /* Fall Through */
						g_data.change_show = 1;
					case 'T':
						FLIP(top_brk.show_top);
						g_data.change_show = 1;
						break;
					case 'v':
						FLIP(cpuinfo_brk.show_verbose);
						g_data.change_show = 1;
						break;
					case 'u':
						if (top_brk.show_args == ARGS_NONE) {
							args_load();
							top_brk.show_args = ARGS_ONLY;
							top_brk.show_top = 1;
							if( top_brk.show_topmode != 3 &&
									top_brk.show_topmode != 4 &&
									top_brk.show_topmode != 5 )
								top_brk.show_topmode = 3;
						} else
							top_brk.show_args = ARGS_NONE;
						g_data.change_show = 1;
						break;
					case '1':
						top_brk.show_topmode = 1;
						top_brk.show_top = 1;
						g_data.change_show = 1;
						break;
						/*
						   case '2':
						   top_brk.show_topmode = 2;
						   show_top = 1;
						   clear();
						   break;
						   */
					case '3':
						top_brk.show_topmode = 3;
						top_brk.show_top = 1;
						g_data.change_show = 1;
						break;
					case '4':
						top_brk.show_topmode = 4;
						top_brk.show_top = 1;
						g_data.change_show = 1;
						break;
					case '5':
						if(g_data.isroot) {
							top_brk.show_topmode = 5;
							top_brk.show_top = 1;
						}
						g_data.change_show = 1;
						break;
					case '0':
						for(i=0;i<(cpuinfo_brk.max_cpus+1);i++)
							cpuinfo_brk.cpu_peak[i]=0;
						for(i=0;i<net_brk.networks;i++) {
							net_brk.net_read_peak[i]=0.0;
							net_brk.net_write_peak[i]=0.0;
							net_brk.net_read_peak_by_bits[i]=0.0;
							net_brk.net_write_peak_by_bits[i]=0.0;
						}
						for(i=0;i<disk_brk.disks;i++) {
							disk_brk.disk_busy_peak[i]=0.0;
							disk_brk.disk_rate_peak[i]=0.0;
						}
						snap_clear(&cpuinfo_brk);
						aiocount_max = 0;
						aiotime_max = 0.0;
						aiorunning_max = 0;
						large_brk.huge_peak = 0;
						g_data.change_show = 1;
						break;
					case 'G':
						if(disk_brk.auto_dgroup) {
							FLIP(disk_brk.disk_only_mode);
							clear();
						}
						g_data.change_show = 1;
						break;
					case 'g':
						FLIP(disk_brk.show_dgroup);
						g_data.change_show = 1;
						break;
					case KEY_UP:
						if (g_data.cur_line > 0)
							g_data.cur_line--;
						break;
					case KEY_DOWN:
						if (LINES - 2 + g_data.cur_line < g_data.x)
							g_data.cur_line++;
						break;
					default: return 0;
				}
			}
			return 1;
		}
	}
	else {
		timeout(g_data.seconds * 1000);
		chars = getch ();
		switch (chars) {
			case 'x':
			case 'q':
				nocbreak();
				endwin();
				sem_wait (&sem);
				pthread_kill (collect_tid, NULL);
				sem_destroy (&sem);
				exit(0);
			case '6':
			case '7':
			case '8':
			case '9':
				cpuinfo_brk.dotline = buf[i] - '0';
				break;
			case '+':
				g_data.seconds = g_data.seconds * 2;
				break;
			case '-':
				g_data.seconds = g_data.seconds / 2;
				if (g_data.seconds < 1)
					g_data.seconds = 1;
				break;
			case '.':
				if (g_data.show_all)
					g_data.show_all = 0;
				else {
					g_data.show_all = 1;
					disk_brk.show_disk = SHOW_DISK_STATS;
					top_brk.show_top = 1;
					top_brk.show_topmode =3;
				}
				g_data.change_show = 1;
				break;
			case '?':
			case 'h':
			case 'H':
				if (help_brk.show_help)
					help_brk.show_help = 0;
				else {
					help_brk.show_help = 1;
					cpuinfo_brk.show_verbose = 0;
				}
				g_data.change_show = 1;
				break;
			case 'b':
			case 'B':
				FLIP(g_data.colour);
				g_data.change_show = 1;
				clear();
				break;
			case 'Z':
				FLIP(smp_brk.show_raw);
				g_data.change_show = 1;
				smp_brk.show_smp=1;
				break;
			case 'l':
				FLIP (cpuinfo_brk.show_longterm);
				g_data.change_show = 1;
				break;
#ifdef POWER
			case 'p':
				FLIP(lparcfg_brk.show_lpar);
				g_data.change_show = 1;
				break;
#endif
			case 'V':
				FLIP(mem_brk.show_vm);
				g_data.change_show = 1;
				break;
			case 'j':
			case 'J':
				FLIP(jfs_brk.show_jfs);
				g_data.change_show = 1;
				break;
			case 'k':
			case 'K':
				FLIP(kernel_brk.show_kernel);
				g_data.change_show = 1;
				break;
			case 'm':
			case 'M':
				FLIP(mem_brk.show_memory);
				g_data.change_show = 1;
				break;
			case 'L':
				FLIP(large_brk.show_large);
				g_data.change_show = 1;
				break;
			case 'D':
				switch (disk_brk.show_disk) {
					case SHOW_DISK_NONE:
						disk_brk.show_disk = SHOW_DISK_STATS;
						break;
					case SHOW_DISK_STATS:
						disk_brk.show_disk = SHOW_DISK_NONE;
						break;
					case SHOW_DISK_GRAPH:
						disk_brk.show_disk = SHOW_DISK_STATS;
						break;
				}
				g_data.change_show = 1;
				break;
			case 'd':
				switch (disk_brk.show_disk) {
					case SHOW_DISK_NONE:
						disk_brk.show_disk = SHOW_DISK_GRAPH;
						break;
					case SHOW_DISK_STATS:
						disk_brk.show_disk = SHOW_DISK_GRAPH;
						break;
					case SHOW_DISK_GRAPH:
						disk_brk.show_disk = 0;
						break;
				}
				g_data.change_show = 1;
				break;
			case 'o':
			case 'O':
				FLIP(disk_brk.show_diskmap);
				g_data.change_show = 1;
				break;
			case 'n':
				if (net_brk.show_net) {
					net_brk.show_net = 0;
					net_brk.show_neterror = 0;
				} else {
					net_brk.show_net = 1;
					net_brk.show_neterror = 3;
				}
				g_data.change_show = 1;
				break;
			case 'N':
				if(nfs_brk.show_nfs == 0)
					nfs_brk.show_nfs = 1;
				else if(nfs_brk.show_nfs == 1)
					nfs_brk.show_nfs = 2;
				else if(nfs_brk.show_nfs == 2)
					nfs_brk.show_nfs = 3;
				else if(nfs_brk.show_nfs == 3)
					nfs_brk.show_nfs = 0;
				nfs_brk.nfs_clear=1;
				g_data.change_show = 1;
				break;
			case 'c':
			case 'C':
				FLIP(smp_brk.show_smp);
				g_data.change_show = 1;
				break;
			case 'r':
			case 'R':
				FLIP(cpuinfo_brk.show_cpu);
				g_data.change_show = 1;
				break;
			case 't':
				top_brk.show_topmode = 3; /* Fall Through */
				g_data.change_show = 1;
			case 'T':
				FLIP(top_brk.show_top);
				g_data.change_show = 1;
				break;
			case 'v':
				FLIP(cpuinfo_brk.show_verbose);
				g_data.change_show = 1;
				break;
			case 'u':
				if (top_brk.show_args == ARGS_NONE) {
					args_load();
					top_brk.show_args = ARGS_ONLY;
					top_brk.show_top = 1;
					if( top_brk.show_topmode != 3 &&
							top_brk.show_topmode != 4 &&
							top_brk.show_topmode != 5 )
						top_brk.show_topmode = 3;
				} else
					top_brk.show_args = ARGS_NONE;
				g_data.change_show = 1;
				break;
			case '1':
				top_brk.show_topmode = 1;
				g_data.change_show = 1;
				top_brk.show_top = 1;
				break;
				/*
				   case '2':
				   top_brk.show_topmode = 2;
				   show_top = 1;
				   clear();
				   break;
				   */
			case '3':
				top_brk.show_topmode = 3;
				g_data.change_show = 1;
				top_brk.show_top = 1;
				break;
			case '4':
				top_brk.show_topmode = 4;
				g_data.change_show = 1;
				top_brk.show_top = 1;
				break;
			case '5':
				if(g_data.isroot) {
					top_brk.show_topmode = 5;
					top_brk.show_top = 1;
				}
				g_data.change_show = 1;
				break;
			case '0':
				for(i=0;i<(cpuinfo_brk.max_cpus+1);i++)
					cpuinfo_brk.cpu_peak[i]=0;
				for(i=0;i<net_brk.networks;i++) {
					net_brk.net_read_peak[i]=0.0;
					net_brk.net_write_peak[i]=0.0;
					net_brk.net_read_peak_by_bits[i]=0.0;
					net_brk.net_write_peak_by_bits[i]=0.0;
				}
				for(i=0;i<disk_brk.disks;i++) {
					disk_brk.disk_busy_peak[i]=0.0;
					disk_brk.disk_rate_peak[i]=0.0;
				}
				snap_clear(&cpuinfo_brk);
				aiocount_max = 0;
				aiotime_max = 0.0;
				aiorunning_max = 0;
				large_brk.huge_peak = 0;
				g_data.change_show = 1;
				break;
			case 'G':
				if(disk_brk.auto_dgroup) {
					FLIP(disk_brk.disk_only_mode);
					clear();
				}
				g_data.change_show = 1;
				break;
			case 'g':
				FLIP(disk_brk.show_dgroup);
				g_data.change_show = 1;
				break;
			case KEY_UP:
				if (g_data.cur_line > 0)
					g_data.cur_line--;
				break;
			case KEY_DOWN:
				if (LINES - 2 + g_data.cur_line < g_data.x)
					g_data.cur_line++;
				break;
			default: 
				return 0;
		}
		return 1;
	}
	return 0;
}

void go_background(int def_loops, int def_secs)
{
	cursed = 0;

	if (g_data.maxloops == -1)
		g_data.maxloops = def_loops;
	if (g_data.seconds  == -1)
		g_data.seconds = def_secs;
	cpuinfo_brk.show_cpu     = 0;
	smp_brk.show_smp     = 0;
	disk_brk.show_disk    = 0; /* SHOW_DISK_STATS */
	jfs_brk.show_jfs     = 0;
	mem_brk.show_memory  = 0;
	large_brk.show_large   = 0;
	kernel_brk.show_kernel  = 0;
	net_brk.show_net     = 0;
	g_data.show_all     = 0;
	top_brk.show_top     = 0; /* top g_data.process */
	top_brk.show_topmode = 0; /* 3*/
	lparcfg_brk.show_lpar = 0;
	mem_brk.show_vm   = 0;
}

int isnumbers(char *s)
{
	while(*s != 0) {
		if( *s < '0' || *s > '9')
			return 0;
		s++;
	}
	return 1;
}


/* Start g_data.process as specified in cmd in a child process without waiting
 * for completion
 * not sure if want to prevent this funcitonality for root user
 * when: CHLD_START, CHLD_SNAP or CHLD_END
 * cmd:  pointer to command string - assumed to be cleansed ....
 * timestamp_type: 0 - T%04d, 1 - detailed time stamp
 * g_data.loop: loop id (0 for CHLD_START)
 * the_time: time to use for timestamp generation
 */
void child_start(int when,
		char *cmd,
		int timestamp_type,
		int loop,
		time_t the_time)
{
	int i;
	pid_t child_pid;
	char time_stamp_str[20]="";
	char *when_info="";
	struct tm *tim; /* used to work out the hour/min/second */

#ifdef DEBUG2
	fprintf(g_data.fp_ss,"child start when=%d cmd=%s time=%d g_data.loop=%d\n",when,cmd,timestamp_type,loop);
#endif
	/* Validate parameter and initialize error text */
	switch( when ) {
		case CHLD_START:
			when_info = "nmon fork exec failure CHLD_START";
			break;
		case CHLD_END:
			when_info = "nmon fork exec failure CHLD_END";
			break;

		case CHLD_SNAP:
			/* check if old child has finished - otherwise we do nothing */
			if( nmon_children[CHLD_SNAP] != -1 ) {
				if(!cursed)
					fprintf(g_data.fp_ss,"ERROR,T%04d, Starting snap command \"%s\" failed as previous child still running - killing it now\n", g_data.loop, cmd);
				kill( nmon_children[CHLD_SNAP],9);
			}

			when_info = "nmon fork exec failure CHLD_SNAP";
			break;
	}


	/* now fork off a child g_data.process. */
	switch (child_pid = fork()) {
		case -1:        /* fork failed. */
			perror(when_info);
			return;

		case 0:         /* inside child g_data.process.  */
			/* create requested timestamp */
			if( timestamp_type == 1 ) {
				tim = localtime(&the_time);
				sprintf(time_stamp_str,"%02d:%02d:%02d,%02d,%02d,%04d",
						tim->tm_hour, tim->tm_min, tim->tm_sec,
						tim->tm_mday, tim->tm_mon + 1, tim->tm_year + 1900);
			}
			else {
				sprintf(time_stamp_str,"T%04d", g_data.loop);
			}

			/* close all open file pointers except the defaults */
			for( i=3; i<5; ++i )
				close(i);

			/* Now switch to the defined command */
			execlp(cmd, cmd, time_stamp_str,(void *)0);

			/* If we get here the specified command could not be started */
			perror(when_info);
			exit(1);                     /* We can't do anything more */
			/* never reached */

		default:        /* inside parent process. */
			/* In father - remember child pid for future */
			nmon_children[when] = child_pid;
	}
}

inline void curse_display(char * nmon_snap)
{
	int i = 0;
	static int first_key_pressed = 0;
	/* Reset the cursor position to top left */


	if (g_data.update_data || g_data.change_show) {
		sem_wait (&sem);
		g_data.y = g_data.x = 0;
		box(stdscr,0,0);
		wclear(g_data.pad);

		mvprintw(g_data.x, 1, "nmon");
		mvprintw(g_data.x, 6, "%s", VERSION);
		if(flash_on)
			mvprintw(g_data.x,15,"[H for help]");
		mvprintw(g_data.x, 30, "Hostname=%s", g_data.hostname);
		//mvprintw(g_data.x, 52, "Refresh=%2.0fsecs ", g_data.elapsed);
		mvprintw(g_data.x, 52, "Refresh=%dsecs ", g_data.seconds);
		mvprintw(g_data.x, 70, "%02d:%02d.%02d",
				tim->tm_hour, tim->tm_min, tim->tm_sec);
		wnoutrefresh(stdscr);
		g_data.x = g_data.x + 1;

		if((welcome_brk.welcome && getenv("NMON") == 0)) {
			show_welcome(&welcome_brk, &cpuinfo_brk, &lparcfg_brk);
		}
		if (cpuinfo_brk.show_verbose) {
			show_verbose (&cpuinfo_brk, &disk_brk);
		}
		if (help_brk.show_help) {
			show_help(&help_brk);
		}
		if (cpuinfo_brk.show_cpu) {
			show_cpu_info(&cpuinfo_brk, &lparcfg_brk, &g_data.uts);
		}
		if (cpuinfo_brk.show_longterm ) {
			show_longterm(&cpuinfo_brk);
		}
		if (smp_brk.show_smp) {
			if( cpuinfo_brk.old_cpus != cpuinfo_brk.cpus )  {
				/* wmove(padsmp,0,0); */
				/* doesn't work CURSE wclrtobot(padsmp); */
				/* Do BRUTE force overwrite of previous data */
				if( cpuinfo_brk.cpus < cpuinfo_brk.old_cpus)    {
					for(i=cpuinfo_brk.cpus; i < cpuinfo_brk.old_cpus; i++)
						mvwprintw(g_data.pad,g_data.x + i + 4,0,"                                                                                    ");
				}
			}
			if (smp_brk.show_smp) {
				show_smp(&smp_brk, &cpuinfo_brk, &lparcfg_brk);
			}       /* if (show_smp)  */
		}       /* if (show_smp || cpuinfo_brk.show_verbose) */
#ifdef POWER
		if (lparcfg_brk.show_lpar) {
			show_lpar(&lparcfg_brk, &cpuinfo_brk);
		}
#endif /*POWER*/
		if (mem_brk.show_memory) {
			show_mem (&mem_brk, &lparcfg_brk);
		}
		if (large_brk.show_large) {
			show_large(&large_brk);
		}
		if (mem_brk.show_vm) {
			show_vm(&mem_brk);
		}
		if (kernel_brk.show_kernel) {
			show_kernel(&kernel_brk);
		}

		if (nfs_brk.show_nfs) {
			show_nfs(&nfs_brk);
		}
		if (net_brk.show_net) {
			show_net(&net_brk);
		}
#ifdef JFS
		if (jfs_brk.show_jfs) {
			jfs_load(&jfs_brk, LOAD);
			show_jfs(&jfs_brk);
			jfs_load(&jfs_brk, UNLOAD);
		}
#endif /* JFS */
		if (disk_brk.show_diskmap) {
			show_diskmap(&disk_brk);
		}
		if (disk_brk.show_disk) {
			show_disk (&disk_brk);
		}
		if ((disk_brk.show_dgroup || (!cursed && disk_brk.dgroup_loaded))) {
			show_dgroup(&disk_brk);
		}       /*              if ((show_dgroup || (!cursed && dgroup_loaded)))  */

		if (top_brk.show_top) {
			show_top_info (&top_brk, mem_brk.pagesize, ignore_procdisk_threshold);
		}
		sem_post (&sem);
		g_data.update_data = 0;
		g_data.change_show = 0;
	}
	wmove(stdscr,0, 0);

	if (g_data.cur_line + LINES - 2 < g_data.x)
		mvwprintw(stdscr, LINES-1, 10, "Warning: Some Statistics may not shown");

	/* underline the end of the stats area border */
	mvwhline(g_data.pad, g_data.x - 1, 0, ACS_HLINE,COLS-2);

	//wrefresh(stdscr);
	//doupdate();

	//			if(g_data.x < LINES-2)
	//				mvwhline(stdscr, g_data.x, 1, ' ', COLS-2);
	/*if(first_key_pressed == 0){
		first_key_pressed = 1;
		wmove(stdscr,0, 0);
		wclear(stdscr);
		wmove(stdscr,0,0);
		wclrtobot(stdscr);
		wrefresh(stdscr);
		//doupdate();
	}*/
	prefresh(g_data.pad, g_data.cur_line,0,1,1,LINES-2,COLS-2);
	//wnoutrefresh(stdscr);
	doupdate();
	if (checkinput()) {
		welcome_brk.welcome = 0;
	}
}

void main_loop (char * nmon_start, char * nmon_end, char * nmon_snap, char * nmon_tmp)
{
	int ret;
	int i = 0;
	int k = 0;
	double nmon_start_time = 0.0;
	double nmon_end_time = 0.0;
	double nmon_run_time = -1.0;
	int seconds_over = 0;
	int secs;
	struct timeval nmon_tv; /* below is used to workout the nmon run, accumalate it and the
							   allow for in in the sleep time  to reduce time drift */


	sem_wait (&sem);
	g_data.p->time = doubletime();
	g_data.elapsed = g_data.p->time - g_data.q->time;
	sem_post (&sem);
	set_timer(0);
	tim = get_timer();

	for(g_data.loop = 1; ; ) {
		/* Save the time and work out how long we were actually asleep
		 * Do this as early as possible and close to reading the CPU statistics in /proc/stat
		 */

		/* Get current count of CPU
		 * As side effect /proc/stat is read
		 */
		cpuinfo_brk.old_cpus = cpuinfo_brk.cpus;


		if (lparcfg_brk.lparcfg.timebase == -1) {
			lparcfg_brk.lparcfg.timebase = 0;
			for(i = 0; i < lparcfg_brk.ext->proc[P_CPUINFO].lines-1; i++) {
				if(!strncmp("timebase",lparcfg_brk.ext->proc[P_CPUINFO].line[i],8)) {
					sscanf(lparcfg_brk.ext->proc[P_CPUINFO].line[i],"timebase : %lld",&lparcfg_brk.lparcfg.timebase);
					break;
				}
			}
		}
		if (cursed) { /* Top line */
			curse_display(nmon_snap);
		}
		else {
			gettimeofday(&nmon_tv, 0);
			nmon_end_time = (double)nmon_tv.tv_sec + (double)nmon_tv.tv_usec * 1.0e-6;
			if(nmon_run_time  < 0.0){
				nmon_start_time = nmon_end_time;
				nmon_run_time = 0.0;
			}
			nmon_run_time += (nmon_end_time - nmon_start_time);
			if(nmon_run_time < 1.0) {
				secs = g_data.seconds;  /* sleep for the requested number of seconds */
			}
			else {
				seconds_over = (int)nmon_run_time; /* reduce the sleep time by whole number of seconds */
				secs = g_data.seconds - seconds_over;
				nmon_run_time -= (double)seconds_over;
			}
			if(secs < 1) /* sanity check in case CPUs are flat out and nmon taking far to long to complete */
				secs = 1;

redo:
			errno = 0;
			ret = sleep(secs);
			if( (ret != 0 || errno != 0) && g_data.loop != g_data.maxloops ) {
				fprintf(g_data.fp_ss,"ERROR,%s, sleep interrupted, sleep(%d g_data.seconds), return value=%d",LOOP(g_data.loop,g_data.show_rrd), secs, ret);
				fprintf(g_data.fp_ss,", errno=%d\n",errno);
				secs=ret;
				goto redo;
			}
			gettimeofday(&nmon_tv, 0);
			nmon_start_time = (double)nmon_tv.tv_sec + (double)nmon_tv.tv_usec * 1.0e-6;
		}


		if (g_data.loop >= g_data.maxloops) {
			CURSE endwin();
			if (nmon_end) {
				child_start(CHLD_END, nmon_end, time_stamp_type, g_data.loop, get_timer_t ());
				/* Give the end - processing some time - 5s for now */
				sleep(5);
			}

			exit(0);
		}
	}
}

void * collect_thread (void * args)
{
	int i = 0;
	int j = 0;

	write_header_lines (g_data.user_filename, &top_brk, &jfs_brk, &net_brk, &lparcfg_brk, &cpuinfo_brk, &disk_brk);

	while (1) {
		sleep (g_data.seconds);
		g_data.p->time = doubletime();
		g_data.elapsed = g_data.p->time - g_data.q->time;
		set_timer(0);
		tim = get_timer();

		if(g_data.loop <= 2) {
			for (i = 0; i < cpuinfo_brk.max_cpus + 1; i++)
				cpuinfo_brk.cpu_peak[i] = 0.0;
			refresh_all_data (&cpuinfo_brk, &lparcfg_brk, &net_brk, &nfs_brk, &mem_brk, &kernel_brk, &disk_brk, &top_brk, &smp_brk);
			g_data.loop++;
			continue;
		}
		smp_brk.smp_first_time = 0;
		disk_brk.disk_first_time = 0;
		lparcfg_brk.lpar_first_time = 0;
		top_brk.top_first_time = 0;
		mem_brk.vm_first_time = 0;
		nfs_brk.nfs_first_time = 0;

		sem_wait (&sem);
		refresh_all_data (&cpuinfo_brk, &lparcfg_brk, &net_brk, &nfs_brk, &mem_brk, &kernel_brk, &disk_brk, &top_brk, &smp_brk);
		cpuinfo_brk.next_cpu_snap++;

		fprintf (g_data.fp_ss,"ZZZZ,%s,%02d:%02d:%02d,%02d-%s-%4d\n", LOOP(g_data.loop,g_data.show_rrd), tim->tm_hour, tim->tm_min, tim->tm_sec, tim->tm_mday, month[tim->tm_mon], tim->tm_year+1900);
		collect_smp (&smp_brk, &cpuinfo_brk);
#ifdef POWER
		collect_lpar(&lparcfg_brk, &cpuinfo_brk);
#endif
		collect_mem (&mem_brk, &lparcfg_brk);
		collect_vm(&mem_brk);
		collect_kernel(&kernel_brk);
		collect_net(&net_brk);
#ifdef JFS
		collect_jfs(&jfs_brk);
#endif
		collect_disk (&disk_brk);
		if (top_brk.collect_top)
			collect_top_info (&top_brk, mem_brk.pagesize, ignore_procdisk_threshold);
		//			collect_dgroup(&disk_brk);
		//			collect_nfs(&nfs_brk);
		//			collect_large(&large_brk);
		g_data.loop++;
		g_data.update_data = 1;
		sem_post (&sem);
		fflush(NULL);
	}
}

int main(int argc, char **argv)
{
	int n = 0;                    /* reusable counters */
	int i = 0;
	int j = 0;
	int ret = 0;
	pid_t childpid = -1;
	int ralfmode = 0;

	/* for popen on oslevel */
	int varperftmp = 0;
	char cmdstr[256];

	char  *nmon_start = (char *)NULL;
	char  *nmon_end   = (char *)NULL;
	char  *nmon_snap  = (char *)NULL;
	char  *nmon_tmp   = (char *)NULL;
	/* Flag what kind of time stamp we give to started children
	 * 0: "T%04d"
	 * 1: "hh:mm:ss,dd,mm,yyyy"
	 */

	g_data.argc = argc;
	g_data.argv = argv;
#define INIT_BRK(brk,__g_data) {brk.ext = (__g_data);}
	INIT_BRK(disk_brk,&g_data);
	INIT_BRK(lparcfg_brk,&g_data);
	INIT_BRK(cpuinfo_brk,&g_data);
	INIT_BRK(top_brk,&g_data);
	INIT_BRK(mem_brk,&g_data);
	INIT_BRK(kernel_brk,&g_data);
	INIT_BRK(help_brk,&g_data);
	INIT_BRK(jfs_brk,&g_data);
	INIT_BRK(large_brk,&g_data);
	INIT_BRK(net_brk,&g_data);
	INIT_BRK(nfs_brk,&g_data);
	INIT_BRK(smp_brk,&g_data);
	INIT_BRK(welcome_brk,&g_data);

	ret = sem_init (&sem, 0, 1);
	if (ret == -1) {
		perror ("semaphore intitialization failed.");
		exit (1);
	}
	/* check the user supplied options */
	g_data.progname = argv[0];
	for (i=(int)strlen(g_data.progname)-1;i>0;i--)
		if(g_data.progname[i] == '/') {
			g_data.progname = &g_data.progname[i+1];
		}

	if(getenv("NMONDEBUG") != NULL)
		debug=1;
	if(getenv("NMONERROR") != NULL)
		error_on=1;
	if(getenv("NMONBUG1") != NULL)
		reread=1;
	if (getenv("NMONDEBUG") != NULL)
		debug = 1;

	if ((nmon_start = getenv("NMON_START")) != NULL) {
		nmon_start = check_call_string(nmon_start, "NMON_START");
	}

	if ((nmon_end = getenv("NMON_END")) != NULL) {
		nmon_end = check_call_string(nmon_end, "NMON_END");
	}

	if ((nmon_tmp = getenv("NMON_ONE_IN")) != NULL) {
		nmon_one_in = atoi(nmon_tmp);
		if( errno != 0 ) {
			fprintf(stderr,"ERROR nmon: invalid NMON_ONE_IN shell variable\n");
			nmon_one_in = 1;
		}
	}

	if ((nmon_snap = getenv("NMON_SNAP")) != NULL) {
		nmon_snap = check_call_string(nmon_snap, "NMON_SNAP");
	}

	if ((nmon_tmp = getenv("NMON_TIMESTAMP")) != NULL) {
		time_stamp_type = atoi(nmon_tmp);
		if (time_stamp_type != 0 && time_stamp_type != 1 )
			time_stamp_type = 1;
	}
#ifdef DEBUG2
	printf("NMON_START=%s.\n",nmon_start);
	printf("NMON_END=%s.\n",nmon_end);
	printf("NMON_SNAP=%s.\n",nmon_snap);
	printf("ONE_IN=%d.\n",nmon_one_in);
	printf("TIMESTAMP=%d.\n",time_stamp_type);
#endif

#ifdef REREAD
	reread=1;
#endif
	for(i=0; i<CMDMAX;i++) {
		sprintf(cmdstr,"NMONCMD%d",i);
		top_brk.cmdlist[i] = getenv(cmdstr);
		if(top_brk.cmdlist[i] != 0)
			top_brk.cmdfound = i+1;
	}
	/* Setup long and short Hostname */
	gethostname(g_data.hostname, sizeof(g_data.hostname));
	strcpy(g_data.fullhostname, g_data.hostname);
	for (i = 0; i < sizeof(g_data.hostname); i++)
		if (g_data.hostname[i] == '.')
			g_data.hostname[i] = 0;
	if(g_data.run_name_set == 0)
		strcpy(g_data.run_name,g_data.hostname);

	if( getuid() == 0)
		g_data.isroot=1;

	/* Check the version of OS */
	uname(&g_data.uts);
	if(sysconf(_SC_PAGESIZE) > 1024*4) /* Check if we have the large 64 KB memory page sizes compiled in to the kernel */
		mem_brk.pagesize = sysconf(_SC_PAGESIZE);
	proc_init(g_data.proc);
	while ( -1 != (i = getopt(argc, argv, "?Rhs:bc:Dd:fF:r:tTxXzeEl:qpC:Vg:Nm:I:Z" ))) {
		switch (i) {
			case '?':
				hint();
				exit(0);
			case 'h':
				help();
				break;
			case 's':
				g_data.seconds = atoi(optarg);
				break;
			case 'p':
				ralfmode = 1;
				break;
			case 'b':
				g_data.colour = 0;
				break;
			case 'c':
				g_data.maxloops = atoi(optarg);
				break;
			case 'N':
				nfs_brk.show_nfs = 1;
				break;
			case 'm':
				if(chdir(optarg) == -1) {
					perror("changing directory failed");
					printf("Directory attempted was:%s\n",optarg);
					exit(993);
				}
				break;
			case 'I':
				ignore_procdisk_threshold = atof(optarg);
				break;
			case 'd':
				disk_brk.diskmax = atoi(optarg);
				if(disk_brk.diskmax < DISKMIN) {
					printf("nmon: ignoring -d %d option as the minimum is %d\n", disk_brk.diskmax, DISKMIN);
					disk_brk.diskmax = DISKMIN;
				}
				break;
			case 'D':
				disk_brk.extended_disk=1;
				break;
			case 'R':
				g_data.show_rrd = 1;
				go_background(288, 300);
				g_data.show_aaa = 0;
				show_para = 0;
				show_headings = 0;
				break;
			case 'r': strcpy(g_data.run_name,optarg);
					  g_data.run_name_set++;
					  break;
			case 'F': /* background mode with user supplied filename */
					  strcpy(g_data.user_filename,optarg);
					  g_data.user_filename_set++;
					  go_background(288, 300);
					  break;

			case 'f': /* background mode i.e. for spread sheet output */
					  go_background(288, 300);
					  break;
			case 'T':
					  top_brk.show_args = ARGS_ONLY; /* drop through */
			case 't':
					  top_brk.show_top     = 1; /* put top process output in spreadsheet mode */
					  top_brk.show_topmode = 3;
					  break;
			case 'z': /* background mode for 1 day output to /var/perf/tmp */
					  varperftmp++;
					  go_background(4*24, 15*60);
					  break;

			case 'x': /* background mode for 1 day capacity planning */
					  go_background(4*24, 15*60);
					  top_brk.collect_top =1;
					  top_brk.collect_topmode = 3;
					  break;
			case 'X': /* background mode for 1 hour capacity planning */
					  go_background(120, 30);
					  top_brk.collect_top =1;
					  top_brk.collect_topmode = 3;
					  break;
			case 'Z':
					  smp_brk.show_raw=1;
					  break;
			case 'l':
					  disk_brk.disks_per_line = atoi(optarg);
					  if(disk_brk.disks_per_line < 3 || disk_brk.disks_per_line >250)
						  disk_brk.disks_per_line = 100;
					  break;
			case 'C': /* commandlist argument */
					  top_brk.cmdlist[0] = malloc(strlen(optarg)+1); /* create buffer */
					  strcpy(top_brk.cmdlist[0],optarg);
					  if(top_brk.cmdlist[0][0]!= 0)
						  top_brk.cmdfound=1;
					  for(i=0,j=1;top_brk.cmdlist[0][i] != 0;i++) {
						  if(top_brk.cmdlist[0][i] == ':') {
							  top_brk.cmdlist[0][i] = 0;
							  top_brk.cmdlist[j] = &top_brk.cmdlist[0][i+1];
							  j++;
							  top_brk.cmdfound=j;
							  if(j >= CMDMAX) break;
						  }
					  }
					  break;
			case 'V': /* nmon version */
					  printf("nmon verion %s\n",VERSION);
					  exit(0);
					  break;
			case 'g': /* disk groups */
					  disk_brk.show_dgroup = 1;
					  disk_brk.dgroup_loaded = 1;
					  disk_brk.dgroup_filename = optarg;
					  if( strncmp("auto",disk_brk.dgroup_filename,5) == 0) {
						  disk_brk.auto_dgroup++;
						  printf("Generating disk group file from lsblk output to file: \"auto\"\n");
						  ret = system("lsblk --nodeps --output NAME,TYPE --raw | grep disk | awk 'BEGIN {printf \"# This file created by: nmon -g auto\\n# It is an automatically generated disk-group file which excluses disk paritions\\n\" } { printf \"%s %s\\n\", $1, $1 }' >auto");
						  if(ret != 0 ) {
							  printf("Create auto file command was: %s\n",
									  "lsblk --nodeps --output NAME,TYPE --raw | grep disk | awk '{ printf \"%s %s\\n\", $1, $1 }' >auto");
							  printf("Creating auto file returned a status of %d\n", ret );
						  }
					  }
					  break;
		}
	}


	/* Set parameters if not set by above */
	if (g_data.maxloops == -1)
		g_data.maxloops = 9999999;
	if (g_data.seconds  == -1)
		g_data.seconds = 2;
	if (cursed)
		disk_brk.show_dgroup = 0;

	/* -D need -g filename */
	if(disk_brk.extended_disk == 1 && disk_brk.show_dgroup == 0) {
		printf("nmon: ignoring -D (extended disk stats) as -g filename is missing\n");
		disk_brk.extended_disk=0;
	}
	/* To get the pointers setup */
	switcher();

	init_cpuinfo_brk(&cpuinfo_brk);
	/* Determine number of active LOGICAL cpu - depends on SMT mode ! */
#ifdef X86
	get_intel_spec();
#endif
	proc_read(g_data.proc, P_STAT);
	proc_cpu(&cpuinfo_brk);
	proc_read(g_data.proc, P_UPTIME);
	proc_read(g_data.proc, P_LOADAVG);
	proc_kernel(&kernel_brk);
	memcpy(&(g_data.q->cpu_total), &(g_data.p->cpu_total), sizeof(struct cpu_stat));

	g_data.p->dk = malloc(sizeof(struct dsk_stat) * disk_brk.diskmax+1);
	g_data.q->dk = malloc(sizeof(struct dsk_stat) * disk_brk.diskmax+1);
	disk_brk.disk_busy_peak = malloc(sizeof(double) * disk_brk.diskmax);
	disk_brk.disk_rate_peak = malloc(sizeof(double) * disk_brk.diskmax);
	for(i=0;i < disk_brk.diskmax;i++) {
		disk_brk.disk_busy_peak[i]=0.0;
		disk_brk.disk_rate_peak[i]=0.0;
	}
	cpuinfo_brk.cpu_peak = malloc(sizeof(double) * (CPUMAX + 1)); /* MAGIC */
	for(i=0;i < cpuinfo_brk.max_cpus+1;i++)
		cpuinfo_brk.cpu_peak[i]=0.0;

	n = getprocs(&g_data, 0, g_data.p);
	if (n != 0)
		top_brk.cur_ps = n;
	g_data.p->procs = malloc((sizeof(struct procsinfo ) * (n+1)));
	g_data.q->procs = malloc((sizeof(struct procsinfo ) * (n+1)));
    if (g_data.p->procs == NULL || g_data.q->procs == NULL) {
        fprintf (stderr, "malloc for progress data error\n");
        return 0;
    }
	g_data.p->nprocs = g_data.q->nprocs = n;
	/* Initialise the top processes table */
	top_brk.topper = malloc(sizeof(struct topper ) * top_brk.topper_size); /* round up */

	/* Get Disk Stats. */
	proc_disk(&disk_brk);
	memcpy(g_data.q->dk, g_data.p->dk, sizeof(struct dsk_stat) * disk_brk.disks);
	/* load dgroup - if required */
	if (disk_brk.dgroup_loaded == 1) {
		load_dgroup(&disk_brk, g_data.p->dk);
	}
	/* Get Network Stats. */
	proc_net(&net_brk);
	memcpy(g_data.q->ifnets, g_data.p->ifnets, sizeof(struct net_stat) * net_brk.networks);
	for(i=0;i<net_brk.networks;i++) {
		net_brk.net_read_peak[i]=0.0;
		net_brk.net_write_peak[i]=0.0;
		net_brk.net_read_peak_by_bits[i]=0.0;
		net_brk.net_write_peak_by_bits[i]=0.0;
	}

	/* If we are running in spreadsheet mode initialize all other data sets as well
	 * so we do not get incorrect data for the first reported interval
	 */
	/* Get VM Stats */
	mem_brk.result = read_vmstat(&mem_brk);

	/* Get Memory info */
	proc_mem(&mem_brk);

#ifdef POWER
	/* Get LPAR Stats */
	proc_lparcfg(&cpuinfo_brk, &lparcfg_brk);
#endif
	/* Set the pointer ready for the next round */
	switcher();

	/* Initialise signal handlers so we can tidy up curses on exit */
	signal(SIGUSR1, interrupt);
	signal(SIGUSR2, interrupt);
	signal(SIGINT, interrupt);
	signal(SIGWINCH, interrupt);
	signal(SIGCHLD, interrupt);

	/* Start Curses */
	if (cursed) {
		initscr();
		cbreak();
		noecho();
		keypad(stdscr, TRUE);
		move(0, 0);
		refresh();
		if (g_data.colour) {
			g_data.colour = has_colors();
			start_color();
			init_pairs();
		}
		clear();
		g_data.pad = newpad(MAXROWS,MAXCOLS * 2);
	} else {
		/* disconnect from terminal */
		fflush(NULL);
		if (!debug && (childpid = fork()) != 0) {
			if(ralfmode)
				printf("%d\n",childpid);
			exit(0); /* parent returns OK */
		}
		if(!debug) {
			close(0);
			close(1);
			close(2);
			setpgrp(); /* become g_data.process group leader */
			signal(SIGHUP, SIG_IGN); /* ignore hangups */
		}
		/* Do the nmon_start activity early on */
		if (nmon_start) {
			set_timer (0);
			child_start(CHLD_START, nmon_start, time_stamp_type, 1, get_timer_t ());
		}
	}
	g_data.fp_ss = open_log_file(&g_data, varperftmp);

	/* To get the pointers setup */
	/* Was already done earlier, DONT'T switch back here to the old pointer! - switcher(); */
	/*checkinput();*/
	if ((ret = pthread_create (&collect_tid, NULL, collect_thread, NULL)) != 0) {
		perror ("Can`t create collect thread.");
		exit (1);
	}
	clear();
#ifdef POWER
	lparcfg_brk.lparcfg.timebase = -1;
#endif
	/* Main loop of the code */
	main_loop (nmon_start, nmon_end, nmon_snap, nmon_tmp);

	return 0;
}
