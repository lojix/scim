#include "scim/scim.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

/*static int scim_turf_prep(scim_root_t _root)
{
	char* path[] = {(char [PATH_MAX]){}, (char [PATH_MAX]){}, NULL};

	snprintf(path[0], PATH_MAX, "%s%s/", SCIM_DATA_PATH, _root->task->host->name);
	snprintf(path[1], PATH_MAX, "%s%s/", SCIM_CELL_PATH, _root->task->host->name);

	for(int i = 0; path[i]; i++) {
		for(char* part = strchr(&path[i][1], '/'); part;) {
			*part = '\0';

			if(mkdir(path[i], 0755) < 0 && errno != EEXIST) {
				fprintf(stderr, "%s: mkdir %s: %m\n", __func__, path[i]);
				return -1;
			}

			*part++ = '/';
			part = strchr(part, '/');
		}
	}

	return 0;
}*/

int scim_tool_call(char* _tool[], char* _environ[], const char* _input, const char* _output)
{
	int result = -1;
	pid_t pid;
	siginfo_t status[1] = {};

	if((pid = fork()) < 0) {
		fprintf(stderr, "%s: vfork: %m\n", __func__);
		return -1;
	}

	if(pid == 0) {
		sigset_t mask;
		sigfillset(&mask);
		sigprocmask(SIG_UNBLOCK, &mask, NULL);

		if(_input) {
			int input;
			int output = 0;

			if(open(_input, O_RDONLY) < 0) {
				_exit(EXIT_SUCCESS);
			}

			if(pipe(&input) < 0) {
				fprintf(stderr, "%s: pipe %s: %m\n", __func__, _input);
				_exit(EXIT_FAILURE);
			}

			dup2(STDIN_FILENO, input);
			close(input);
			close(output);
		}

		if(_output) {
			int input;
			int output = 0;

			if(open(_output, O_CREAT|O_TRUNC|O_WRONLY, 0640) < 0) {
				fprintf(stderr, "%s: open: %s: %m\n", __func__, _output);
				_exit(EXIT_FAILURE);
			}

			if(pipe(&input) < 0) {
				fprintf(stderr, "%s: pipe %s: %m\n", __func__, _input);
				_exit(EXIT_FAILURE);
			}

			dup2(output, STDOUT_FILENO);
			close(input);
			close(output);
		}

		execve(*_tool, _tool, _environ);
		fprintf(stderr, "%s: execve: %s: %m\n", __func__, *_tool);
		_exit(EXIT_FAILURE);
	}

	while(waitid(P_PID, pid, status, WEXITED) < 0) {
		if(errno != EINTR) {
			fprintf(stderr, "%s: waitid: %m\n", __func__);
			return -1;
		}
	}

	switch(status->si_code) {
		case CLD_EXITED:
		if(status->si_status == EXIT_SUCCESS) {
			result = 0;
		}
		break;

		case CLD_KILLED:
		fprintf(stderr, "%s: waitid: kill signal: %s\n", __func__, sys_siglist[status->si_status]);
		break;
	}

	return result;
}
