#include "scim/scim.h"
#include <net/if.h>

int main(int argc, char* argv[])
{
	if(argc < 4) {
		exit(EXIT_FAILURE);
	}

	if(scim_glusterfs_root_init(argv[1]) < 0) {
		exit(EXIT_FAILURE);
	}

	if(scim_glusterfs_entry_init(argv[2]) < 0) {
		exit(EXIT_FAILURE);
	}

	if(scim_glusterfs_make(argv[3], argv[4]) < 0) {
		exit(EXIT_FAILURE);
	}
/*	scim_port_data_t list;
	scim_port_item_t item;

	if(scim_port_list(list) < 0) {
		exit(EXIT_FAILURE);
	}

	if(scim_port_find(list, &item, _PORT_ADDRESS, argv[1])) {
		printf("%s %s %s %s %s\n", item[_PORT_NAME], item[_PORT_ADDRESS], item[_PORT_INDEX], item[_PORT_FLAGS], item[_PORT_MTU]);
	}

	if(argc > 1) {
		scim_port_wipe(port);
		scim_port_head(port);

		for(argv++; *argv; argv++) {
			if(scim_port_read(port, *argv) < 0)  {
				exit(EXIT_FAILURE);
			}

			if(scim_port_flag(port, 1, IFF_UP) < 0) {
				exit(EXIT_FAILURE);
			}
		}

		scim_port_dump(port);
	}
	else {
		scim_port_list(port);
		scim_port_dump(port);
	}

	scim_site_data_t site = {};
	scim_site_scan(site, "/proc/mounts");
	scim_site_dump(site);

	putchar('\n');

	scim_site_data_t disk = {};
	scim_site_pick(disk, site, _SITE_TYPE, "btrfs", true);
	scim_site_dump(disk);

	putchar('\n');

	scim_site_data_t none = {};
	scim_site_pick(none, site, _SITE_TYPE, "btrfs", false);
	scim_site_dump(none);

	printf("%d\n", LONG_BIT);

	if(scim_cpio_data_save(NULL, NULL, NULL) < 0) {
		exit(EXIT_FAILURE);
	}

	if(scim_lzma_pack("/dev/zram0", "/tmp/data.xz") < 0) {
		exit(EXIT_FAILURE);
	}

	if(scim_loop_bind("/dev/loop2", "/boot/system", 0) < 0) {
		exit(EXIT_FAILURE);
	}

	sleep(15);

	scim_loop_free("/dev/loop2");
	scim_link_save((struct scim_root_t[]){});
*/
	exit(EXIT_SUCCESS);
}
