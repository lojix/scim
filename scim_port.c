#include "scim/scim.h"
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_link.h>
#include "scim/netlink.h"

int scim_port_flag(scim_port_data_t _port, bool _rise, uint32_t _flag)
{
	char term[12];
	char path[PATH_MAX];
	uint32_t flag = 0U;

	if(!sscanf(_port->info[_PORT_FLAGS], "0x%u", &flag)) {
		errno = EINVAL;
		fprintf(stderr, "%s: port->info[_PORT_FLAGS]: %m\n", __func__);
		return -1;
	}

	if(_rise == true) {
		flag |= _flag;
	}
	else {
		flag &= ~_flag;
	}

	scim_port_t fact = __port + _PORT_FLAGS;
	snprintf(term, sizeof(term), "0x%0x", flag);
	snprintf(path, sizeof(path), "/sys/class/net/%s/%s", _port->info[_PORT_NAME], fact->term);

	if(scim_term_post(path, term) < 0) {
		return -1;
	}

	return 0;
}

int scim_port_wipe(scim_port_data_t _port)
{
	memset(_port->list, 0, sizeof(_port->list));
	memset(_port->heap, 0, sizeof(_port->heap));

	_port->data = _port->heap;
	_port->room = sizeof(_port->heap);
	_port->size = 0;

	return 0;
}

int scim_port_head(scim_port_data_t _port)
{
	_port->fact = __port;
	_port->last = -1;
	_port->item = _port->list;
	_port->info = _port->item[0];

	return 0;
}

int scim_port_read(scim_port_data_t _port, const char* _name)
{
	int fail;
	int file;
	int info = 1;
	int result = -1;
	char path[48];
	ssize_t size;

	if(++_port->last >= _PORT_CURB_MAX) {
		errno = ENOMEM;
		fprintf(stderr, "%s: _port->last %d: %m\n", __func__, _port->last);
		return -1;
	}

	if((size = strlen(_name) + 1) > _port->room) {
		errno = ENOMEM;
		fprintf(stderr, "%s: _port->room %zu: %m\n", __func__, _port->room);
		return -1;
	}

	strncpy(_port->data, _name, _port->room);

	_port->item = _port->list + _port->last;
	_port->info = _port->item[0];
	_port->info[0] = _port->data;
	_port->data += size;
	_port->size += size;
	_port->room -= size;
	_port->fact = __port + 1;

	for(; *_port->fact->term; _port->fact++) {
		snprintf(path, sizeof(path), "/sys/class/net/%s/%s", _port->info[_PORT_NAME], _port->fact->term);

		if((file = open(path, O_RDONLY)) < 0) {
			fprintf(stderr, "%s: open %s: %m\n", __func__, path);
			goto done;
		}

		size = read(file, _port->data, _port->room);
		fail = errno;
		close(file);

		if(size < 0) {
			fprintf(stderr, "%s: read %s: %m\n", __func__, path);
			errno = fail;
			goto done;
		}

		_port->info[info++] = _port->data;
		_port->data[size - 1] = '\0';
		_port->data += size;
		_port->size += size;
		_port->room -= size;
	}

	result = 0;

	done:
	return result;
}

int scim_port_list(scim_port_data_t _port)
{
	int base;
	int result = -1;
	DIR* list;
	struct dirent* item;

	retry:

	if((list = opendir("/sys/class/net/")) == NULL) {
		if(errno != ENOENT) {
			fprintf(stderr, "%s: opendir /sys/class/net/: %m\n", __func__);
			return -1;
		}

		if(mount("none", "/sys", "sysfs", 0, NULL) < 0) {
			fprintf(stderr, "%s: mount /sys: %m\n", __func__);
			return -1;
		}

		goto retry;
	}

	if((base = dirfd(list)) < 0) {
		fprintf(stderr, "%s: dirfd /sys/class/net/: %m\n", __func__);
		goto done;
	}

	scim_port_wipe(_port);
	scim_port_head(_port);

	for(;; _port->info = *_port->item) {
		errno = 0;

		if((item = readdir(list)) == NULL) {
			if(errno) {
				fprintf(stderr, "%s: readdir /sys/class/net/: %m\n", __func__);
				goto done;
			}

			break;
		}

		if(item->d_type != DT_LNK) {
			continue;
		}

		if(scim_port_read(_port, item->d_name) < 0) {
			goto done;
		}
	}

	scim_port_head(_port);
	result = 0;

	done:
	closedir(list);
	return result;
}

