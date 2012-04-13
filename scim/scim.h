#ifndef __SCIM_H__
#define __SCIM_H__
#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "code.h"

#ifndef __CELL_TEAM_MAX
#define __CELL_TEAM_MAX 7
#endif

#ifndef __TASK_TEAM_MAX
#define __TASK_TEAM_MAX 31
#endif

#ifndef __PORT_CURB_MAX
#define __PORT_CURB_MAX 63
#endif

#ifndef __SITE_CURB_MAX
#define __SITE_CURB_MAX 63
#endif

#ifndef SCIM_DATA_PATH
#define SCIM_DATA_PATH "/adm/"
#endif

#ifndef SCIM_CELL_PATH
#define SCIM_CELL_PATH "/vol/cell/"
#endif

#define _FORK_CELL CLONE_NEWNS|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWNET|SIGCHLD

typedef enum scim_kind_code_t {
 _KIND_DISTRICT,
 _KIND_EMULATOR,
 _KIND_HARDWARE,
 _KIND_CODE_MAX = _KIND_HARDWARE,
 _KIND_CODE_END,
 _KIND_CODE_SUM
} scim_kind_code_t;

typedef enum scim_role_code_t {
 _ROLE_GATEWAY,
 _ROLE_STATION,
 _ROLE_STEWARD,
 _ROLE_CODE_MAX = _ROLE_STEWARD,
 _ROLE_CODE_END,
 _ROLE_CODE_SUM
} scim_role_code_t;

typedef enum scim_zone_code_t {
 _ZONE_INTRANET,
 _ZONE_EXTRANET,
 _ZONE_INTERNET,
 _ZONE_CODE_MAX = _ZONE_INTERNET,
 _ZONE_CODE_END,
 _ZONE_CODE_SUM
} scim_zone_code_t;

typedef enum scim_duty_code_t {
 _DUTY_CONTENT,
 _DUTY_HOSTING,
 _DUTY_MESSAGE,
 _DUTY_PRESIDE,
 _DUTY_ROUTING,
 _DUTY_SESSION,
 _DUTY_STORAGE,
 _DUTY_CODE_MAX = _DUTY_STORAGE,
 _DUTY_CODE_END,
 _DUTY_CODE_SUM
} scim_duty_code_t;

typedef enum scim_post_code_t {
 _POST_INTRANET_CONTENT,
 _POST_INTRANET_HOSTING,
 _POST_INTRANET_MESSAGE,
 _POST_INTRANET_PRESIDE,
 _POST_INTRANET_ROUTING,
 _POST_INTRANET_SESSION,
 _POST_INTRANET_STORAGE,
 _POST_EXTRANET_CONTENT,
 _POST_EXTRANET_HOSTING,
 _POST_EXTRANET_MESSAGE,
 _POST_EXTRANET_PRESIDE,
 _POST_EXTRANET_ROUTING,
 _POST_EXTRANET_SESSION,
 _POST_EXTRANET_STORAGE,
 _POST_INTERNET_CONTENT,
 _POST_INTERNET_HOSTING,
 _POST_INTERNET_MESSAGE,
 _POST_INTERNET_PRESIDE,
 _POST_INTERNET_ROUTING,
 _POST_INTERNET_SESSION,
 _POST_INTERNET_STORAGE,
 _POST_CODE_MAX = _POST_INTERNET_STORAGE,
 _POST_CODE_END,
 _POST_CODE_SUM
} scim_post_code_t;

typedef enum scim_form_code_t {
 _FORM_ROOT,
 _FORM_CELL,
 _FORM_HOST,
 _FORM_TASK,
 _FORM_CODE_MAX = _FORM_TASK,
 _FORM_CODE_END,
 _FORM_CODE_SUM
} scim_form_code_t;

typedef enum scim_mode_code_t {
 _MODE_REST,
 _MODE_WAKE,
 _MODE_WARD,
 _MODE_FREE,
 _MODE_CODE_MAX = _MODE_FREE,
 _MODE_CODE_END,
 _MODE_CODE_SUM
} scim_mode_code_t;

typedef enum scim_step_code_t {
 _STEP_DOWN,
 _STEP_WAKE,
 _STEP_WORK,
 _STEP_DONE,
 _STEP_CODE_MAX = _STEP_DONE,
 _STEP_CODE_END,
 _STEP_CODE_SUM
} scim_step_code_t;

typedef enum scim_test_code_t {
 _TEST_OKAY,
 _TEST_BUSY,
 _TEST_FAIL,
 _TEST_CODE_MAX = _TEST_FAIL,
 _TEST_CODE_END,
 _TEST_CODE_SUM
} scim_test_code_t;

typedef enum scim_move_code_t {
 _MOVE_POKE,
 _MOVE_REST,
 _MOVE_WAKE,
 _MOVE_CODE_MAX = _MOVE_WAKE,
 _MOVE_CODE_END,
 _MOVE_CODE_SUM
} scim_move_code_t;

