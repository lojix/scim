#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <linux/if_ether.h>
#include <net/if.h>

#include "scim/netlink.h"

#define __MODE_MACVLAN "macvlan"

#define NETLINK_MACVLAN_REPLY_SIZE \
	(NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	 NLA_ALIGN(sizeof(struct nlmsgerr)) + \
	 NLA_ALIGN(sizeof(struct ifinfomsg)) + \
	 RTA_SPACE(RTA_SPACE(sizeof(__MODE_MACVLAN)) + RTA_SPACE(RTA_SPACE(sizeof(uint32_t)))) + \
	 RTA_SPACE(sizeof(uint32_t)) + RTA_SPACE(ETH_ALEN) + RTA_SPACE(IFNAMSIZ))

#define NETLINK_LINK_CHANGE_REPLY_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct nlmsgerr)) + \
	NLA_ALIGN(sizeof(struct ifinfomsg))

#define NETLINK_LINK_MOVE_REQUEST_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct ifinfomsg)) + \
	RTA_SPACE(sizeof(uint32_t))

#define NETLINK_LINK_MOVE_REPLY_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct nlmsgerr)) + \
	NLA_ALIGN(sizeof(struct ifinfomsg)) + \
	RTA_SPACE(sizeof(uint32_t))

#define NETLINK_LINK_RENAME_REPLY_SIZE \
	(NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	 NLA_ALIGN(sizeof(struct nlmsgerr)) + \
	 NLA_ALIGN(sizeof(struct ifinfomsg)) + \
	 RTA_SPACE(IFNAMSIZ))

int netlink_macvlan_create(int _netlink, const char* _name, uint8_t _mac[], unsigned _index, uint32_t _mode)
{
	uint32_t length = NLA_ALIGN(sizeof(struct nlmsghdr)) +
		NLA_ALIGN(sizeof(struct ifinfomsg)) +
		RTA_SPACE(RTA_SPACE(sizeof(__MODE_MACVLAN)) + RTA_SPACE(RTA_SPACE(sizeof(uint32_t)))) +
		RTA_SPACE(sizeof(uint32_t)) + RTA_SPACE(ETH_ALEN) + RTA_SPACE(strlen(_name) + 1);

	struct msghdr request = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]) {
			{
				.nl_family = AF_NETLINK,
				.nl_pid = 0,
				.nl_groups = 0
			}
		},
		.msg_iovlen = 14,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = sizeof(struct nlmsghdr),
				.iov_base = (struct nlmsghdr[]) {
					{
						.nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_EXCL|NLM_F_ACK,
						.nlmsg_len = length,
						.nlmsg_pid = 0,
						.nlmsg_seq = 0,
						.nlmsg_type = RTM_NEWLINK 
					}
				}
			},
			{
				.iov_len = sizeof(struct ifinfomsg),
				.iov_base = (struct ifinfomsg[]) {
					{
						.ifi_change = 0,
						.ifi_family = AF_UNSPEC,
						.ifi_flags = 0,
						.ifi_index = 0,
						.ifi_type = 0
					}
				}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_LINKINFO,
						.nla_len = RTA_SPACE(RTA_SPACE(sizeof(__MODE_MACVLAN)) + RTA_SPACE(RTA_SPACE(sizeof(uint32_t))))
					}
				}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_INFO_KIND,
						.nla_len = RTA_LENGTH(sizeof(__MODE_MACVLAN))
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(sizeof(__MODE_MACVLAN)),
				.iov_base = (char[NLA_ALIGN(sizeof(__MODE_MACVLAN))]) {__MODE_MACVLAN}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_INFO_DATA,
						.nla_len = RTA_LENGTH(RTA_LENGTH(sizeof(uint32_t)))
					}
				}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_MACVLAN_MODE,
						.nla_len = RTA_LENGTH(sizeof(uint32_t))
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(sizeof(uint32_t)),
				.iov_base = (uint32_t[]){_mode}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_LINK,
						.nla_len = RTA_LENGTH(sizeof(uint32_t))
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(sizeof(uint32_t)),
				.iov_base = (uint32_t[]){_index}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_ADDRESS,
						.nla_len = RTA_LENGTH(ETH_ALEN)
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(ETH_ALEN),
				.iov_base = (char [NLA_ALIGN(ETH_ALEN)]){
					_mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]
				}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_IFNAME,
						.nla_len = RTA_LENGTH(strlen(_name) + 1)
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(strlen(_name) + 1),
				.iov_base = (char [IFNAMSIZ]){}
			}
		}
	};
	struct msghdr reply = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]) {
			{
				.nl_family = AF_NETLINK,
				.nl_pid = 0,
				.nl_groups = 0
			}
		},
		.msg_iovlen = 1,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = NETLINK_MACVLAN_REPLY_SIZE,
				.iov_base = (char[NETLINK_MACVLAN_REPLY_SIZE]){}
			}
		}
	};

	strcpy(request.msg_iov[13].iov_base, _name);

	if(netlink_exchange(_netlink, &request, &reply) < 0)
		return -1;

	return 0;
}