int scim_port_dump(scim_port_data_t _port)
{
	for(scim_port_head(_port); *_port->info; _port->item++, _port->info = *_port->item) {
		for(_port->fact = __port; *_port->info; _port->info++, _port->fact++) {
			char* format;

			switch(_port->fact->code) {
				case _PORT_ADDRESS:
				format = "%-19s";
				break;

				case _PORT_NAME:
				format = "%-8s";
				break;

				case _PORT_MTU:
				format = " %-s";
				break;

				case _PORT_INDEX:
				format = " %-4s";
				break;

				case _PORT_FLAGS:
				format = " %-6s";
				break;

				default:
				format = " %-s";
				break;
			}

			printf(format, *_port->info);
		}

		putchar('\n');
	}

	return 0;
}

int scim_port_pick(scim_port_data_t _port, scim_port_code_t _code, const char* _term)
{
	for(scim_port_head(_port); *_port->info; _port->item++, _port->info = *_port->item) {
		if(!strncmp(_port->info[_code], _term, strlen(_term))) {
			return 0;
		}
	}

	errno = ENOENT;
	fprintf(stderr, "%s: %s: %m\n", __func__, _term);
	return -1;
}

int scim_port_find(scim_port_data_t _data, scim_port_item_t* _port, scim_port_code_t _code, const char* _term)
{
	size_t size = strlen(_term);
	scim_port_list_t list = _data->list;

	for(; **list; list++) {
		*_port = *list;

		if(!strncmp((*_port)[_code], _term, size)) {
			return true;
		}
	}

	return false;
}
 
int scim_port_make(scim_task_t _task)
{
	int index;
	int netlink;
	int result = -1;
	char name[IFNAMSIZ];
	unsigned char mac[6] = {};
	scim_port_data_t port;
	scim_port_data_t link;

	scim_port_wipe(link);
	scim_port_head(link);

	if((netlink = netlink_open()) < 0) {
		return -1;
	}

	if(scim_port_list(port) < 0) {
		return -1;
	}

	mac[0] = 0x02;
	mac[5] = _task->cell->code + 1;

	for(scim_lane_t* lane = *_task->cell->lanes; *lane; lane++) {
		snprintf(name, sizeof(name), "%sbone", (*lane)->name);

		if(scim_port_pick(port, _PORT_NAME, name) < 0) {
			goto done;
		}
	
		sscanf(port->info[_PORT_INDEX], "%d", &index);

		if(_task->form == __form_cell) {
			snprintf(name, sizeof(name), "%sport%d", (*lane)->name, _task->cell->code);
		}
		else {
			snprintf(name, sizeof(name), "%sport", (*lane)->name);
		}

		if((netlink_macvlan_create(netlink, name, mac, index, MACVLAN_MODE_BRIDGE)) < 0 && errno != EEXIST) {
			fprintf(stderr, "%s: netlink_macvlan_create: %m\n", __func__);
			goto done;
		}
	
		if(scim_port_read(link, name) < 0) {
			goto done;
		}

		sscanf(link->info[_PORT_INDEX], "%d", &index);

		if(_task->form == __form_cell) {
			if(netlink_link_move(netlink, index, _task->stat->pawn) < 0) {
				fprintf(stderr, "%s: netlink_link_move: %s to %d: %m\n", __func__, name, _task->stat->pawn);
			}
		}
	}

	result = 0;
	done:
	close(netlink);
	return result;
}

