#include "scim/scim.h"
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <wordexp.h>

int scim_task_wipe(scim_task_t _task)
{
	_task->stat->mode = __mode_ward;
	_task->stat->step = __step_down;
	_task->stat->test = __test_okay;
	_task->stat->mark = 0;
	_task->stat->dean = 0;
	_task->stat->pawn = 0;
	_task->stat->hear = 0;
	_task->stat->tell = 0;
	_task->stat->aide = 0;
	_task->stat->ward = 0;
	_task->stat->wake = 0;
	return 0;
}

int scim_task_mark(unsigned _mark)
{
	FILE* file = fopen("/proc/self/loginuid", "w");

	if(!file) {
		fprintf(stderr, "Error %s: fopen: /proc/self/loginuid: %m\n", __func__);
		return -1;
	}

	int result = 0;

	if(fprintf(file, "%u", _mark) < 0) {
		fprintf(stderr, "Error %s: fprintf: /proc/self/loginuid: %m\n", __func__);
		result = -1;
	}

	fclose(file);

	return result;
}

int scim_task_test(unsigned _mark, pid_t _pid)
{
	char path[32];
	FILE* file;
	unsigned mark = ~0U;

	if(_pid < 1) {
		return false;
	}

	snprintf(path, sizeof(path), "/proc/%d/loginuid", _pid);

	if(!(file = fopen(path, "r"))) {
		return false;
	}

	switch(fscanf(file, "%u", &mark)) {
		case EOF:
		break;

		case 0:
		break;

		default:
		break;
	}

	fclose(file);
	return mark == _mark;
}

int scim_task_seek(scim_stat_t _state)
{
	for(pid_t next = _state->pawn - 32768, pid = _state->pawn + 1; ; next++, pid++) {
		if(!next) {
			return -1;
		}

		if(!kill(pid, 0) && scim_task_test(_state->mark, pid)) {
			return (_state->pawn = pid);
		}
	}
}

int scim_task_reap(scim_root_t _root)
{
	for(;;) {
		scim_task_t* task = NULL;
		siginfo_t detail = {};

		waitid(P_ALL, 0, &detail, WEXITED|WNOHANG|WNOWAIT);

		if(!detail.si_pid) {
			break;
		}

		if(_root->tasks && _root->tasks->task) {
			for(task = _root->tasks->task; *task; task++) {
				if((*task)->stat->pawn == detail.si_pid) {
					break;
				}
			}
		}

		if(task && *task == NULL && _root->cells && _root->cells->task) {
			for(task = _root->cells->task; *task; task++) {
				if((*task)->stat->pawn == detail.si_pid) {
					break;
				}
			}
		}

		if(_root->step != __step_done && task && *task) {
			switch((*task)->stat->step->code) {
				case _STEP_DOWN:
				case _STEP_WAKE:
				if((*task)->mode == __mode_free) {
					switch(scim_task_seek((*task)->stat)) {
						case -1:
						fprintf(stderr, "fail %s %d (%s)\n", (*task)->name, (*task)->stat->pawn, (*task)->spec);
						break;

						default:
						(*task)->stat->dean = 1;
						(*task)->stat->mode = __mode_free;
						_root->wild++;
						fprintf(stderr, "hier %s %d\n", (*task)->name, (*task)->stat->pawn);
						break;
					}
				}

				break;

				default:
				break;
			}
		}

		waitid(P_PID, detail.si_pid, NULL, WEXITED|WNOHANG);
	}

	return 0;
}

int scim_wait_toll(scim_root_t _root)
{
	return (_root->wait = _root->tend - _root->work - _root->wake - _root->dead);
}

