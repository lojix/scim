#include "scim/scim.h"
#include <time.h>

#define _HALT					ROOTSBINDIR "halt"
#define _REBOOT					ROOTSBINDIR "reboot"
#define _POWEROFF				ROOTSBINDIR "poweroff"
#define _SYNC					 ROOTBINDIR "sync"
#define _KILLALL5				ROOTSBINDIR "killall5"
#define _UMOUNT					 ROOTBINDIR "umount"

enum {
 _SHUTDOWN,
 _SHUTOFF,
 _RESTART
};

char** __halt_sundries[] = {
 (char*[]){_SYNC, NULL},
 (char*[]){_KILLALL5, "-15", NULL, NULL},
 (char*[]){_KILLALL5, "-9", NULL, NULL},
 (char*[]){_HALT, "-w", NULL},
 (char*[]){_UMOUNT, "-a", "-d", "-t", "btrfs,ext4,squashfs", NULL},
 NULL
};

char** __halt_shutdown[] = {
 [_SHUTDOWN]	= (char*[]){_HALT, "-f", "-h", "-i", "-n", NULL},
 [_SHUTOFF]	= (char*[]){_POWEROFF, "-f", "-h", "-i", "-n", NULL},
 [_RESTART]	= (char*[]){_REBOOT, "-f", "-h", "-i", "-n", NULL},
 NULL
};

int main(int argc, char* argv[])
{
	char* level;
	char*** tool;
	int halt = _SHUTOFF;

	if((level = getenv("RUNLEVEL"))) {
		switch(*level) {
			case '6':
			halt = _RESTART;
			break;

			default:
			break;
		}
	}

	for(tool = __halt_sundries; *tool; tool++) {
		scim_tool_call(*tool, NULL, NULL, NULL);
	}

	//nanosleep((struct timespec[]){{.tv_sec = 5}}, NULL);
	//scim_loop_free("/dev/loop1");
	scim_tool_call(__halt_shutdown[halt], NULL, NULL, NULL);

	_exit(EXIT_SUCCESS);
}
