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
#include <ocfs2/ocfs2.h>
#include <ftw.h>
#include <utime.h>

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

int scim_path_copy(const char* _host, const char* _cell)
{
	char host[PATH_MAX];
	char cell[PATH_MAX];
	int result = -1;
	DIR* list;
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

			if(scim_path_copy(host, cell) < 0) {
				goto done;
			}

			utimbuf.actime = state.st_atime;
			utimbuf.modtime = state.st_mtime;

			utime(cell, &utimbuf);
			break;

			case DT_LNK:
			if(link(host, cell) < 0) {
				if(errno != EEXIST) {
					fprintf(stderr, "%s: link %s: %m\n", __func__, cell);
					goto done;
				}
			}
			break;

			default:
			break;
		}
	}

	result = 0;

	done:
	closedir(list);
	return result;
}

int scim_cell_copy(scim_task_t _task)
{
	char host[PATH_MAX];
	char cell[PATH_MAX];
	struct stat state;
	struct utimbuf utimbuf;

	snprintf(host, sizeof(host), "%slocalhost", SCIM_CELL_PATH);
	snprintf(cell, sizeof(cell), "%s%s", SCIM_CELL_PATH, _task->name);

	if(stat(host, &state) < 0) {
		fprintf(stderr, "%s: stat %s: %m\n", __func__, host);
		return -1;
	}

	if(mkdir(cell, state.st_mode) < 0) {
		if(errno != EEXIST) {
			fprintf(stderr, "%s: mkdir %s: %m\n", __func__, cell);
			return -1;
		}
	}

	utimbuf.actime = state.st_atime;
	utimbuf.modtime = state.st_mtime;

	if(utime(cell, &utimbuf) < 0) {
		fprintf(stderr, "%s utime %s: %m\n", __func__, cell);
		return -1;
	}

	if(scim_path_copy(host, cell) < 0) {
		return -1;
	}

	return 0;
}

int scim_cell_move(const char* _path, const char* _name)
{
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s%s/", SCIM_CELL_PATH, _name);

	if(chdir(path) < 0) {
		fprintf(stderr, "%s: chdir %s: %m\n", __func__, path);
		return -1;
	}

	if(scim_site_redo(__host_link) < 0) {
		return -1;
	}

	if(scim_site_redo(__cell_link) < 0) {
		return -1;
	}

	if(chdir(_path) < 0) {
		fprintf(stderr, "%s: chdir %s: %m\n", __func__, _path);
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

int scim_cell_make(scim_root_t _root, scim_task_t _task, const char* _path)
{
	int result = -1;
	int status;

	if(!_path) {
		_path = "/mnt";
	}

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

	if(scim_cell_copy(_task) < 0) {
		goto done;
	}

	if(scim_cell_move(_path, _task->name) < 0) {
		goto done;
	}

	if(scim_site_redo(__cell_site) < 0) {
		goto done;
	}

	if(scim_tale_open(_task->name) < 0) {
		goto done;
	}

	int error = errno;

	for(int slot = getdtablesize(); slot > 2; slot--) {
		close(slot);
	}

	errno = error;

	_root->task = _task;
	_root->tasks = _task->cell->tasks;
	_root->tend = _task->cell->tasks->size;

	for(scim_task_t* task = _root->tasks->task; *task; task++) {
		scim_task_wipe(*task);
	}

	result = 0;

	done:
	status = errno;
	write(STDOUT_FILENO, &status, sizeof(status));
	return result;
}
