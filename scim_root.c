#include "scim/scim.h"
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/syscall.h>
#include "scim/netlink.h"

#define _BASH					 ROOTBINDIR "bash"
#define _DBUS_UUIDGEN			 ROOTBINDIR "dbus-uuidgen"
#define _IPSET					ROOTSBINDIR "ipset"
#define _IPTABLES				ROOTSBINDIR "iptables-restore"
#define _IP6TABLES				ROOTSBINDIR "ip6tables-restore"
#define _SSH_KEYGEN				 ROOTBINDIR "ssh-keygen"
#define _RNDC_CONFGEN			ROOTSBINDIR "rndc-confgen"

#define _IPSET_FILE				LOCALSTATEDIR "state/ipset"
#define _IPTABLES_FILE			LOCALSTATEDIR "state/iptables"
#define _IP6TABLES_FILE			LOCALSTATEDIR "state/ip6tables"

#define _SSH_HOST_DSA_KEY		"/etc/openssh/ssh_host_dsa_key"
#define _SSH_HOST_ECDSA_KEY		"/etc/openssh/ssh_host_ecdsa_key"
#define _SSH_HOST_RSA_KEY		"/etc/openssh/ssh_host_rsa_key"

char** __tool_sundries[] = {
 (char*[]){_DBUS_UUIDGEN, "--ensure", NULL},
 NULL
};

// (char*[]){_RNDC_CONFGEN, "-a", "-k", "rndc", "-c", "/srv/bind/etc/rndc.key", NULL},
char* __file_firewall[] = {
 _IPSET_FILE,
 _IPTABLES_FILE,
 _IP6TABLES_FILE,
 NULL
};

char** __tool_firewall[] = {
 (char*[]){_IPSET, "--flush", NULL},
 (char*[]){_IPSET, "restore", NULL},
 (char*[]){_IPTABLES, NULL},
 (char*[]){_IP6TABLES, NULL},
 NULL
};

char* __file_ssh_keys[] = {
 _SSH_HOST_DSA_KEY,
 _SSH_HOST_RSA_KEY,
 _SSH_HOST_ECDSA_KEY,
 NULL
};

char** __tool_ssh_keys[] = {
 (char*[]){_SSH_KEYGEN, "-N", "", "-f", _SSH_HOST_DSA_KEY, "-q", "-t", "dsa", NULL},
 (char*[]){_SSH_KEYGEN, "-N", "", "-f", _SSH_HOST_RSA_KEY, "-q", "-t", "rsa", NULL},
 (char*[]){_SSH_KEYGEN, "-N", "", "-f", _SSH_HOST_ECDSA_KEY, "-q", "-t", "ecdsa", NULL},
 NULL
};

static int scim_fail_redo(scim_root_t _root, scim_task_t* _task)
{
	for(; *_task; _task++) {
		if((*_task)->stat->test == __test_fail) {
			(*_task)->stat->test = __test_okay;
			_root->dead--;
		}
	}

	scim_wait_toll(_root);

	return 0;
}

static int scim_sign_heed(scim_root_t _root)
{
	struct signalfd_siginfo siginfo[32] = {};

	if(read(_root->signal, siginfo, sizeof(siginfo)) < 0) {
		fprintf(stderr, "%s: read: %m\n", __func__);
		return -1;
	}

	for(struct signalfd_siginfo* detail = siginfo; detail->ssi_signo; detail++) {
		switch(detail->ssi_signo) {
			case SIGHUP:
			if(_root->tasks) {
				scim_fail_redo(_root, _root->tasks->task);
			}

			if(_root->cells) {
				scim_fail_redo(_root, _root->cells->task);
			}
			break;

			case SIGINT:
			if(_root->step == __step_work) {
				_root->halt = __reboot;
				_root->step = __step_done;

				fprintf(stderr, "%s rebooting!\n", _root->task->name);
			}
			break;

			case SIGTERM:
			if(_root->step == __step_work) {
				_root->halt = __poweroff;
				_root->step = __step_done;

				fprintf(stderr, "%s powering down!\n", _root->task->name);
			}
			break;

			case SIGCHLD:
			scim_task_reap(_root);
			break;
		}
	}

	return 0;
}

static int scim_peal_heed(scim_root_t _root)
{
	uint64_t tick = 0;

	if(read(_root->timer, &tick, sizeof(tick)) < 0) {
		return -1;
	}

	_root->tick += tick;

	if((_root->tick % 6) == 0) {
		if(_root->step == __step_work) {
			if(_root->tasks) {
				scim_fail_redo(_root, _root->tasks->task);
			}

			if(_root->cells) {
				scim_fail_redo(_root, _root->cells->task);
			}
		}
	}

	return 0;
}

