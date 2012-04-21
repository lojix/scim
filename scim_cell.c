#include "scim/scim.h"
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <ocfs2/ocfs2.h>
#include <ftw.h>
#include <utime.h>
#include <uuid/uuid.h>
#include <attr/xattr.h>

typedef struct scim_root_item_t {
 const char* name;
 mode_t mode;
}* scim_root_item_t;

struct scim_root_item_t __root[] = {
 {"boot", 0755},
 {"bin", 0755},
 {"config", 0755},
 {"dev", 0755},
 {"etc", 0755},
 {"home", 0755},
 {"lib", 0755},
 {"lib32", 0755},
 {"lib64", 0755},
 {"media", 0755},
 {"mnt", 0755},
 {"proc", 0755},
 {"root", 0755},
 {"run", 0755},
 {"sbin", 0755},
 {"srv", 0755},
 {"sys", 0755},
 {"tmp", 01777},
 {"usr", 0755},
 {"usr32", 0755},
 {"usr64", 0755},
 {"var", 0755},
 {}
};

const char* __replica[] = {
 "alpha",
 "beta",
 "gamma",
 "delta",
 NULL
};

int scim_ocfs2_reflink(const char* _host, const char* _cell)
{
	int file;
	int result = -1;
	struct reflink_arguments reflink;

	reflink.old_path = (__u64)_host;
	reflink.new_path = (__u64)_cell;
	reflink.preserve = 1;

	if((file = open(_host, O_RDWR)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _host);
		return -1;
	}

	errno = 0;

	if(ioctl(file, OCFS2_IOC_REFLINK, &reflink) < 0) {
		fprintf(stderr, "%s: ioctl %s: %m\n", __func__, _cell);
		goto done;
	}

	result = 0;

	done:
	close(file);
	return result;
}

int scim_glusterfs_root_init(const char* _path)
{
	uuid_t uuid;
	char name[NAME_MAX];

	for(const char** replica = __replica; *replica; replica++) {
		snprintf(name, sizeof(name), "trusted.afr.%s", *replica);

		if(lsetxattr(_path, name, (char[12]){}, 12, 0) < 0) {
			fprintf(stderr, "%s: lsetxattr %s %s: %m\n", __func__, _path, name);
			return -1;
		}
	}

	if(uuid_parse("00000000-0000-0000-0000-000000000001", uuid) < 0) {
		fprintf(stderr, "%s: uuid_parse failed!\n", __func__);
		return -1;
	}

	if(lsetxattr(_path, "trusted.gfid", uuid, 16, 0) < 0) {
		fprintf(stderr, "%s: lsetxattr %s trusted.gfid: %m\n", __func__, _path);
		return -1;
	}

	if(lsetxattr(_path, "trusted.glusterfs.test", "working", 8, 0) < 0) {
		fprintf(stderr, "%s: lsetxattr %s trusted.glusterfs.test: %m\n", __func__, _path);
		return -1;
	}

	return 0;
}

int scim_glusterfs_entry_init(const char* _path)
{
	uuid_t uuid;
	char name[NAME_MAX];

	for(const char** replica = __replica; *replica; replica++) {
		snprintf(name, sizeof(name), "trusted.afr.%s", *replica);

		if(lsetxattr(_path, name, (char[12]){}, 12, 0) < 0) {
			fprintf(stderr, "%s: lsetxattr %s %s: %m\n", __func__, _path, name);
			return -1;
		}
	}

	uuid_generate(uuid);

	if(lsetxattr(_path, "trusted.gfid", uuid, 16, 0) < 0) {
		fprintf(stderr, "%s: lsetxattr %s trusted.gfid: %m\n", __func__, _path);
		return -1;
	}

	return 0;
}