int scim_port_wake(scim_root_t _root)
{
	int index;
	int netlink;
	int result = -1;
	char name[IFNAMSIZ];
	scim_lane_t lane;
	scim_port_data_t port;

	if((netlink = netlink_open()) < 0) {
		fprintf(stderr, "%s: netlink_open: %m\n", __func__);
		return -1;
	}

	if(scim_port_list(port) < 0) {
		goto done;
	}

	for(scim_port_list_t list = port->list; **list; list++) {
		scim_port_item_t info = *list;

		snprintf(name, sizeof(name), "%s",  info[_PORT_NAME]);

		if(strcmp(name, "lo") && strncmp(name + 3, "bone", 4) && strncmp(name + 3, "port", 4)) {
			continue;
		}

		if(!sscanf(info[_PORT_INDEX], "%d", &index)) {
			errno = EINVAL;
			fprintf(stderr, "%s: port->info[_PORT_INDEX]: %m\n", __func__);
			goto done;
		}

		if(name[2] && !strncmp(name + 3, "port", 4)) {
			if(isdigit(name[7])) {
				name[7] = '\0';

				if(netlink_link_rename(netlink, name, index) < 0) {
					fprintf(stderr, "%s: netlink_link_rename %s to %s: %m\n", __func__, info[_PORT_NAME], name);
					goto done;
				}
			}

			for(lane = __lane; *lane->name && strncmp(lane->name, name, 3); lane++);

			if(*lane->name && lane->base) {
				uint32_t mask = ((uint32_t)0xffffffffU << lane->size) >> lane->size;

				uint32_t address = htonl(lane->base|(_root->task->cell->code + 1));

				uint32_t broadcast = htonl(lane->base|mask);

				if(netlink_address_four_create(netlink, index, address, broadcast, lane->size, 0) < 0) {
					if(errno != EEXIST) {
						fprintf(stderr, "%s: netlink_address_four_create: %s, %08x: %m\n", __func__, name, address);
						goto done;
					}
				}
			}
		}

		netlink_link_change(netlink, index, IFF_UP, IFF_UP);
	}

	result = 0;

	done:
	close(netlink);
	return result;
}

int scim_lane_make(scim_root_t _root)
{
	char name[IFNAMSIZ];
	char path[PATH_MAX];
	char term[BUFSIZ];

	for(scim_lane_t lane = __lane; *lane->name; lane++) {
		snprintf(name, sizeof(name), "%sbone", lane->name);
		snprintf(path, sizeof(path), "/sys/class/net/%s/bonding/", name);

		if(access(path, F_OK) < 0) {
			switch(errno) {
				case ENOENT:
				case ENOTDIR:
				snprintf(term, sizeof(term), "+%s", name);

				if(scim_term_post("/sys/class/net/bonding_masters", term) < 0) {
					return -1;
				}
				break;

				default:
				fprintf(stderr, "%s: access %s: %m\n", __func__, path);
				return -1;
			}
		}
		else {
			continue;
		}

		snprintf(path, sizeof(path), "/sys/class/net/%s/bonding/mode", name);

		if(scim_term_post(path, "1") < 0) {
			return -1;
		}

		snprintf(path, sizeof(path), "/sys/class/net/%s/bonding/miimon", name);
		
		if(scim_term_post(path, "1000") < 0) {
			return -1;
		}

		snprintf(path, sizeof(path), "/sys/class/net/%s/netdev_group", name);
		snprintf(term, sizeof(term), "%d", lane->code + 1);
		
		if(scim_term_post(path, term) < 0) {
			return -1;
		}

		snprintf(path, sizeof(path), "/sys/class/net/%s/bonding/slaves", name);
		snprintf(term, sizeof(term), "+%sloop", lane->name);

		if(scim_term_post(path, term) < 0) {
			return -1;
		}

		snprintf(path, sizeof(path), "/proc/sys/net/ipv6/conf/%s/disable_ipv6", name);

		if(scim_term_post(path, "1") < 0) {
			return -1;
		}
	}

	return 0;
}

