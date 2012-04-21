#include "scim/scim.h"
#include <fcntl.h>
#include <gnu/libc-version.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <archive.h>
#include <archive_entry.h>

typedef struct scim_cpio_file_t {
 int file;
 const char* name;
}* scim_cpio_file_t;

static int scim_cpio_open(struct archive* _archive, void* _info)
{
	scim_cpio_file_t info = _info;

	if((info->file = open(info->name, O_CREAT|O_TRUNC|O_WRONLY, 0640)) < 0) {
		return ARCHIVE_FATAL;
	}

	return ARCHIVE_OK;
}

static ssize_t scim_cpio_copy(struct archive* _archive, void* _info, const void* _data, size_t _size)
{
	scim_cpio_file_t info = _info;
	return write(info->file, _data, _size);
}

static int scim_cpio_shut(struct archive* _archive, void* _info)
{
	scim_cpio_file_t info = _info;

	if(info->file != -1) {
		close(info->file);
	}

	return ARCHIVE_OK;
}

int scim_cpio_data_save(const char* _source, const char* _target, const char* _archive)
{
	int file;
	int result = -1;
	char* system;
	void* data = NULL;
	struct stat info = {};
	struct utsname utsname;
	struct archive* source = NULL;
	struct archive* target = NULL;
	struct archive_entry* object = NULL;
	struct scim_cpio_file_t state = {.file = -1, .name = NULL};

	uname(&utsname);
	system = alloca(snprintf(NULL, 0, "x%d-linux-%s-glibc-%s", LONG_BIT, utsname.release, gnu_get_libc_version()));
	sprintf(system, "x%d-linux-%s-glibc-%s", LONG_BIT, utsname.release, gnu_get_libc_version());

	if(!_source) {
		char* path = alloca(snprintf(NULL, 0, "/vol/os/%s/boot/sysdata", system));
		sprintf(path, "/vol/os/%s/boot/sysdata", system);
		_source = path;
	}

	if(!_target) {
		_target = "boot/sysdata";
	}

	if(!_archive) {
		char host[HOST_NAME_MAX + 1];
		char* path;

		gethostname(host, sizeof(host));
		path = alloca(snprintf(NULL, 0, "/vol/os/%s/boot/%s", system, host));
		sprintf(path,  "/vol/os/%s/boot/%s", system, host);
		_archive = path;
	}

	state.name = _archive;

	if(stat(_source, &info) < 0) {
		fprintf(stdout, "%s: stat %s: %m\n", __func__, _source);
		return -1;
	}

	if((file = open(_source, O_RDONLY)) < 0) {
		fprintf(stdout, "%s: open %s: %m\n", __func__, _source);
		return -1;
	}

	if((data = mmap(NULL, info.st_size, PROT_READ, MAP_SHARED, file, 0)) == MAP_FAILED) {
		fprintf(stdout, "%s: mmap %s: %m\n", __func__, _source);
		goto done;
	}

	if((source = archive_read_disk_new()) == NULL) {
		fprintf(stdout, "%s: archive_read_disk_new: failed!\n", __func__);
		goto done;
	}

	if(archive_read_disk_set_standard_lookup(source) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_read_disk_set_standard_lookup: %s\n", __func__,
			archive_error_string(source));
		goto done;
	}

	if((object = archive_entry_new()) == NULL) {
		fprintf(stdout, "%s: archive_entry_new: failed!\n", __func__);
		goto done;
	}

	archive_entry_copy_sourcepath(object, _source);
	archive_entry_copy_pathname(object, _target);

	if(archive_read_disk_entry_from_file(source, object, file, &info) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_read_disk_entry_from_file %s: %s\n", __func__, _source,
			archive_error_string(source));
		goto done;
	}

	if((target = archive_write_new()) == NULL) {
		fprintf(stdout, "%s: archive_write_new: failed!\n", __func__);
		goto done;
	}

	if(archive_write_set_format_cpio_newc(target) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_write_set_format_cpio_newc: %s\n", __func__,
			archive_error_string(target));
		goto done;
	}

	if(archive_write_set_compression_none(target) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_write_set_compression_none: %s\n", __func__,
			archive_error_string(target));
		goto done;
	}

	if(archive_write_open(target, &state, scim_cpio_open, scim_cpio_copy, scim_cpio_shut) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_write_open: %s\n", __func__, archive_error_string(target));
		goto done;
	}

	if(archive_write_header(target, object) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_write_header: %s\n", __func__, archive_error_string(target));
		goto done;
	}

	if(archive_write_data(target, data, info.st_size) < 0) {
		fprintf(stdout, "%s: archive_write_data: %s\n", __func__, archive_error_string(target));
		goto done;
	}

	if(archive_write_close(target) != ARCHIVE_OK) {
		fprintf(stdout, "%s: archive_write_close: %s\n", __func__, archive_error_string(target));
		goto done;
	}

	result = 0;

	done:
	close(file);
	unlink(_source);

	if(data) {
		munmap(data, info.st_size);
	}

	if(object) {
		archive_entry_free(object);
	}

	if(source) {
		archive_read_free(source);
	}

	if(target) {
		archive_write_free(target);
	}

	return result;
}
