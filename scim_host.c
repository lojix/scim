#include "scim/scim.h"
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <gnu/libc-version.h>
#include <sys/mount.h>
#include <sys/utsname.h>

#define _MKINITRD				ROOTSBINDIR "mkinitrd"
#define _MODPROBE				ROOTSBINDIR "modprobe"
#define _UDEVD					 ROOTLIBDIR	"udev/udevd"
#define _UDEVADM				ROOTSBINDIR "udevadm"

char* __file_modprobe[] = {
 "bonding",
 "dummy",
 "fuse",
 "ipv6",
 NULL
};

char* __tool_modprobe[] = {_MODPROBE, NULL, NULL};

char** __tool_hardware[] = {
 (char*[]){_UDEVD, "--daemon", "--resolve-names=never", NULL},
 (char*[]){_UDEVADM, "trigger", NULL},
 (char*[]){_UDEVADM, "settle", NULL},
 (char*[]){_UDEVADM, "control", "--exit", NULL},
 NULL
};

int scim_host_site_redo(scim_root_t _root)
{
	char path[PATH_MAX] = "/dev/disk/by-label/";
	scim_site_data_t site = {
	 {
		.last = 1,
		.list = {
			{path, "/vol", "btrfs", "subvolid=0"},
			{path, "/boot", "btrfs", "subvol=boot"},
			{}
		}
	 }
	};

	strncat(path, _root->task->name, 12);

	if(access(path, F_OK) != 0) {
		fprintf(stderr, "%s: access %s: %m\n", __func__, path);
		return 0;
	}

	if(scim_site_redo(site) < 0) {
		return -1;
	}

	return 0;
}

int scim_host_save(scim_root_t _root)
{
	char path[PATH_MAX];
	struct utsname utsname;

	uname(&utsname);
	snprintf(path, sizeof(path), "/boot/x%d-linux-%s-glibc-%s/", LONG_BIT, utsname.release, gnu_get_libc_version());

	if(access(path, F_OK) < 0) {
		errno = 0;
		return 0;
	}

	if(scim_lzma_pack(NULL, NULL) < 0) {
		return -1;
	}

	if(scim_cpio_data_save(NULL, NULL, NULL) < 0) {
		return -1;
	}

	return 0;
}

int scim_host_wake(scim_root_t _root)
{
	char** file;
	char*** tool;
	scim_cell_code_t cell;

	if((unsigned)(cell = gethostid()) > 3) {
		cell = 0;
	}

	_root->cells = __cells + cell;
	_root->task = __cell[cell].task;
	_root->tasks = _root->task->cell->tasks;
	_root->tend = _root->cells->size + _root->tasks->size;

	scim_tale_open(_root->task->name);

	fprintf(stderr, "%02X %s %s %s %s\n",
		_root->task->cell->code, _root->task->name,
		_root->task->cell->zone->term, _root->task->cell->role->term,
		_root->task->cell->duty->term);

	if(scim_site_redo(__host_site) < 0) {
		return -1;
	}

	for(file = __file_modprobe; *file; file++) {
		__tool_modprobe[1] = *file;
		scim_tool_call(__tool_modprobe, NULL, NULL, NULL);
	}

	for(tool = __tool_hardware; *tool; tool++) {
		scim_tool_call(*tool, NULL, NULL, NULL);
	}

	if(scim_lane_make(_root) < 0) {
		return -1;
	}

	if(scim_link_redo(_root) < 0) {
		return -1;
	}

	if(scim_port_make(_root->task) < 0) {
		return -1;
	}

	if(scim_host_site_redo(_root) < 0) {
		return -1;
	}

	return 0;
}

int scim_host_down(scim_root_t _root)
{
	int console;
	int fifo;
	size_t size;

	scim_link_note(_root);
	scim_host_save(_root);

	if(!_root->halt) {
		_root->halt = __poweroff;
	}

	if((console = open("/dev/console", O_NOCTTY|O_WRONLY)) != -1) {
		if(console != STDERR_FILENO) {
			dup2(console, STDERR_FILENO);
			close(console);
		}
	}

	for(;;) {
		if((fifo = open("/dev/initctl", O_NONBLOCK|O_WRONLY)) != -1) {
			break;
		}

		fprintf(stderr, "%s: open /dev/initctl: %m\n", __func__);

		if(errno == ENOENT) {
			kill(1, SIGUSR1);
		}

		nanosleep((struct timespec[]){{.tv_sec = 1}}, NULL);
	}

	for(size = sizeof(*_root->halt);;) {
		if(write(fifo, _root->halt, size) == size) {
			close(fifo);
			break;
		}

		fprintf(stderr, "%s: write %zu byte to /dev/initctl: %m\n", __func__, size);

		nanosleep((struct timespec[]){{.tv_sec = 1}}, NULL);
	}

	return 0;
}