static int scim_deed_wait(scim_root_t _root)
{
	struct epoll_event* event = (struct epoll_event[10]){};

	for(int result = epoll_wait(_root->epoll, event, 10, _root->stay); result; event++, result--) {
		if(result < 0) {
			if(errno == EINTR) {
				continue;
			}
		}

		if(event->events & EPOLLERR) {
			continue;
		}

		switch(event->data.u32) {
			case _HEED_FILE:
			//scim_file_heed(_root);
			break;

			case _HEED_SIGN:
			scim_sign_heed(_root);
			break;

			case _HEED_PEAL:
			scim_peal_heed(_root);
			break;
		}
	}

	return 0;
}

/*static int scim_turf_prep(scim_root_t _root)
{
	char* path[] = {(char [PATH_MAX]){}, (char [PATH_MAX]){}, NULL};

	snprintf(path[0], PATH_MAX, "%s%s/", SCIM_DATA_PATH, _root->task->host->name);
	snprintf(path[1], PATH_MAX, "%s%s/", SCIM_CELL_PATH, _root->task->host->name);

	for(int i = 0; path[i]; i++) {
		for(char* part = strchr(&path[i][1], '/'); part;) {
			*part = '\0';

			if(mkdir(path[i], 0755) < 0 && errno != EEXIST) {
				fprintf(stderr, "%s: mkdir %s: %m\n", __func__, path[i]);
				return -1;
			}

			*part++ = '/';
			part = strchr(part, '/');
		}
	}

	return 0;
}*/

static int scim_wake_prep(scim_root_t _root)
{
	if((_root->epoll = epoll_create1(EPOLL_CLOEXEC)) < 0) {
		fprintf(stderr, "%s: epoll_create: %m\n", __func__);
		return -1;
	}

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGTERM);

	if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		fprintf(stderr, "Error %s: sigprocmask: %m\n", __func__);
		return -1;
	}

	if((_root->signal = signalfd(-1, &mask, SFD_CLOEXEC)) < 0) {
		fprintf(stderr, "Error %s: signalfd: %m\n", __func__);
		return -1;
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.u32 = _HEED_SIGN;

	if(epoll_ctl(_root->epoll, EPOLL_CTL_ADD, _root->signal, &event) < 0) {
		fprintf(stderr, "Error %s: epoll_ctl: _root->signal: %m\n", __func__);
		return -1;
	}

	if((_root->timer = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC|TFD_NONBLOCK)) < 0) {
		return -1;
	}

	event.events = EPOLLIN;
	event.data.u32 = _HEED_PEAL;

	if(epoll_ctl(_root->epoll, EPOLL_CTL_ADD, _root->timer, &event) < 0) {
		fprintf(stderr, "Error %s: epoll_ctl: _root->timer: %m\n", __func__);
		return -1;
	}

	struct itimerspec timeout = {};
	timeout.it_value.tv_sec = 1;
	timeout.it_interval.tv_sec = 10;

	if(timerfd_settime(_root->timer, 0, &timeout, NULL) < 0) {
		fprintf(stderr, "Error %s: timerfd_settime: %m\n", __func__);
		return -1;
	}

	if((_root->inotify = inotify_init1(IN_CLOEXEC|IN_NONBLOCK)) < 0) {
		fprintf(stderr, "Error %s: inotify_init1: %m\n", __func__);
		return -1;
	}

	//char* path = alloca(snprintf(NULL, 0, "%smail", _root->path));
	//sprintf(path, "%smail", _root->path);

	//if((_root->stub = inotify_add_watch(_root->inotify, path, IN_CLOSE_WRITE)) < 0) {
	//	fprintf(stderr, "Error %s: inotify_add_watch: %m\n", __func__);
	//	return -1;
	//}

	event.events = EPOLLIN;
	event.data.u32 = _HEED_FILE;

	if(epoll_ctl(_root->epoll, EPOLL_CTL_ADD, _root->inotify, &event) < 0) {
		fprintf(stderr, "Error %s: epoll_ctl: _root->inotify: %m\n", __func__);
		return -1;
	}

	if((_root->netlink = netlink_open()) < 0) {
		return -1;
	}

	return 0;
}

/*int scim_team_read(scim_root_t _root, const char* _team)
{
	char* end;
	scim_team_code_t team;

	errno = 0;
	team = strtoul(_team, &end, 10);

   	if(errno) {
		fprintf(stderr, "%s: strtoul: %m\n", __func__);
		return -1;
   	}

	if(end == _team || team > _TEAM_MAX) {
		fprintf(stderr, "%s: strtoul: %u > %u or %p == %p: %s\n", __func__, team, _TEAM_MAX, strerror(EINVAL), end, _team);
		return -1;
	}

	_root->cells = __team[team].task;
	_root->tend = __team[team].size;
	_root->task = __task + _TASK_SRVCD;
	_root->task->host = __team[team].task[0]->host;

	return 0;
}*/

