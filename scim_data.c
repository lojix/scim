#include "scim/scim.h"
#include "scim/data.h"

struct scim_kind_t __kind[] = {
 {_KIND_DISTRICT, "district"},
 {_KIND_EMULATOR, "emulator"},
 {_KIND_HARDWARE, "hardware"},
 {}
};

struct scim_role_t __role[] = {
 {_ROLE_GATEWAY, "gateway"},
 {_ROLE_STATION, "station"},
 {_ROLE_STEWARD, "steward"},
 {}
};

struct scim_duty_t __duty[] = {
 {_DUTY_CONTENT, "content"},
 {_DUTY_HOSTING, "hosting"},
 {_DUTY_MESSAGE, "message"},
 {_DUTY_PRESIDE, "preside"},
 {_DUTY_ROUTING, "routing"},
 {_DUTY_SESSION, "session"},
 {_DUTY_STORAGE, "storage"},
 {}
};

struct scim_zone_t __zone[] = {
 {_ZONE_INTRANET, "intranet"},
 {_ZONE_EXTRANET, "extranet"},
 {_ZONE_INTERNET, "internet"},
 {}
};

struct scim_post_t __post[] = {
 {_POST_INTRANET_CONTENT, "intranet-content"},
 {_POST_INTRANET_HOSTING, "intranet-hosting"},
 {_POST_INTRANET_MESSAGE, "intranet-message"},
 {_POST_INTRANET_PRESIDE, "intranet-preside"},
 {_POST_INTRANET_ROUTING, "intranet-routing"},
 {_POST_INTRANET_SESSION, "intranet-session"},
 {_POST_INTRANET_STORAGE, "intranet-storage"},
 {_POST_EXTRANET_CONTENT, "extranet-content"},
 {_POST_EXTRANET_HOSTING, "extranet-hosting"},
 {_POST_EXTRANET_MESSAGE, "extranet-message"},
 {_POST_EXTRANET_PRESIDE, "extranet-preside"},
 {_POST_EXTRANET_ROUTING, "extranet-routing"},
 {_POST_EXTRANET_SESSION, "extranet-session"},
 {_POST_EXTRANET_STORAGE, "extranet-storage"},
 {_POST_INTERNET_CONTENT, "internet-content"},
 {_POST_INTERNET_HOSTING, "internet-hosting"},
 {_POST_INTERNET_MESSAGE, "internet-message"},
 {_POST_INTERNET_PRESIDE, "internet-preside"},
 {_POST_INTERNET_ROUTING, "internet-routing"},
 {_POST_INTERNET_SESSION, "internet-session"},
 {_POST_INTERNET_STORAGE, "internet-storage"},
 {}
};

struct scim_lane_t __lane[] = {
 {_LANE_LAN, "lan", 0x0A000000, 8},
 {_LANE_SAN, "san", 0xAC1FFF00, 24},
 {_LANE_DMZ, "dmz", 0xC0A80000, 16},
 {_LANE_ISP, "isp", 0, 0},
 {}
};

struct scim_port_t __port[] = {
 {_PORT_NAME,		"name"},
 {_PORT_INDEX,		"ifindex"},
 {_PORT_ADDRESS,	"address"},
 {_PORT_FLAGS,		"flags"},
 {_PORT_MTU,		"mtu"},
 {}
};

struct scim_site_t __site[] = {
 {_SITE_DISK, "disk"},
 {_SITE_PATH, "path"},
 {_SITE_TYPE, "type"},
 {_SITE_FLAG, "flag"},
 {}
};

struct scim_form_t __form[] = {
 {_FORM_ROOT, "root"},
 {_FORM_CELL, "cell"},
 {_FORM_HOST, "host"},
 {_FORM_TASK, "task"},
 {}
};

struct scim_mode_t __mode[] = {
 {_MODE_REST, "rest"},
 {_MODE_WAKE, "wake"},
 {_MODE_WARD, "ward"},
 {_MODE_FREE, "free"},
 {}
};


struct scim_step_t __step[] = {
 {_STEP_DOWN, "down"},
 {_STEP_WAKE, "wake"},
 {_STEP_WORK, "work"},
 {_STEP_DONE, "done"},
 {}
};

struct scim_test_t __test[] = {
 {_TEST_OKAY, "okay"},
 {_TEST_BUSY, "busy"},
 {_TEST_FAIL, "fail"},
 {}
};

struct scim_move_t __move[] = {
 {_MOVE_POKE, "poke"},
 {_MOVE_REST, "rest"},
 {_MOVE_WAKE, "wake"},
 {}
};

struct scim_heed_t __heed[] = {
 {_HEED_FILE, "file"},
 {_HEED_SIGN, "sign"},
 {_HEED_PEAL, "peal"},
 {}
};

struct scim_task_team_t __aide[] = {
 __AIDE__
};

struct scim_task_team_t __ward[] = {
 __WARD__
};

struct scim_stat_t __stat[] = {
 __STAT__
};

struct scim_task_t __task[] = {
 __TASK__
};

struct scim_cell_t __cell[] = {
 __CELL__
};

struct scim_task_team_t __tasks[] = {
 __TASKS__
};

struct scim_task_team_t __cells[] = {
 __CELLS__
};

scim_lane_t __lanes[][_LANE_CODE_SUM] = {
 __LANES__
};

scim_halt_data_t __poweroff = (struct scim_halt_data_t[1]){
 {.code = 0x03091969, .type = 1, .step = '0', .time = 15, {}}
};

scim_halt_data_t __reboot = (struct scim_halt_data_t[1]){
 {.code = 0x03091969, .type = 1, .step = '6', .time = 15, {}}
};

scim_form_t __form_root = __form + _FORM_ROOT;
scim_form_t __form_cell = __form + _FORM_CELL;
scim_form_t __form_host = __form + _FORM_HOST;
scim_form_t __form_task = __form + _FORM_TASK;

scim_mode_t __mode_rest = __mode + _MODE_REST;
scim_mode_t __mode_wake = __mode + _MODE_WAKE;
scim_mode_t __mode_ward = __mode + _MODE_WARD;
scim_mode_t __mode_free = __mode + _MODE_FREE;

scim_step_t __step_down = __step + _STEP_DOWN;
scim_step_t __step_wake = __step + _STEP_WAKE;
scim_step_t __step_work = __step + _STEP_WORK;
scim_step_t __step_done = __step + _STEP_DONE;

scim_test_t __test_okay = __test + _TEST_OKAY;
scim_test_t __test_busy = __test + _TEST_BUSY;
scim_test_t __test_fail = __test + _TEST_FAIL;