int scim_link_init(void)
{
	char path[PATH_MAX];
	int file;
	int result = -1;
	scim_link_data_t link = {};

	snprintf(path, sizeof(path), "%sdb/link", LOCALSTATEDIR);

	if((file = open(path, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, path);
		return -1;
	}

	for(scim_lane_t lane = __lane; *lane->name; lane++) {
		strncpy(link->lane, lane->name, sizeof(link->lane));
		strncat(link->lane, "bone", sizeof(link->lane) - strlen(link->lane));

		if(write(file, link, sizeof(link)) < 0) {
			fprintf(stderr, "%s: write %s: %m\n", __func__, path);
			goto done;
		}
	}

	result = 0;
	done:
	close(file);
	return result;
}

int scim_link_save(const char* _lane, const char* _card)
{
	char path[PATH_MAX];
	int file;
	int result = -1;
	scim_link_data_t link = {};
	scim_port_data_t list = {};
	scim_port_item_t lane;
	scim_port_item_t card;

	if(scim_port_list(list) < 0) {
		return -1;
	}

	if(!scim_port_find(list, &lane, _PORT_NAME, _lane)) {
		return -1;
	}

	if(!scim_port_find(list, &card, _PORT_NAME, _card)) {
		return -1;
	}

	snprintf(path, sizeof(path), "%sdb/link", LOCALSTATEDIR);

	if(access(path, W_OK) < 0) {
		if(errno != ENOENT) {
			fprintf(stderr, "%s: access %s: %m\n", __func__, path);
			return -1;
		}

		if(scim_link_init() < 0) {
			return -1;
		}
	}

	if((file = open(path, O_RDWR)) < 0) {
		fprintf(stderr, "%s: open %s: %m\n", __func__, path);
		return -1;
	}

	for(int slot = 0;; slot++) {
		switch(read(file, link, sizeof(link))) {
			case -1:
			fprintf(stderr, "%s: read %s: %m\n", __func__, path);
			goto done;

			case 0:
			result = 0;
			goto done;

			default:
			break;
		}

		if(strcmp(link->lane, _lane)) {
			continue;
		}

		strncpy(link->code, card[_PORT_ADDRESS], sizeof(link->code));

		if(pwrite(file, link, sizeof(link), sizeof(link) * slot) < 0) {
			fprintf(stderr, "%s: pwrite %s: %m\n", __func__, path);
			goto done;
		}

		break;
	}

	result = 0;
	done:
	close(file);
	return result;
}

int scim_link_call(scim_link_call_t _call, void* _data)
{
	char path[PATH_MAX];
	int result = -1;
	int file;
	scim_link_data_t link = {};

	snprintf(path, sizeof(path), "%sdb/link", LOCALSTATEDIR);

	retry:

	if((file = open(path, O_RDONLY)) < 0) {
		if(errno != ENOENT) {
			fprintf(stderr, "%s: open %s: %m\n", __func__, path);
			return -1;
		}

		if(scim_link_init() < 0) {
			return -1;
		}

		goto retry;
	}

	for(;;) {
		switch(read(file, link, sizeof(link))) {
			case -1:
			fprintf(stderr, "%s: read %s: %m\n", __func__, path);
			goto done;

			case 0:
			result = 0;
			goto done;

			default:
			break;
		}

		if(_call(link, _data) < 0) {
			goto done;
		}
	}

	result = 0;
	done:
	close(file);
	return result;
}

int scim_link_open(scim_root_t _root)
{
	char path[PATH_MAX];
	char term[IFNAMSIZ];
	int file;
	int result = -1;
	scim_link_data_t link = {};
	scim_port_data_t port = {};
	scim_port_item_t info;

	snprintf(path, sizeof(path), "%sdb/link", LOCALSTATEDIR);

	if((file = open(path, O_RDONLY)) < 0) {
		if(errno != ENOENT) {
			fprintf(stderr, "%s: open %s: %m\n", __func__, path);
			return -1;
		}

		return 0;
	}

	if(scim_port_list(port) < 0) {
		goto done;
	}

	for(;;) {
		switch(read(file, link, sizeof(link))) {
			case -1:
			fprintf(stderr, "%s: read %s: %m\n", __func__, path);
			goto done;

			case 0:
			result = 0;
			goto done;

			default:
			break;
		}

		if(*link->code == '\0') {
			continue;
		}

		if(!scim_port_find(port, &info, _PORT_ADDRESS, link->code)) {
			continue;
		}

		snprintf(path, sizeof(path), "/sys/class/net/%s/bonding/slaves", link->lane);
		snprintf(term, sizeof(term), "+%s", info[_PORT_NAME]);

		if(scim_term_post(path, term) < 0) {
			return -1;
		}

		snprintf(path, sizeof(path), "/sys/class/net/%s/bonding/primary", link->lane);
		snprintf(term, sizeof(term), "%s", info[_PORT_NAME]);

		if(scim_term_post(path, term) < 0) {
			return -1;
		}
	}

	done:
	close(file);
	return result;
}
