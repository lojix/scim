#include "scim/scim.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>

scim_site_data_t __host_link = {
 {
	.last = 4,
	.flag = MS_BIND,
	.list = {
		{"/", "/mnt/", NULL, NULL},
		{"/adm/", "/mnt/adm/", NULL, NULL},
		{ROOTLIBDIR "modules/", "/mnt" ROOTLIBDIR "modules/", NULL, NULL},
		{PREFIX, "/mnt" PREFIX, NULL, NULL},
		{EXEC_PREFIX, "/mnt" EXEC_PREFIX, NULL, NULL},
		{}
	}
 }
};

scim_site_data_t __cell_link = {
 {
	.last = 3,
	.flag = MS_BIND,
	.list = {
		{"etc/", "/mnt/etc/", NULL, NULL},
		{"root/", "/mnt/root/", NULL, NULL},
		{"srv/", "/mnt/srv/", NULL, NULL},
		{"var/", "/mnt/var/", NULL, NULL},
		{}
	}
 }
};

scim_site_data_t __host_site = {
 {
	.last = 5,
	.list = {
		{"none", "/config", "configfs", NULL},
		{"none", "/dev/mqueue", "mqueue", NULL},
		{"none", "/dev/pts", "devpts", NULL},
		{"none", "/dev/shm", "tmpfs", "mode=1777"},
		{"none", "/tmp", "tmpfs", "mode=1777"},
		{"none", "/sys/fs/fuse/connections", "fusectl", NULL},
		{}
	}
 }
};

scim_site_data_t __cell_site = {
 {
	.last = 8,
	.list = {
		{"none", "/config", "configfs", NULL},
		{"none", "/dev", "devtmpfs", NULL},
		{"none", "/dev/mqueue", "mqueue", NULL},
		{"none", "/dev/pts", "devpts", NULL},
		{"none", "/dev/shm", "tmpfs", "mode=1777"},
		{"none", "/proc", "proc", NULL},
		{"none", "/proc/sys/fs/binfmt_misc", "binfmt_misc", NULL},
		{"none", "/sys", "sysfs", NULL},
		{"none", "/tmp", "tmpfs", "mode=1777"},
		{}
	}
 }
};

int scim_site_put(scim_site_data_t _site, const char* _disk, const char* _path, const char* _type, const char* _flag)
{
	if(_site->last < _SITE_CURB_END) {
		scim_site_item_t item = *(_site->list + _site->last);

		item[_SITE_DISK] = _disk;
		item[_SITE_PATH] = _path;
		item[_SITE_TYPE] = _type;
		item[_SITE_FLAG] = _flag;

		_site->last++;

		return 0;
	}

	fprintf(stderr, "%s: %s\n", __func__, strerror(ENOBUFS));

	return -1;
}

int scim_site_get(scim_site_data_t _site, int _item, const char** _disk, const char** _path, const char** _type, const char** _flag)
{
	if(_item <= _site->last) {
		scim_site_item_t item = *(_site->list + _item);

		*_disk = item[_SITE_DISK];
		*_path = item[_SITE_PATH];
		*_type = item[_SITE_TYPE];
		*_flag = item[_SITE_FLAG];

		return 0;
	}

	fprintf(stderr, "%s: %s\n", __func__, strerror(ENOENT));

	return -1;
}

int scim_site_scan(scim_site_data_t _site, const char* _file)
{
	int file;
	ssize_t size;

	if(!_file) {
		_file = "/proc/mounts";
	}

	retry:

	if((file = open(_file, O_RDONLY)) < 0) {
		if(errno != ENOENT) {
			fprintf(stderr, "%s: open %s: %m\n", __func__, _file);
			return -1;
		}

		if(mount("none", "/proc", "proc", 0, NULL) < 0) {
			fprintf(stderr, "%s: mount /proc: %m\n", __func__);
			return -1;
		}

		goto retry;
	}

	size = read(file, _site->data, sizeof(_site->data));
	close(file);

	if(size < 0) {
		fprintf(stderr, "%s: read %s: %m\n", __func__, _file);
		return -1;
	}

	char* data = _site->data;
	scim_site_list_t site = _site->list;
	scim_site_item_t item = *site;

	for(_site->last = -1, *item = data, size = -size; size; size++, data++) {
		switch(*data) {
			case 0x0A: // Newline
			*data = '\0';
			item = *++site;
			*item = data + 1;
			_site->last++;
			break;

			case 0x20: // Space
			*data = '\0';
			*++item = data + 1;
			break;
		}
	}

	*item = NULL;

	return 0;
}

int scim_site_dump(scim_site_data_t _site)
{
	scim_site_list_t site;
	scim_site_item_t item;

	for(site = _site->list, item = *site; *item; site++, item = *site) {
		for(; *item; item++) {
			printf("%s ", *item);
		}
		printf("\n");
	}
	return 0;
}

int scim_site_pick(scim_site_data_t _pick, scim_site_data_t _pool, scim_site_code_t _fact, const char* _text, bool _bool)
{
	_pick->last = -1;

	for(scim_site_list_t pool = _pool->list, pick = _pick->list; **pool; pool++) {
		if(!strncmp(_text, (*pool)[_fact], strlen(_text)) != _bool) {
			continue;
		}

		for(scim_site_item_t item = *pool, copy = *pick; (*copy++ = *item++););

		pick++;
		_pick->last++;
	}

	return _pick->last + 1;
}

int scim_site_find(scim_site_data_t _data, scim_site_item_t* _site, scim_site_code_t _code, const char* _term)
{
	size_t size = strlen(_term);
	scim_site_list_t list = _data->list;

	if(_code == _SITE_PATH && _term[size - 1] == '/') {
		size--;
	}

	for(; **list; list++) {
		*_site = *list;

		if(!strncmp((*_site)[_code], _term, size)) {
			return true;
		}
	}

	return false;
}

int scim_site_redo(scim_site_data_t _site)
{
	scim_site_data_t data;
	scim_site_item_t info;
	scim_site_list_t site;
	scim_site_item_t item;

	scim_site_scan(data, NULL);

	for(site = _site->list, item = *site; *item; site++, item = *site) {
		if(scim_site_find(data, &info, _SITE_PATH, item[_SITE_PATH])) {
			continue;
		}

		if(mount(item[_SITE_DISK], item[_SITE_PATH], item[_SITE_TYPE], _site->flag, item[_SITE_FLAG]) < 0) {
			if(errno != EBUSY) {
				fprintf(stderr, "%s: mount %s %s: %m\n", __func__, item[_SITE_DISK], item[_SITE_PATH]);
				return -1;
			}
		}
	}

	return 0;
}

int scim_site_undo(scim_site_data_t _site)
{
	for(scim_site_list_t list = _site->list + _site->last; list >= _site->list; list--) {
		if(umount2((*list)[_SITE_PATH], 0) < 0) {
			fprintf(stderr, "%s: umount2 %s: %m\n", __func__, (*list)[_SITE_PATH]);
			return -1;
		}
	}

	return 0;
}

int scim_path_make(const char* _path, mode_t _mode)
{
	char* path;
	size_t size = strlen(_path);

	if(_path[size - 1] == '/') {
		size--;
	}

	path = alloca(size + 1);
	memcpy(path, _path, size);
	path[size++] = '/';
	path[size] = '\0';

	for(char* part = strchr(path, '/'); part;) {
		*part = '\0';

		if(mkdir(path, _mode) < 0 && errno != EEXIST) {
			fprintf(stderr, "%s: mkdir %s: %m\n", __func__, path);
			return -1;
		}

		*part++ = '/';
		part = strchr(part, '/');
	}

	return 0;
}
