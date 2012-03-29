#ifndef __NETLINK_ROUTE_H__
#define __NETLINK_ROUTE_H__

extern int netlink_open(void);

extern int netlink_exchange(int _socket, struct msghdr* _request, struct msghdr* _reply);

extern int netlink_address_four_create(int _netlink, unsigned _index, const uint32_t _address, const uint32_t _broadcast, const int _prefix, const int _scope);

extern int netlink_address_six_create(int _netlink, unsigned _index, const uint8_t _address[16], const uint8_t _anycast[16], const int _prefix, const int _scope);

extern int netlink_macvlan_create(int _netlink, const char* _name, uint8_t _mac[], unsigned _index, uint32_t _mode);

extern int netlink_link_rename(int _netlink, const char* _name, unsigned _index);

extern int netlink_link_change(int _socket, int _index, unsigned _flags, unsigned _change);

extern int netlink_link_move(int _socket, int _index, pid_t _pid);

#endif
