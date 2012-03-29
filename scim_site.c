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
	.last = 4,
	.flag = MS_BIND,
	.list = {
		{"etc/", "/mnt/etc/", NULL, NULL},
		{"root/", "/mnt/root/", NULL, NULL},
		{"run/", "/mnt/run/", NULL, NULL},
		{"srv/", "/mnt/srv/", NULL, NULL},
		{"var/", "/mnt/var/", NULL, NULL},
		{}
	}
 }
};

scim_site_data_t __host_site = {
 {
	.last = 4,
	.list = {
		{"none", "config", "configfs", NULL},
		{"none", "dev/mqueue", "mqueue", NULL},
		{"none", "dev/pts", "devpts", NULL},
		{"none", "dev/shm", "tmpfs", "mode=1777"},
		{"none", "tmp", "tmpfs", "mode=1777"},
		{}
	}
 }
};

scim_site_data_t __cell_site = {
 {
	.last = 8,
	.list = {
		{"none", "config", "configfs", NULL},
		{"none", "dev", "devtmpfs", NULL},
		{"none", "dev/mqueue", "mqueue", NULL},
		{"none", "dev/pts", "devpts", NULL},
		{"none", "dev/shm", "tmpfs", "mode=1777"},
		{"none", "proc", "proc", NULL},
		{"none", "proc/sys/fs/binfmt_misc", "binfmt_misc", NULL},
		{"none", "sys", "sysfs", NULL},
		{"none", "tmp", "tmpfs", "mode=1777"},
		{}
	}
 }
};

int scim_site_scan(scim_site_data_t _site, const char* _file)
{
	int file;
	ssize_t size;

	if(!_file) {
		_file = "/proc/mounts";
	}

	if((file = open(_file, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: fopen %s: %m\n", __func__, _file);
		return -1;
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

int scim_site_redo(scim_site_data_t _site)
{
	scim_site_list_t site;
	scim_site_item_t item;

	for(site = _site->list, item = *site; *item; site++, item = *site) {
		if(mount(item[_SITE_DISK], item[_SITE_PATH], item[_SITE_TYPE], _site->flag, item[_SITE_FLAG]) < 0) {
			if(errno != EBUSY) {
				fprintf(stderr, "%s: mount %s: %m\n", __func__, item[_SITE_PATH]);
				return -1;
			}
		}
	}

	return 0;
}

int scim_site_undo(scim_site_data_t _site)
{
	for(int last = _site->last; last > 0; last--) {
		if(umount2(_site->list[last][_SITE_PATH], 0) < 0) {
			fprintf(stderr, "%s: umount2 %s: %m\n", __func__, _site->list[last][_SITE_PATH]);
			return -1;
		}
	}

	return 0;
}
/*
int scim_site_link(const char* _site, const char* _base, const char** _path)
{
	char source[PATH_MAX];
	char target[PATH_MAX];

	if(!_base) {
		_base = "";
	}

	for(; *_path; _path++) {
		snprintf(source, sizeof(source), "%s%s", _base, *_path);
		snprintf(target, sizeof(target), "%s%s", _site, *_path);

		if(access(source, F_OK) < 0) {
			if(errno == ENOENT) {
				continue;
			}

			fprintf(stderr, "%s: access: %s: %m\n", __func__, source);
			return -1;
		}

		if(access(target, F_OK) < 0) {
			for(char* part = strchr(&target[1], '/'); part;) {
				*part = '\0';

				if(mkdir(target, 0755) < 0 && errno != EEXIST) {
					fprintf(stderr, "%s: mkdir %s: %m\n", __func__, target);
					printf("%s: mkdir %s: %m\n", __func__, target);
					return -1;
				}

				*part++ = '/';
				part = strchr(part, '/');
			}
		}

		if(mount(source, target, NULL, MS_BIND, NULL) < 0) {
			fprintf(stderr, "%s: mount: %s %s: %m\n", __func__, source, target);
			continue;
		}
	}

	return 0;
}*/
