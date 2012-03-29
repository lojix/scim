#include "scim/scim.h"
#include <sys/stat.h>
#include <fcntl.h>

int scim_term_post(const char* _path, const char* _term)
{
	int file;
	int result = 0;

	if((file = open(_path, O_WRONLY)) < 0) {
		fprintf(stderr, "%s: open: %s: %m\n", __func__, _path);
		return -1;
	}

	if(write(file, _term, strlen(_term)) < 0) {
		fprintf(stderr, "%s: write %s %s: %m\n", __func__, _path, _term);
		result = -1;
	}

	int error = errno;
	close(file);
	errno = error;
	return result;
}

int scim_term_call(const char* _path, char* _term, size_t _room)
{
	int file;
	int result = 0;

	if((file = open(_path, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, _path);
		return -1;
	}

	if(read(file, _term, _room) < 0) {
		fprintf(stderr, "%s: read %s: %m\n", __func__, _path);
		result = -1;
	}

	int error = errno;
	close(file);
	errno = error;
	return result;
}

int scim_term_stow(const char* _path, const char* _term)
{
	int result = 0;
	return result;
}

int scim_term_take(const char* _path, char* _term, size_t _room)
{
	int result = 0;
	return result;
}
