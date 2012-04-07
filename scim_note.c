#include "scim/scim.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int scim_tale_open(const char* _name)
{
	char path[PATH_MAX];
	int fd;

	snprintf(path, sizeof(path), "var/log/%s", _name);

	switch((fd = open(path, O_APPEND|O_CLOEXEC|O_CREAT|O_RDWR, 0644))) {
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