int netlink_link_rename(int _netlink, const char* _name, unsigned _index)
{
	struct msghdr request = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]) {
			{
				.nl_family = AF_NETLINK,
				.nl_pid = 0,
				.nl_groups = 0
			}
		},
		.msg_iovlen = 4,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = sizeof(struct nlmsghdr),
				.iov_base = (struct nlmsghdr[]) {
					{
						.nlmsg_flags = NLM_F_ACK|NLM_F_REQUEST,
						.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct ifinfomsg) + RTA_SPACE(strlen(_name) + 1),
						.nlmsg_pid = 0,
						.nlmsg_seq = 0,
						.nlmsg_type = RTM_NEWLINK 
					}
				}
			},
			{
				.iov_len = sizeof(struct ifinfomsg),
				.iov_base = (struct ifinfomsg[]) {
					{
						.ifi_change = 0,
						.ifi_family = AF_UNSPEC,
						.ifi_flags = 0,
						.ifi_index = _index,
						.ifi_type = 0
					}
				}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_IFNAME,
						.nla_len = RTA_LENGTH(strlen(_name) + 1)
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(strlen(_name) + 1),
				.iov_base = (char [IFNAMSIZ]){}
			}
		}
	};
	struct msghdr reply = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]) {
			{
				.nl_family = AF_NETLINK,
				.nl_pid = 0,
				.nl_groups = 0
			}
		},
		.msg_iovlen = 1,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = NETLINK_LINK_RENAME_REPLY_SIZE,
				.iov_base = (char[NETLINK_LINK_RENAME_REPLY_SIZE]){}
			}
		}
	};

	strcpy(request.msg_iov[3].iov_base, _name);

	if(netlink_exchange(_netlink, &request, &reply) < 0)
		return -1;

	return 0;
}

int netlink_link_change(int _socket, int _index, unsigned _flags, unsigned _change)
{
	struct msghdr request = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]) {
			{
				.nl_family = AF_NETLINK,
				.nl_pid = 0,
				.nl_groups = 0
			}
		},
		.msg_iovlen = 2,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = sizeof(struct nlmsghdr),
				.iov_base = (struct nlmsghdr[]) {
					{
						.nlmsg_flags = NLM_F_ACK|NLM_F_REQUEST,
						.nlmsg_len = NLA_ALIGN(sizeof(struct nlmsghdr)) + NLA_ALIGN(sizeof(struct ifinfomsg)),
						.nlmsg_pid = 0,
						.nlmsg_seq = 1,
						.nlmsg_type = RTM_NEWLINK 
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(sizeof(struct ifinfomsg)),
				.iov_base = (struct ifinfomsg[]) {
					{
						.ifi_change = _change,
						.ifi_family = AF_UNSPEC,
						.ifi_flags = _flags,
						.ifi_index = _index,
						.ifi_type = 0
					}
				}
			}
		}
	};
	struct msghdr reply = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]) {
			{
				.nl_family = AF_NETLINK,
				.nl_groups = 0,
				.nl_pid = 0
			}
		},
		.msg_iovlen = 1,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = NETLINK_LINK_CHANGE_REPLY_SIZE,
				.iov_base = (char [NETLINK_LINK_CHANGE_REPLY_SIZE]){}
			}
		}
	};

	if(netlink_exchange(_socket, &request, &reply) < 0)
		return -1;

	return 0;
}

int netlink_link_move(int _socket, int _index, pid_t _pid)
{
	struct msghdr request = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]){
			{
				.nl_family = AF_NETLINK,
				.nl_pid = 0,
				.nl_groups = 0
			}
		},
		.msg_iovlen = 4,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = NLA_ALIGN(sizeof(struct nlmsghdr)),
				.iov_base = (struct nlmsghdr[]) {
					{
						.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK,
						.nlmsg_len = NETLINK_LINK_MOVE_REQUEST_SIZE,
						.nlmsg_pid = 0,
						.nlmsg_seq = 1,
						.nlmsg_type = RTM_NEWLINK
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(sizeof(struct ifinfomsg)),
				.iov_base = (struct ifinfomsg[]) {
					{
						.ifi_family = AF_UNSPEC,
						.ifi_index = _index
					}
				}
			},
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFLA_NET_NS_PID,
						.nla_len = RTA_LENGTH(sizeof(uint32_t))
					}
				}
			},
			{
				.iov_len = NLA_ALIGN(sizeof(uint32_t)),
				.iov_base = (uint32_t[]){_pid}
			}
		},
		.msg_control = NULL,
		.msg_controllen = 0
	};
	struct msghdr reply = {
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_name = (struct sockaddr_nl[]){},
		.msg_iovlen = 1,
		.msg_iov = (struct iovec[]) {
			{
				.iov_len = NETLINK_LINK_MOVE_REPLY_SIZE,
				.iov_base = (char[NETLINK_LINK_MOVE_REPLY_SIZE]){}
			},
		}
	};

	if(netlink_exchange(_socket, &request, &reply) < 0)
		return -1;

	return 0;
}
