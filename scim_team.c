#include "scim/scim.h"

int scim_team_wake(scim_root_t _root)
{
	for(scim_task_t* task = _root->cells->task; *task; task++) {
		if((*task)->stat->step != __step_down) {
			continue;
		}

		scim_task_redo(_root, *task);
	}

	return 0;
}

int scim_team_down(scim_root_t _root)
{
	for(scim_task_t* task = _root->cells->task; *task; task++) {
		if((*task)->stat->step->code != _STEP_WORK) {
			continue;
		}

		scim_task_undo(_root, *task);
	}

	return 0;
}

int scim_team_tend(scim_root_t _root)
{
	for(scim_task_t* task = _root->cells->task; *task; task++) {
		if((*task)->stat->pawn == 0) {
			continue;
		}

		scim_task_tend(_root, *task);
	}

	return 0;
}

int scim_team_cull(scim_root_t _root)
{
	for(scim_task_t* task = _root->cells->task; *task; task++) {
		if((*task)->stat->mode->code == _MODE_REST && (*task)->stat->step->code == _STEP_WORK) {
			scim_task_kill(_root, *task);
		}
	}

	return 0;
}
