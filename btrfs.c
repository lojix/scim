#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "scim/btrfs.h"

int btrfs_volume_create(const char* _target)
{
	char* prefix;
	char* target;
	int handle;
	int result = -1;
	struct btrfs_ioctl_vol_args detail = {};

	prefix = strdupa(_target);

	if((target = strrchr(prefix, '/'))) {
		*target++ = '\0';
	}
	else {
		prefix = ".";
	}

	if((handle = open(prefix, O_DIRECTORY|O_RDONLY)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, prefix);
		return -1;
	}

	strncpy(detail.name, target, BTRFS_SUBVOL_NAME_MAX);

	if(ioctl(handle, BTRFS_IOC_SUBVOL_CREATE, &detail) < 0) {
		fprintf(stderr, "%s: ioctl BTRFS_IOC_SUBVOL_CREATE: %m\n", __func__);
		goto done;
	}

	result = 0;
	done:
	close(handle);
	return result;
}

int btrfs_volume_mirror(const char* _source, const char* _target)
{
	int result = -1;
	result = 0;
	done:
	return result;
}

int btrfs_volume_remove(const char* _source)
{
	int result = -1;
	result = 0;
	done:
	return result;
}
