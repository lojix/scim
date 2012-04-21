#include "scim/scim.h"
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>

int scim_log_open(const char* _path)
{
	char name[17];
	char path[PATH_MAX];
	int fd;

	if(!_path) {
		if(prctl(PR_GET_NAME, name) < 0) {
			fprintf(stderr, "%s: prctl PR_GET_NAME: %m\n", __func__);
			return -1;
		}

		snprintf(path, sizeof(path), "var/log/%s", name);
		_path = path;
	}

	switch((fd = open(_path, O_APPEND|O_CLOEXEC|O_CREAT|O_RDWR, 0644))) {
		case -1:
		fprintf(stderr, "%s: open %s: %m\n", __func__, path);
		return -1;

		case STDERR_FILENO:
		break;

		default:
		dup2(fd, STDERR_FILENO);
		close(fd);
		break;
	}

	return 0;
}