int scim_task_tend(scim_root_t _root, scim_task_t _task)
{
	bool dead = ((kill(_task->stat->pawn, 0) < 0)? true: false);

	switch(_root->step->code) {
		case _STEP_WAKE:
		//goto done;

		case _STEP_WORK:
		switch(dead) {
			case true:
			switch(_task->stat->step->code) {
				case _STEP_DOWN:
				if(!_task->stat->pawn) {
					goto done;
				}

				_task->stat->step = __step_wake;
				_root->down--;
				_root->wake++;
				break;

				case _STEP_WAKE:
				if(_task->stat->mode == __mode_free && _task->stat->mode == __mode_ward) {
					goto done;
				}

				_task->stat->step = __step_done;
				_task->stat->test = __test_fail;
				_root->wake--;
				_root->done++;
				_root->dead++;
				break;

				case _STEP_WORK:
				_task->stat->step = __step_done;
				_root->work--;
				_root->done++;
				break;

				case _STEP_DONE:
				_task->stat->step = __step_down;
				_root->done--;
				_root->down++;
				break;

				default:
				goto done;
			}
			break;

			case false:
			switch(_task->stat->step->code) {
				case _STEP_DOWN:
				_task->stat->step = __step_wake;
				_root->down--;
				_root->wake++;

				if(_task->stat->test == __test_fail) {
					_task->stat->test = __test_okay;

					if(_root->dead > 0) {
						_root->dead--;
					}
				}
				break;

				case _STEP_WAKE:
				if(_task->mode == __mode_free && _task->stat->mode == __mode_ward) {
					goto done;
				}

				_task->stat->step = __step_work;
				_root->wake--;
				_root->work++;
				break;

				case _STEP_WORK:
				goto done;

				case _STEP_DONE:
				if(_task->stat->test == __test_busy) {
					goto done;
				}

				_task->stat->test = __test_busy;
				break;

				default:
				goto done;
			}
			break;
		}
		break;

		case _STEP_DONE:
		switch(dead) {
			case true:
			switch(_task->stat->step->code) {
				case _STEP_DOWN:
				goto done;

				case _STEP_WAKE:
				_task->stat->step = __step_done;
				_root->wake--;
				_root->done++;
				break;

				case _STEP_WORK:
				if(_task->stat->test == __test_busy) {
					_task->stat->test = __test_okay;
				}

				_task->stat->step = __step_done;
				_root->work--;
				_root->done++;
				break;

				case _STEP_DONE:
				_task->stat->step = __step_down;
				_root->done--;
				_root->down++;
				break;

				default:
				goto done;
			}
			break;

			case false:
			switch(_task->stat->step->code) {
				case _STEP_DOWN:
				goto done;

				case _STEP_WAKE:
				if(_task->stat->test == __test_okay) {
					_task->stat->test = __test_busy;
				}
				goto done;

				case _STEP_WORK:
				if(_task->form != __form_root) {
					if(_task->stat->test == __test_okay) {
						_task->stat->test = __test_busy;
					}

					goto done;
				}

				if(_root->work > 1) {
					goto done;
				}

				_task->stat->step = __step_done;
				_root->done++;
				_root->work--;
				break;

				case _STEP_DONE:
				_task->stat->step = __step_down;
				_root->done--;
				_root->down++;
				break;

				default:
				goto done;
			}
			break;
		}
		break;

		case _STEP_DOWN:
		switch(dead) {
			case true:
			goto done;

			case false:
			switch(_task->stat->step->code) {
				case _STEP_DONE:
				_task->stat->step = __step_down;
				_root->done--;
				break;

				default:
				goto done;
			}
			break;
		}
		break;

		default:
		goto done;
	}

	if(_root->step != __step_done) {
		fprintf(stderr, "%s %s %d\n", _task->stat->step->term, _task->name, _task->stat->pawn);
	}

	switch(_task->stat->step->code) {
		case _STEP_DOWN:
		if(_task->ward->task) {
			for(scim_task_t* ward = _task->ward->task; *ward; ward++) {
				if((*ward)->stat->aide > 0) {
					(*ward)->stat->aide--;
				}
			}
		}

		if(_task->aide->task) {
			for(scim_task_t* aide = _task->aide->task; *aide; aide++) {
				if((*aide)->stat->ward > 0) {
					(*aide)->stat->ward--;
				}
			}
		}

		if(_task->stat->mode == __mode_free && _root->wild) {
			_root->wild--;
		}

		_task->stat->dean = 0;
		_task->stat->pawn = 0;
		break;

		case _STEP_WAKE:
		break;

		case _STEP_WORK:
		if(_task->ward->task) {
			for(scim_task_t* ward = _task->ward->task; *ward; ward++) {
				if((*ward)->stat->aide < (*ward)->aide->size) {
					(*ward)->stat->aide++;
				}
			}
		}

		if(_task->aide->task) {
			for(scim_task_t* aide = _task->aide->task; *aide; aide++) {
				if((*aide)->stat->ward < (*aide)->ward->size) {
					(*aide)->ward++;
				}
			}
		}
		break;

		case _STEP_DONE:
		break;

		default:
		break;
	}

	done:
	scim_wait_toll(_root);
	return 0;
}

