#include "scim/scim.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <gnu/libc-version.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#define _MODPROBE				ROOTSBINDIR "modprobe"
#define _UDEVD					 ROOTLIBDIR "udev/udevd"
#define _UDEVADM				ROOTSBINDIR "udevadm"

static char* __module[] = {
 "bonding",
 "dummy",
 "fuse",
 "ipv6",
 NULL
};

char* __tool_modprobe[] = {_MODPROBE, NULL, NULL};

char** __cold_plug[] = {
 (char*[]){_UDEVD, "--daemon", "--resolve-names=never", NULL},
 (char*[]){_UDEVADM, "trigger", NULL},
 (char*[]){_UDEVADM, "settle", NULL},
 (char*[]){_UDEVADM, "control", "--exit", NULL},
 NULL
};

char** __warm_plug[] = {
 (char*[]){_UDEVADM, "trigger", NULL},
 (char*[]){_UDEVADM, "settle", NULL},
 NULL
};

int scim_host_kmod_load(char** _list)
{
	for(char** file = _list; *file; file++) {
		__tool_modprobe[1] = *file;
		scim_tool_call(__tool_modprobe, NULL, NULL, NULL);
	}

	return 0;
}

int scim_host_cold_plug(void)
{
	for(char*** tool = __cold_plug; *tool; tool++) {
		scim_tool_call(*tool, NULL, NULL, NULL);
	}

	return 0;
}

int scim_host_warm_plug(void)
{
	for(char*** tool = __warm_plug; *tool; tool++) {
		scim_tool_call(*tool, NULL, NULL, NULL);
	}

	return 0;
}

int scim_host_read(scim_root_t _root, const char* _host)
{
	char* end;
	int host;

	errno = 0;
	host = strtoul(_host, &end, 10);

   	if(errno) {
		fprintf(stderr, "%s: strtoul: %m\n", __func__);
		return -1;
   	}

	if(end == _host || host > _CELL_CODE_MAX) {
		fprintf(stderr, "%s: strtoul: %u > %u or %p == %p: %s\n", __func__, host, _CELL_CODE_MAX, strerror(EINVAL), end, _host);
		return -1;
	}

	return host;
}

int scim_host_site_redo(scim_root_t _root)
{
	char path[PATH_MAX];
	char system[NAME_MAX];
	char* boot;
	char* exec_prefix;
	char* prefix;
	struct utsname utsname;
	scim_site_data_t site = {};

	snprintf(path, sizeof(path), "/dev/disk/by-label/%s", _root->task->name);

	if(access(path, F_OK) < 0) {
		fprintf(stderr, "%s: access %s: %m\n", __func__, path);
		return -1;
	}

	scim_site_put(site, "none", "/config", "configfs", NULL);
	scim_site_put(site, "none", "/dev/mqueue", "mqueue", NULL);
	scim_site_put(site, "none", "/dev/pts", "devpts", NULL);
	scim_site_put(site, "none", "/dev/shm", "tmpfs", "mode=1777");
	scim_site_put(site, "none", "/proc/sys/fs/binfmt_misc", "binfmt_misc", NULL);
	scim_site_put(site, "none", "/sys/fs/fuse/connections", "fusectl", NULL);
	scim_site_put(site, "none", "/tmp", "tmpfs", "mode=1777");
	scim_site_put(site, path, "/vol", "btrfs", "subvolid=0");

	if(scim_site_redo(site) < 0) {
		return -1;
	}

	uname(&utsname);

	snprintf(system, sizeof(system), "x%d-linux-%s-glibc-%s",
		LONG_BIT, utsname.release, gnu_get_libc_version());

	snprintf(path, sizeof(path), "/vol/os/%s/boot", system);
	boot = strdupa(path);

	snprintf(path, sizeof(path), "/vol/os/%s/usr", system);
	prefix = strdupa(path);

	snprintf(path, sizeof(path), "/vol/os/%s/usr%d", system, LONG_BIT);
	exec_prefix = strdupa(path);

	site->flag = MS_BIND;
	site->last = 0;

	scim_site_put(site, boot, "/boot", NULL, NULL);
	scim_site_put(site, prefix, PREFIX, NULL, NULL);
	scim_site_put(site, exec_prefix, EXEC_PREFIX, NULL, NULL);
	scim_site_put(site, "/vol/log", "/var/log", NULL, NULL);
	scim_site_put(site, NULL, NULL, NULL, NULL);

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
	snprintf(path, sizeof(path), "/vol/os/x%d-linux-%s-glibc-%s/boot/", LONG_BIT, utsname.release, gnu_get_libc_version());

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
	scim_cell_code_t cell;

	if((unsigned)(cell = gethostid()) > 3) {
		cell = 0;
	}

	_root->cells = __cells + cell;
	_root->task = __cell[cell].task;
	_root->tasks = _root->task->cell->tasks;
	_root->tend = _root->cells->size + _root->tasks->size;
	_root->down = _root->tend;
	_root->mark = 0x7f000000|((_root->task->cell->code & 0xff) << 16);
	_root->step = __step_wake;

	umask(0);

	if(scim_host_kmod_load(__module) < 0) {
		return -1;
	}

	if(scim_host_cold_plug() < 0) {
		return -1;
	}

	if(scim_host_site_redo(_root) < 0) {
		return -1;
	}

	if(scim_log_open(NULL) < 0) {
		return -1;
	}

	if(scim_lane_make(_root) < 0) {
		return -1;
	}

	if(scim_port_make(_root->task) < 0) {
		return -1;
	}

	if(scim_port_wake(_root) < 0) {
		return -1;
	}

	if(scim_cell_site_open(_root) < 0) {
		return -1;
	}

	if(scim_cell_make(_root) < 0) {
		return -1;
	}

	fprintf(stderr, "%02X %s %s %s %s\n",
		_root->task->cell->code, _root->task->name,
		_root->task->cell->zone->term, _root->task->cell->role->term,
		_root->task->cell->duty->term);

	return 0;
}

int scim_host_down(scim_root_t _root)
{
	int console;
	int fifo;
	size_t size;

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