int scim_file_copy(const char* _host, const char* _cell)
{
	int file;
	int result = -1;
	void* host = NULL;
	struct stat info = {};

	if(stat(_host, &info) < 0) {
		fprintf(stderr, "%s: fstat %s: %m\n", __func__, _host);
		return -1;
	}

	if((file = open(_host, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _host);
		return -1;
	}

	if(info.st_size) {
		if((host = mmap(NULL, info.st_size, PROT_READ, MAP_SHARED, file, 0)) == MAP_FAILED) {
			fprintf(stderr, "%s: mmap %s: %m\n", __func__, _host);
		}
	}

	close(file);

	if(info.st_size && !host) {
		return -1;
	}

	if((file = open(_cell, O_CREAT|O_WRONLY, info.st_mode)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _cell);
		goto done;
	}

	if(info.st_size) {
		if(write(file, host, info.st_size) < 0) {
			fprintf(stderr, "%s: write %s: %m\n", __func__, _cell);
			goto done;
		}
	}

	result = 0;

	done:
	if(file != -1) {
		close(file);
	}

	munmap(host, info.st_size);
	return result;
}

int scim_path_copy(const char* _host, const char* _cell, bool _glusterfs)
{
	char host[PATH_MAX];
	char cell[PATH_MAX];
	char path[PATH_MAX];
	int result = -1;
	DIR* list;
	ssize_t size;
	struct dirent* item;
	struct stat state;
	struct utimbuf utimbuf;

	if((list = opendir(_host)) == NULL) {
		fprintf(stderr, "%s: opendir %s: %m\n", __func__, _host);
		return -1;
	}

	while((item = readdir(list))) {
		if(!strcmp(".", item->d_name) || !strcmp("..", item->d_name)) {
			continue;
		}

		snprintf(host, sizeof(host), "%s/%s", _host, item->d_name);
		snprintf(cell, sizeof(cell), "%s/%s", _cell, item->d_name);

		switch(item->d_type) {
			case DT_REG:
			if(!access(cell, F_OK)) {
				break;
			}

			if(scim_file_copy(host, cell) < 0) {
				goto done;
			}
			break;

			case DT_DIR:
			if(stat(host, &state) < 0) {
				fprintf(stderr, "%s: stat %s: %m\n", __func__, host);
				goto done;
			}

			if(mkdir(cell, state.st_mode) < 0) {
				if(errno != EEXIST) {
					fprintf(stderr, "%s: mkdir %s: %m\n", __func__, cell);
					goto done;
				}
			}

			if(scim_path_copy(host, cell, _glusterfs) < 0) {
				goto done;
			}

			utimbuf.actime = state.st_atime;
			utimbuf.modtime = state.st_mtime;

			utime(cell, &utimbuf);
			break;

			case DT_LNK:
			if((size = readlink(host, path, sizeof(path))) < 0) {
				fprintf(stderr, "%s: readlink %s: %m\n", __func__, host);
				goto done;
			}

			path[size] = '\0';

			if(symlink(path, cell) < 0) {
				if(errno != EEXIST) {
					fprintf(stderr, "%s: link %s: %m\n", __func__, cell);
					goto done;
				}
			}
			break;

			default:
			break;
		}

		if(_glusterfs && scim_glusterfs_entry_init(cell) < 0) {
			goto done;
		}
	}

	result = 0;

	done:
	closedir(list);
	return result;
}

int scim_glusterfs_make(const char* _source, const char* _target)
{
	int result = -1;

	if(mkdir(_target, 0755) < 0 && errno != EEXIST) {
		fprintf(stderr, "%s: mkdir %s: %m\n", __func__, _target);
		return -1;
	}

	if(scim_glusterfs_entry_init(_target) < 0) {
		goto done;
	}

	if(scim_path_copy(_source, _target, true) < 0) {
		goto done;
	}

	result = 0;
	done:
	return result;
}

int scim_cell_copy(const char* _cell)
{
	char host[PATH_MAX];
	struct stat state;
	struct utimbuf utimbuf;

	snprintf(host, sizeof(host), "/lib/cell");
	
	if(stat(host, &state) < 0) {
		fprintf(stderr, "%s: stat %s: %m\n", __func__, host);
		return -1;
	}

	if(mkdir(_cell, state.st_mode) < 0) {
		if(errno != EEXIST) {
			fprintf(stderr, "%s: mkdir %s: %m\n", __func__, _cell);
			return -1;
		}
	}

	utimbuf.actime = state.st_atime;
	utimbuf.modtime = state.st_mtime;

	if(utime(_cell, &utimbuf) < 0) {
		fprintf(stderr, "%s utime %s: %m\n", __func__, _cell);
		return -1;
	}

	if(scim_path_copy(host, _cell, false) < 0) {
		return -1;
	}

	return 0;
}

int scim_cell_root_make(const char* _path)
{
	char path[PATH_MAX];
	int result = -1;

	if(mkdir(_path, 0755) < 0 && errno != EEXIST) {
		fprintf(stderr, "%s: mkdir %s\n", __func__, _path);
		return -1;
	}

	for(scim_root_item_t item = __root; item->name; item++) {
		snprintf(path, sizeof(path), "%s/%s", _path, item->name);

		if(mkdir(path, item->mode) < 0 && errno != EEXIST) {
			fprintf(stderr, "%s: mkdir %s: %m\n", __func__, path);
			goto done;
		}
	}

	result = 0;
	done:
	return result;
}

int scim_cell_site_open(scim_root_t _root)
{
	scim_task_t glusterfs = __task + _TASK_GLUSTERFS_CELL;
	scim_task_t glusterfsd = __task + _TASK_GLUSTERFSD_CELL;

	if(scim_task_wake_sure(_root, glusterfsd) < 0) {
		return -1;
	}

	if(scim_task_wake_sure(_root, glusterfs) < 0) {
		kill(glusterfsd->stat->pawn, SIGTERM);
		waitpid(glusterfsd->stat->pawn, NULL, 0);
		return -1;
	}

	return 0;
}

int scim_cell_make(scim_root_t _root)
{
	char path[PATH_MAX];
	int result = -1;

	for(scim_task_t* task = _root->cells->task + 1; *task; task++) {
		snprintf(path, sizeof(path), "%s%s", SCIM_CELL_PATH, (*task)->name);

		if(access(path, F_OK) == 0) {
			continue;
		}

		if(scim_cell_root_make(path) < 0) {
			goto done;
		}

		if(scim_cell_copy(path) < 0) {
			goto done;
		}
	}

	result = 0;
	done:
	return result;
}

int scim_cell_open(scim_root_t _root)
{
	char path[PATH_MAX];
	scim_site_data_t site = {};

	snprintf(path, sizeof(path), "%s%s", SCIM_CELL_PATH, _root->task->name);

	site->flag = MS_BIND;
	site->last = 0;

	scim_site_put(site, path, "/mnt", NULL, NULL);
	scim_site_put(site, "/bin", "/mnt/bin", NULL, NULL);
	scim_site_put(site, "/lib", "/mnt/lib", NULL, NULL);
	scim_site_put(site, ROOTLIBDIR, "/mnt" ROOTLIBDIR, NULL, NULL);
	scim_site_put(site, "/sbin", "/mnt/sbin", NULL, NULL);
	scim_site_put(site, PREFIX, "/mnt" PREFIX, NULL, NULL);
	scim_site_put(site, EXEC_PREFIX, "/mnt" EXEC_PREFIX, NULL, NULL);
	scim_site_put(site, NULL, NULL, NULL, NULL);

	if(scim_site_redo(site) < 0) {
		return -1;
	}

	site->flag = 0;
	site->last = 0;

	scim_site_put(site, "none", "/mnt/config", "configfs", NULL);
	scim_site_put(site, "none", "/mnt/dev", "devtmpfs", NULL);
	scim_site_put(site, "none", "/mnt/dev/mqueue", "mqueue", NULL);
	scim_site_put(site, "none", "/mnt/dev/pts", "devpts", NULL);
	scim_site_put(site, "none", "/mnt/dev/shm", "tmpfs", "mode=1777");
	scim_site_put(site, "none", "/mnt/proc", "proc", NULL);
	scim_site_put(site, "none", "/mnt/proc/sys/fs/binfmt_misc", "binfmt_misc", NULL);
	scim_site_put(site, "none", "/mnt/sys", "sysfs", NULL);
	scim_site_put(site, "none", "/mnt/tmp", "tmpfs", "mode=1777");
	scim_site_put(site, NULL, NULL, NULL, NULL);

	if(scim_site_redo(site) < 0) {
		return -1;
	}

	return 0;
}

int scim_cell_move(void)
{
	if(chdir("/mnt") < 0) {
		fprintf(stderr, "%s: chdir /mnt: %m\n", __func__);
		return -1;
	}

	if(syscall(SYS_pivot_root, ".", "mnt") < 0) {
		fprintf(stderr, "%s: pivot_root . mnt: %m\n", __func__);
		return -1;
	}

	if(chdir("/") < 0) {
		fprintf(stderr, "%s: chdir /: %m\n", __func__);
		return -1;
	}

	if(chroot(".") < 0) {
		fprintf(stderr, "%s: chroot .: %m\n", __func__);
		return -1;
	}

	return 0;
}

int scim_cell_free(scim_root_t _root)
{
	char path[PATH_MAX];
	int fd;
	scim_site_data_t site = {};
	scim_site_data_t list = {};

	snprintf(path, sizeof(path), "/var/log/%s", _root->task->name);

	scim_log_open(path);

	for(fd = getdtablesize(); fd > 2; fd--) {
		close(fd);
	}

	if(scim_site_scan(site, NULL) < 0) {
		return -1;
	}

	if(scim_site_pick(list, site, _SITE_PATH, "/mnt", true) < 0) {
		return -1;
	}

	if(scim_site_undo(list) < 0) {
		return -1;
	}

	return 0;
}

int scim_cell_wake(scim_root_t _root, scim_task_t _task)
{
	int result = -1;
	int status;

	_root->task = _task;
	_root->tasks = _task->cell->tasks;
	_root->tend = _task->cell->tasks->size;
	_root->down = _root->tend;
	_root->mark = 0x7f000000|((_task->cell->code & 0xff) << 16);
	_root->step = __step_wake;

	if(_task->stat->hear != STDIN_FILENO) {
		dup2(_task->stat->hear, STDIN_FILENO);
		close(_task->stat->hear);
	}

	if(_task->stat->tell != STDOUT_FILENO) {
		dup2(_task->stat->tell, STDOUT_FILENO);
		close(_task->stat->tell);
	}

	if(read(STDIN_FILENO, &status, sizeof(status)) < 0) {
		fprintf(stderr, "%s: read: %m\n", __func__);
		goto done;
	}

	if(!status) {
		errno = ECANCELED;
		goto done;
	}

	if(scim_cell_open(_root) < 0) {
		goto done;
	}

	if(scim_cell_move() < 0) {
		goto done;
	}

	if(scim_cell_free(_root) < 0) {
		goto done;
	}

	if(scim_port_wake(_root) < 0) {
		goto done;
	}

	for(scim_task_t* task = _root->tasks->task; *task; task++) {
		scim_task_wipe(*task);
	}

	result = 0;

	done:
	status = errno;
	write(STDOUT_FILENO, &status, sizeof(status));
	return result;
}
