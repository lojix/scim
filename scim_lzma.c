#include "scim/scim.h"
#include <fcntl.h>
#include <gnu/libc-version.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <lzma.h>
#include <linux/fs.h>

int scim_lzma_pack(const char* _source, const char* _target)
{
	int input = -1;
	int output = -1;
	int result = -1;
	uint8_t buffer[BUFSIZ];
	void* source = NULL;
	size_t size;
	struct stat state = {};
	lzma_ret error;
	lzma_stream stream = LZMA_STREAM_INIT;

	if(!_target) {
		char* path;
		struct utsname utsname;

		uname(&utsname);
		path = alloca(snprintf(NULL, 0, "/boot/x%d-linux-%s-glibc-%s/sysdata", LONG_BIT, utsname.release, gnu_get_libc_version()));
		sprintf(path, "/boot/x%d-linux-%s-glibc-%s/sysdata", LONG_BIT, utsname.release, gnu_get_libc_version());
		_target = path;
	}

	if(!_source) {
		_source = "/dev/zram0";
	}

	if(stat(_source, &state) < 0) {
		fprintf(stdout, "%s: stat %s: %m\n", __func__, _source);
		return -1;
	}

	if((error = lzma_easy_encoder(&stream, 6, LZMA_CHECK_CRC64)) != LZMA_OK) {
		fprintf(stdout, "%s: lzma_easy_encoder: error %d\n", __func__, error);
		return -1;
	}

	if((input = open(_source, O_RDONLY)) < 0) {
		fprintf(stdout, "%s: open %s: %m\n", __func__, _source);
		goto done;
	}

	if(S_ISBLK(state.st_mode)) {
		if(ioctl(input, BLKGETSIZE64, &size) < 0) {
			fprintf(stdout, "%s: ioctl %s: %m\n", __func__, _source);
			goto done;
		}

		state.st_size = size;
	}

	if((source = mmap(NULL, state.st_size, PROT_READ, MAP_SHARED, input, 0)) == MAP_FAILED) {
		fprintf(stdout, "%s: mmap %s: %m\n", __func__, _source);
		goto done;
	}

	close(input);

	if((output = open(_target, O_TRUNC|O_CREAT|O_WRONLY, 0640)) < 0) {
		fprintf(stdout, "%s: open %s: %m\n", __func__, _target);
		goto done;
	}

	stream.next_in = source;
	stream.avail_in = state.st_size;

	for(;;) {
		stream.next_out = buffer;
		stream.avail_out = sizeof(buffer);

		switch((error = lzma_code(&stream, LZMA_FINISH))) {
			case LZMA_OK:
			case LZMA_STREAM_END:
			break;

			default:
			fprintf(stdout, "%s: lzma_code: error %d\n", __func__, error);
			goto done;
		}

		if((size = sizeof(buffer) - stream.avail_out)) {
			if(write(output, buffer, size) < 0) {
				fprintf(stdout, "%s: write %s: %m\n", __func__, _target);
				goto done;
			}
		}

		if(error == LZMA_STREAM_END) {
			break;
		}
	}

	result = 0;

	done:
	lzma_end(&stream);

	if(source) {
		munmap(source, state.st_size);
	}

	if(output != -1) {
		close(output);
	}

	return result;
}

int scim_lzma_puff(const char* _source, const char* _target)
{
	int result = -1;
	return result;
}