int scim_root_wake(scim_root_t _root)
{
	char** file;
	char*** tool;

	_root->down = _root->tend;
	_root->pawn = syscall(SYS_getpid);
	_root->step = __step_down;

	sethostname(_root->task->name, strlen(_root->task->name));

	if(prctl(PR_SET_NAME, _root->task->name) < 0) {
		fprintf(stderr, "%s: prctl: PR_SET_NAME, %s: %m\n", __func__, _root->task->name);
	}

	for(tool = __tool_sundries; *tool; tool++) {
		scim_tool_call(*tool, NULL, NULL, NULL);
	}

	for(file = __file_ssh_keys, tool = __tool_ssh_keys; *tool; file++, tool++) {
		if(access(*file, F_OK) != 0) {
			scim_tool_call(*tool, NULL, NULL, NULL);
		}
	}

	for(file = __file_firewall, tool = __tool_firewall; *tool; tool++) {
		scim_tool_call(*tool, NULL, *file, NULL);
	}

	if(scim_wake_prep(_root) < 0) {
		return -1;
	}

	scim_term_post("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", "0");

	if(scim_port_wake(_root) < 0) {
		return -1;
	}

	return 0;
}

int scim_root_done(scim_root_t _root)
{
	sigset_t mask;

	close(_root->timer);
	close(_root->inotify);
	close(_root->netlink);
	close(_root->signal);
	close(_root->epoll);

	_root->timer = -1;
	_root->inotify = -1;
	_root->netlink = -1;
	_root->signal = -1;
	_root->epoll = -1;

	sigfillset(&mask);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	return 0;
}

int scim_root_down(scim_root_t _root)
{
	char** file;
	char*** tool;

	for(file = __file_firewall, tool = __tool_firewall; *tool; tool++) {
		scim_tool_call(*tool, NULL, NULL, *file);
	}

	return 0;
}

int scim_root_work(scim_root_t _root)
{
	for(_root->loop = 0;; _root->loop++) {
		if(_root->step == __step_down) {
			if(_root->loop == 0) {
				fprintf(stderr, "Starting %s/%0X, zone %s, role %s, duty %s\n",
					_root->task->name, _root->task->cell->code, _root->task->cell->zone->term,
						_root->task->cell->role->term, _root->task->cell->duty->term);
				_root->step = __step_wake;
				scim_wait_toll(_root);
			}
			else {
				if(_root->tend == _root->down) {
					fprintf(stderr, "%s shuting down\n", _root->task->name);
					scim_root_down(_root);
					break;
				}
			}
		}

		if(_root->step == __step_wake) {
			_root->step = __step_work;
		}

		if(_root->step == __step_work) {

			if(_root->wait) {
				fprintf(stderr, "tend %d, wake %d, work %d, done %d, down %d, wait %d, wild %d, dead %d, waking\n",
					_root->tend, _root->wake, _root->work, _root->done, _root->down, _root->wait, _root->wild, _root->dead);

				if(_root->tasks) {

					scim_crew_wake(_root);
				}

				if(_root->cells) {
					scim_team_wake(_root);
				}
			}

			if(_root->work > _root->tend) {
				fprintf(stderr, "tend %d, wake %d, work %d, done %d, down %d, wait %d, wild %d, dead %d, culling\n",
					_root->tend, _root->wake, _root->work, _root->done, _root->down, _root->wait, _root->wild, _root->dead);

				if(_root->tasks) {
					scim_crew_cull(_root);
				}

				if(_root->cells) {
					scim_team_cull(_root);
				}
			}
		}

		scim_deed_wait(_root);

		if(_root->tasks) {
			scim_crew_tend(_root);
		}

		if(_root->cells) {
			scim_team_tend(_root);
		}

		if(_root->step == __step_done) {
			if(_root->down == _root->tend) {
				scim_root_done(_root);
				_root->step = __step_down;
			}
			else {
				//fprintf(stderr, "tend %d, wake %d, work %d, done %d, down %d, wait %d, wild %d, dead %d, resting\n", _root->tend, _root->wake, _root->work, _root->done, _root->down, _root->wait, _root->wild, _root->dead);
				if(_root->cells) {
					scim_team_down(_root);
				}

				if(_root->tasks) {
					scim_crew_down(_root);
				}
			}
		}
		else {
			_root->stay = -1;
		}
	}

	return 0;
}
