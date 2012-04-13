#include "scim/scim.h"
#include <net/if.h>

int main(int argc, char* argv[])
{/*
	scim_port_data_t port;

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
*/
	if(scim_loop_bind("/dev/loop1", "/boot/system", 0) < 0) {
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
