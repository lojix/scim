#include "scim/scim.h"

int main(int argc, char* argv[])
{
	scim_root_t root = (struct scim_root_t[1]){};

	if(scim_host_wake(root) < 0) {
		_exit(EXIT_FAILURE);
	}

	if(scim_root_wake(root) < 0) {
		_exit(EXIT_FAILURE);
	}

	if(scim_root_work(root) < 0) {
		_exit(EXIT_FAILURE);
	}

	scim_host_down(root);

	_exit(EXIT_SUCCESS);
}
