extern int scim_port_flag(scim_port_data_t _port, bool _rise, uint32_t _flag);

extern int scim_port_wipe(scim_port_data_t _port);

extern int scim_port_head(scim_port_data_t _port);

extern int scim_port_read(scim_port_data_t _port, const char* _name);

extern int scim_port_list(scim_port_data_t _port);

extern int scim_port_dump(scim_port_data_t _port);

extern int scim_port_pick(scim_port_data_t _port, scim_port_code_t _code, const char* _term);

extern int scim_port_make(scim_task_t _task);

extern int scim_port_wake(scim_root_t _root);

extern int scim_lane_make(scim_root_t _root);

extern int scim_link_note(scim_root_t _root);

extern int scim_link_redo(scim_root_t _root);

extern int scim_site_scan(scim_site_data_t _site, const char* _file);

extern int scim_site_dump(scim_site_data_t _site);

extern int scim_site_pick(scim_site_data_t _pick, scim_site_data_t _pool, scim_site_code_t _fact, const char* _text, bool _bool);

extern int scim_site_redo(scim_site_data_t _site);

extern int scim_site_undo(scim_site_data_t _site);

extern int scim_site_link(const char* _site, const char* _base, const char** _path);

extern int scim_crew_wake(scim_root_t _root);

extern int scim_crew_down(scim_root_t _root);

extern int scim_crew_tend(scim_root_t _root);

extern int scim_crew_cull(scim_root_t _root);

extern int scim_cell_copy(scim_task_t _task, char* _archive);

extern int scim_cell_move(const char* _path, const char* _name);

extern int scim_cell_make(scim_root_t _root, scim_task_t _task, const char* _path);

extern int scim_host_save(scim_root_t _root);

extern int scim_host_wake(scim_root_t _root);

extern int scim_host_down(scim_root_t _root);

extern int scim_tale_open(const char* _name);

extern int scim_root_wake(scim_root_t _root);

extern int scim_root_done(scim_root_t _root);

extern int scim_root_down(scim_root_t _root);

extern int scim_root_work(scim_root_t _root);

extern int scim_task_wipe(scim_task_t _task);

extern int scim_task_mark(unsigned _mark);

extern int scim_task_test(unsigned _mark, pid_t _pid);

extern int scim_task_seek(scim_stat_t _state);

extern int scim_task_reap(scim_root_t _root);

extern int scim_wait_toll(scim_root_t _root);

extern int scim_task_tend(scim_root_t _root, scim_task_t _task);

extern int scim_task_wake(scim_root_t _root, scim_task_t _task);

extern int scim_task_kill(scim_root_t _root, scim_task_t _task);

extern int scim_task_redo(scim_root_t _root, scim_task_t _task);

extern int scim_task_undo(scim_root_t _root, scim_task_t _task);

extern int scim_team_wake(scim_root_t _root);

extern int scim_team_down(scim_root_t _root);

extern int scim_team_tend(scim_root_t _root);

extern int scim_team_cull(scim_root_t _root);

extern int scim_term_post(const char* _path, const char* _term);

extern int scim_term_call(const char* _path, char* _term, size_t _room);

extern int scim_term_stow(const char* _path, const char* _term);

extern int scim_term_take(const char* _path, char* _term, size_t _room);

extern int scim_tool_call(char* _tool[], char* _environ[], const char* _input, const char* _output);

