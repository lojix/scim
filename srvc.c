#include "scim/scim.h"

static int srvc_link_print(scim_link_data_t _link, void* _data)
{
	printf("%s %s\n", _link->lane, _link->code);
	return 0;
}

static int srvc_link_list(int _argc, char* _argv[])
{
	int result = -1;
	result = scim_link_call(srvc_link_print, NULL);
	return result;
}

static int srvc_link_save(int _argc, char* _argv[])
{
	int result = -1;

	if(_argc < 2) {
		fprintf(stderr, "usage: %s link save BACKBONE PORT\n", __progname);
		goto done;
	}

	result = scim_link_save(_argv[0], _argv[1]);

	done:
	return result;
}

static int srvc_link_open(int _argc, char* _argv[])
{
	struct scim_root_t root = {};
	return scim_link_open(&root);
}

static int srvc_link_shut(int _argc, char* _argv[])
{
	int result = -1;
	return result;
}

static int srvc_link(int _argc, char* _argv[])
{
	int result = 0;

	if(!_argc || !*_argv) {
		return result;
	}
	else if(!strcmp("list", *_argv)) {
		result = srvc_link_list(--_argc, ++_argv);
	}
	else if(!strcmp("open", *_argv)) {
		result = srvc_link_open(--_argc, ++_argv);
	}
	else if(!strcmp("shut", *_argv)) {
		result = srvc_link_shut(--_argc, ++_argv);
	}
	else if(!strcmp("save", *_argv)) {
		result = srvc_link_save(--_argc, ++_argv);
	}

	return result;
}

static int srvc_slot(int _argc, char* _argv[])
{
	int result = -1;
	long slot = 0L;

	if(!_argc || !*_argv) {
		return result;
	}
	else if(!strcmp("set", *_argv)) {
		if(!*++_argv) {
			fprintf(stderr, "%s: _argv: %s\n", __func__, strerror(EINVAL));
			goto done;
		}

		if(sscanf(*_argv, "%ld", &slot) != 1) {
			fprintf(stderr, "%s: sscanf %s: %s\n", __func__, *_argv, strerror(EINVAL));
			goto done;
		}

		if(sethostid(slot) < 0) {
			fprintf(stderr, "%s: sethostid %ld: %m\n", __func__, slot);
		}
	}
	else if(!strcmp("get", *_argv)) {
		if(access("/etc/hostid", R_OK) < 0) {
			fprintf(stderr, "%s: access /etc/hostid: %m\n", __func__);
			result = -1;
		}
		else {
			fprintf(stdout, "%ld\n", gethostid());
		}
	}

	result = 0;
	done:
	return result;
}

int main(int argc, char* argv[])
{
	int result = 0;

	argc--;
	argv++;

	if(!argc || !*argv) {
		_exit(EXIT_FAILURE);
	}
	else if(!strcmp("slot", *argv)) {
		result = srvc_slot(--argc, ++argv);
	}
	else if(!strcmp("link", *argv)) {
		result = srvc_link(--argc, ++argv);
	}

	_exit(!result? EXIT_SUCCESS: EXIT_FAILURE);
}