typedef enum scim_heed_code_t {
 _HEED_FILE,
 _HEED_SIGN,
 _HEED_PEAL,
 _HEED_CODE_MAX = _HEED_PEAL,
 _HEED_CODE_END,
 _HEED_CODE_SUM
} scim_heed_code_t;

typedef enum scim_lane_code_t {
 _LANE_LAN,
 _LANE_SAN,
 _LANE_DMZ,
 _LANE_ISP,
 _LANE_CODE_MAX = _LANE_ISP,
 _LANE_CODE_END,
 _LANE_CODE_SUM
} scim_lane_code_t;

typedef enum scim_port_code_t {
 _PORT_NAME,
 _PORT_INDEX,
 _PORT_ADDRESS,
 _PORT_FLAGS,
 _PORT_MTU,
 _PORT_CODE_MAX = _PORT_MTU,
 _PORT_CODE_END,
 _PORT_CODE_SUM
} scim_port_code_t;

typedef enum scim_site_code_t {
 _SITE_DISK,
 _SITE_PATH,
 _SITE_TYPE,
 _SITE_FLAG,
 _SITE_PAD1,
 _SITE_PAD2,
 _SITE_CODE_MAX = _SITE_PAD2,
 _SITE_CODE_END,
 _SITE_CODE_SUM
} scim_site_code_t;

typedef enum scim_port_curb_t {
 _PORT_CURB_MAX = __PORT_CURB_MAX,
 _PORT_CURB_END,
 _PORT_CURB_SUM
} scim_port_curb_t;

typedef enum scim_site_curb_t {
 _SITE_CURB_MAX = __SITE_CURB_MAX,
 _SITE_CURB_END,
 _SITE_CURB_SUM
} scim_site_curb_t;

typedef enum scim_cell_curb_t {
 _CELL_TEAM_MAX = __CELL_TEAM_MAX,
 _CELL_TEAM_END,
 _CELL_TEAM_SUM
} scim_cell_curb_t;

typedef enum scim_task_curb_t {
 _TASK_TEAM_MAX = __TASK_TEAM_MAX,
 _TASK_TEAM_END,
 _TASK_TEAM_SUM
} scim_task_curb_t;

typedef char scim_form_term_t[16];
typedef char scim_mode_term_t[16];
typedef char scim_step_term_t[16];
typedef char scim_test_term_t[16];
typedef char scim_move_term_t[16];
typedef char scim_heed_term_t[16];

typedef char scim_task_term_t[32];
typedef char scim_task_path_t[64];
typedef char scim_task_spec_t[512];

typedef char scim_kind_term_t[16];
typedef char scim_role_term_t[16];
typedef char scim_duty_term_t[16];
typedef char scim_port_term_t[32];
typedef char scim_port_call_t[20];
typedef char scim_site_term_t[16];
typedef char scim_post_term_t[32];
typedef char scim_zone_term_t[16];
typedef char scim_lane_term_t[32];
typedef char scim_lane_name_t[16];
typedef char scim_host_term_t[32];

typedef char scim_card_code_t[32];
typedef char scim_card_term_t[32];

typedef struct scim_form_t {
 scim_form_code_t code;
 scim_form_term_t term;
}* scim_form_t;

typedef struct scim_mode_t {
 scim_mode_code_t code;
 scim_mode_term_t term;
}* scim_mode_t;

typedef struct scim_step_t {
 scim_step_code_t code;
 scim_step_term_t term;
}* scim_step_t;

typedef struct scim_test_t {
 scim_test_code_t code;
 scim_test_term_t term;
}* scim_test_t;

typedef struct scim_move_t {
 scim_move_code_t code;
 scim_move_term_t term;
}* scim_move_t;

typedef struct scim_heed_t {
 scim_heed_code_t code;
 scim_heed_term_t term;
}* scim_heed_t;

typedef struct scim_kind_t {
 scim_kind_code_t code;
 scim_kind_term_t term;
}* scim_kind_t;

typedef struct scim_role_t {
 scim_role_code_t code;
 scim_role_term_t term;
}* scim_role_t;

typedef struct scim_duty_t {
 scim_duty_code_t code;
 scim_duty_term_t term;
}* scim_duty_t;

typedef struct scim_post_t {
 scim_post_code_t code;
 scim_post_term_t term;
}* scim_post_t;

typedef struct scim_zone_t {
 scim_zone_code_t code;
 scim_zone_term_t term;
}* scim_zone_t;

typedef struct scim_lane_t {
 scim_lane_code_t code;
 scim_lane_term_t name;
 unsigned int base;
 unsigned int size;
}* scim_lane_t;

typedef struct scim_port_t {
 scim_port_code_t code;
 scim_port_term_t term;
}* scim_port_t;

typedef struct scim_site_t {
 scim_site_code_t code;
 scim_site_term_t term;
}* scim_site_t;

/*typedef struct scim_card_t {
 scim_card_code_t code;
 scim_card_term_t name;
 scim_host_code_t host;
 unsigned slot;
}* scim_card_t;*/

typedef struct scim_link_data_t {
 scim_port_call_t port;
 scim_lane_name_t lane;
} scim_link_data_t[1];

