#include "scim/scim.h"
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/loop.h>

int scim_loop_find(int* _unit)
{
	int file;
	int result = -1;

	if((file = open("/dev/loop-control", O_RDWR)) < 0) {
		fprintf(stderr, "%s: open /dev/loop-control: %m\n", __func__);
		return -1;
	}

	if((*_unit = ioctl(file, LOOP_CTL_GET_FREE)) < 0) {
		fprintf(stderr, "%s: ioctl LOOP_CTL_GET_FREE: %m\n", __func__);
		goto done;
	}

	result = 0;
	done:
	close(file);
	return result;
}

int scim_loop_bind(const char* _loop, const char* _file, bool _writeable)
{
	int file = -1;
	int flag;
	int loop = -1;
	int result = -1;
	struct loop_info64 info = {};

	flag = _writeable? O_RDWR: O_RDONLY;

	if((file = open(_file, flag)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _file);
		return -1;
	}

	if((loop = open(_loop, flag)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _loop);
		goto done;
	}

	if(ioctl(loop, LOOP_SET_FD, file) < 0) {
		fprintf(stderr, "%s: ioctl LOOP_SET_FD: %m\n", __func__);
		goto done;
	}

	info.lo_flags = LO_FLAGS_PARTSCAN;

	if(!_writeable) {
		info.lo_flags |= LO_FLAGS_READ_ONLY;
	}

	if(ioctl(loop, LOOP_SET_STATUS64, &info) < 0) {
		fprintf(stderr, "%s: ioctl LOOP_SET_STATUS64: %m\n", __func__);
		goto done;
	}

	result = 0;

	done:
	if(file != -1) {
		close(file);
	}

	if(loop != -1) {
		close(loop);
	}

	return result;
}

int scim_loop_free(const char* _loop)
{
	int loop;
	int result = -1;

	if((loop = open(_loop, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _loop);
		return -1;
	}

	if(ioctl(loop, LOOP_CLR_FD, 0) < 0) {
		fprintf(stderr, "%s: ioctl LOOP_CLR_FD: %m\n", __func__);
		goto done;
	}

	result = 0;
	done:
	close(loop);
	return result;
}