int scim_task_wake(scim_root_t _root, scim_task_t _task)
{
	int result = 0;
	int cell[2];
	int root[2];

	if(_task->stat->step->code != _STEP_DOWN) {
		return 0;
	}

	_task->stat->mark = ++_root->mark;
	_task->stat->dean = _root->pawn;
	_task->stat->mode = __mode_ward;

	switch(_task->form->code) {
		case _FORM_ROOT:
		break;

		case _FORM_CELL:
		if(pipe2(cell, O_CLOEXEC) < 0) {
			fprintf(stderr, "%s: pipe2: %m\n", __func__);
			return -1;
		}

		if(pipe2(root, O_CLOEXEC) < 0) {
			fprintf(stderr, "%s: pipe2: %m\n", __func__);
			close(cell[0]);
			close(cell[1]);
			return -1;
		}
		break;

		default:
		_task->cell = _root->task->cell;
		break;
	}

	switch(_task->form->code) {
		case _FORM_ROOT:
		_task->stat->pawn = _root->pawn;
		break;

		case _FORM_CELL:
		if((_task->stat->pawn = syscall(SYS_clone, _FORK_CELL, NULL)) < 0) {
			fprintf(stderr, "Error %s: SYS_clone: %m\n", __func__);
			return -1;
		}
		break;

		default:
		if((_task->stat->pawn = fork()) < 0) {
			fprintf(stderr, "Error %s: fork: %m\n", __func__);
			return -1;
		}
		break;
	}

	if(_task->stat->pawn > 0) {
		switch(_task->form->code) {
			case _FORM_ROOT:
			break;

			case _FORM_CELL:
			close(cell[0]);
			close(root[1]);
			_task->stat->hear = root[0];
			_task->stat->tell = cell[1];

			result = 1;

			if(scim_port_make(_task) < 0) {
				result = 0;
			}

			if(write(_task->stat->tell, &result, sizeof(result)) < 0) {
				fprintf(stderr, "%s: write: %m\n", __func__);
			}

			result = -1;

			switch(read(_task->stat->hear, &result, sizeof(result))) {
				case -1:
				fprintf(stderr, "%s: read: %m\n", __func__);
				break;

				case 0:
				result = 0;
				break;

				default:
				errno = result;
				result = -1;
				break;
			}
			break;

			default:
			break;
		}

		if(result < 0) {
			int error = errno;
			close(_task->stat->hear);
			close(_task->stat->tell);
			errno = error;
		}

		_root->stay = 0;
		return result;
	}

	char path[PATH_MAX];
	wordexp_t argv[1];

	scim_task_mark(_root->mark);
	scim_root_done(_root);

	switch(_task->form->code) {
		case _FORM_CELL:
		close(cell[1]);
		close(root[0]);

		_task->stat->hear = cell[0];
		_task->stat->tell = root[1];

		snprintf(path, sizeof(path), "%s%s/var/log/%s", SCIM_CELL_PATH, _task->name, _task->name);

		if(scim_log_open(path) < 0) {
			goto fail;
		}
		break;

		case _FORM_ROOT:
		break;

		default:
		close(0);
		open("/dev/null", O_WRONLY);
		close(1);
		open("/dev/null", O_RDONLY);
		close(2);
		open("/dev/null", O_RDONLY);

		if(_task->sect > 0) {
			setgid(_task->sect);
		}

		if(_task->soul > 0) {
			setuid(_task->soul);
		}

		wordexp(_task->spec, argv, 0);
		execve(argv->we_wordv[0], argv->we_wordv, environ);
		goto fail;
	}

	*_root = *((struct scim_root_t[1]){});

	if(_task->form == __form_cell && scim_cell_wake(_root, _task) < 0) {
		goto fail;
	}

	if(scim_root_wake(_root) == 0) {
		scim_root_work(_root);
		_exit(EXIT_SUCCESS);
	}

	fail:
	_exit(EXIT_FAILURE);
}

int scim_task_wake_sure(scim_root_t _root, scim_task_t _task)
{
	int result = -1;

	if(scim_task_wake(_root, _task) < 0) {
		return -1;
	}

	for(;;) {
		nanosleep((struct timespec[]){{0, 15000000}}, NULL);

		scim_task_reap(_root);
		scim_task_tend(_root, _task);

		if(_task->stat->step == __step_work) {
			break;
		}

		if(_task->stat->step == __step_done) {
			goto done;
		}
	}

	result = 0;
	done:
	return result;
}

int scim_task_kill(scim_root_t _root, scim_task_t _task)
{
	if(_task->stat->step == __step_down) {
		return 0;
	}

	if(kill(_task->stat->pawn, _root->kill) < 0) {
		if(errno != ESRCH) {
			fprintf(stderr, "%s: kill: %d: %m\n", __func__, _task->stat->pawn);
			return -1;
		}
	}

	return 0;
}

int scim_task_redo(scim_root_t _root, scim_task_t _task)
{
	if(_task->stat->aide < _task->aide->size) {
		for(scim_task_t* aide = _task->aide->task; *aide; aide++) {
			scim_task_redo(_root, *aide);
		}
	}
	else if(_task->stat->pawn == 0 && _task->stat->test == __test_okay) {
		scim_task_wake(_root, _task);
	}

	return 0;
}

int scim_task_undo(scim_root_t _root, scim_task_t _task)
{
	if(_task->stat->ward) {
		for(scim_task_t* ward = _task->ward->task; *ward; ward++) {
			scim_task_undo(_root, *ward);
		}
	}
	else if(_task->stat->pawn != 0) {
		scim_task_kill(_root, _task);
	}

	return 0;
}