typedef char* (*scim_port_item_t);
typedef char* (*scim_port_list_t)[_PORT_CODE_SUM];

typedef struct scim_port_data_t {
 int last;
 scim_port_t fact;
 char* list[_PORT_CURB_SUM][_PORT_CODE_SUM] __attribute__((aligned));
 char* (*item)[_PORT_CODE_SUM];
 char** info;
 char* data;
 size_t room;
 size_t size;
 char heap[BUFSIZ];
} scim_port_data_t[1];

typedef char* (*scim_site_item_t);
typedef char* (*scim_site_list_t)[_SITE_CODE_SUM];

typedef struct scim_site_data_t {
 int last;
 unsigned size;
 unsigned flag;
 char* list[_SITE_CURB_SUM][_SITE_CODE_SUM] __attribute__((aligned));
 char data[BUFSIZ];
} scim_site_data_t[1];

typedef struct scim_stat_t {
 scim_mode_t mode;
 scim_step_t step;
 scim_test_t test;
 unsigned mark;
 pid_t dean;
 pid_t pawn;
 int hear;
 int tell;
 int aide;
 int ward;
 int wake;
}* scim_stat_t;

typedef struct scim_cell_t* scim_cell_t;
typedef struct scim_host_t* scim_host_t;
typedef struct scim_task_t* scim_task_t;
typedef struct scim_task_team_t* scim_task_team_t;

struct scim_task_team_t {
 scim_task_t* task;
 unsigned size;
};

struct scim_task_t {
 scim_task_code_t code;
 scim_task_term_t name;
 scim_task_spec_t spec;
 scim_soul_code_t soul;
 scim_sect_code_t sect;
 scim_form_t form;
 scim_mode_t mode;
 scim_task_team_t aide;
 scim_task_team_t ward;
 scim_stat_t stat;
 scim_cell_t cell;
};

struct scim_cell_t {
 scim_cell_code_t code;
 scim_kind_t kind;
 scim_role_t role;
 scim_duty_t duty;
 scim_zone_t zone;
 scim_task_t task;
 scim_task_team_t tasks;
 scim_lane_t (*lanes)[_LANE_CODE_SUM];
};

struct scim_host_t {
 scim_task_t task;
 scim_task_team_t tasks;
 scim_task_team_t cells;
};

typedef struct scim_halt_data_t {
 int code;					// Magic number.
 int type;					// What kind of request.
 int step;					// Runlevel to change to.
 int time;					// Time between TERM and KILL.
 char data[368];
}* scim_halt_data_t;  

typedef struct scim_root_t {
 scim_task_t task;
 scim_task_team_t tasks;
 scim_task_team_t cells;
 scim_step_t step;
 int epoll;
 int inotify;
 int netlink;
 int stub;
 int signal;
 int timer;
 unsigned mark;
 pid_t pawn;
 int stay;
 int tend;
 int omit;
 int down;
 int wake;
 int work;
 int done;
 int dead;
 int wait;
 int wild;
 long long tick;
 long long loop;
 scim_halt_data_t halt;
}* scim_root_t;

extern struct scim_form_t __form[];
extern struct scim_mode_t __mode[];
extern struct scim_step_t __step[];
extern struct scim_test_t __test[];
extern struct scim_move_t __move[];
extern struct scim_heed_t __heed[];
extern struct scim_task_t __task[];

extern struct scim_kind_t __kind[];
extern struct scim_role_t __role[];
extern struct scim_duty_t __duty[];
extern struct scim_zone_t __zone[];
extern struct scim_lane_t __lane[];
extern struct scim_port_t __port[];
extern struct scim_site_t __site[];
extern struct scim_host_t __host[];

extern struct scim_cell_t __cell[];
extern struct scim_stat_t __stat[];
extern struct scim_task_t __task[];

extern struct scim_task_team_t __aide[];
extern struct scim_task_team_t __ward[];

extern struct scim_task_team_t __tasks[];
extern struct scim_task_team_t __cells[];

extern scim_lane_t __lanes[][_LANE_CODE_SUM];

extern scim_form_t __form_root;
extern scim_form_t __form_host;
extern scim_form_t __form_cell;
extern scim_form_t __form_task;

extern scim_mode_t __mode_rest;
extern scim_mode_t __mode_wake;
extern scim_mode_t __mode_ward;
extern scim_mode_t __mode_free;

extern scim_step_t __step_down;
extern scim_step_t __step_wake;
extern scim_step_t __step_work;
extern scim_step_t __step_done;

extern scim_test_t __test_okay;
extern scim_test_t __test_busy;
extern scim_test_t __test_fail;

extern scim_halt_data_t __poweroff;
extern scim_halt_data_t __reboot;

extern scim_site_data_t __cell_link;
extern scim_site_data_t __host_link;
extern scim_site_data_t __cell_site;
extern scim_site_data_t __host_site;

extern char** environ;
extern char* __progname;

#include "api.h"

#endif /* __SCIM_H__ */
