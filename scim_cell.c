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

int scim_cell_copy(scim_task_t _task, char* _archive)
{
	char path[PATH_MAX];

	if(mkdir(SCIM_CELL_PATH, 0755) < 0 && errno != EEXIST) {
		fprintf(stderr, "%s: mkdir: %s: %m\n", __func__, SCIM_CELL_PATH);
		return -1;
	}

	snprintf(path, sizeof(path), "%s/%s", SCIM_CELL_PATH, _task->name);

	if(mkdir(path, 0755) < 0) {
		if(errno == EEXIST) {
			return 0;
		}

		fprintf(stderr, "%s: mkdir: %s: %m\n", __func__, path);
		return -1;
	}

	if(!_archive) {
		_archive = CELL_ARCHIVE_PATH;
	}

	char* argument[] = {"bash", "-c", NULL, NULL};

	argument[2] = alloca(snprintf(NULL, 0, "xzcat %s | cpio -idm", _archive));
	sprintf(argument[2], "xzcat %s | cpio -idm 2> /dev/null", _archive);

	pid_t pid = fork();

	if(pid < 0) {
		fprintf(stderr, "%s: fork: %m\n", __func__);
		return -1;
	}

	/*
	* Parent.
	*/
	if(pid) {
		int status;
		if((pid = wait(&status)) < 0) {
			fprintf(stderr, "%s: wait: %m\n", __func__);
			return -1;
		}

		if(WIFEXITED(status)) {
			if(WEXITSTATUS(status) != EXIT_SUCCESS) {
				fprintf(stderr, "%s: filesystem setup failed\n", __func__);
				return -1;
			}
		}

		return 0;
	}

	/*
	* Child.
	*/
	if(chdir(path) < 0) {
		fprintf(stderr, "%s: chdir: %m\n", __func__);
		return -1;
	}

	execve("/bin/bash", argument, environ);
	fprintf(stderr, "%s: execve: %m\n", __func__);
	_exit(EXIT_FAILURE);
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

	if(scim_cell_copy(_task, NULL) < 0) {
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
