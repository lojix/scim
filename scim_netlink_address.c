#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "scim/netlink.h"

#define NETLINK_ADDRESS_FOUR_CREATE_REQUEST_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct ifaddrmsg)) + \
	RTA_SPACE(sizeof(4)) + \
	RTA_SPACE(sizeof(4)) + \
	RTA_SPACE(sizeof(4))

#define NETLINK_ADDRESS_FOUR_CREATE_REPLY_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct nlmsgerr)) + \
	NLA_ALIGN(sizeof(struct ifaddrmsg)) + \
	RTA_SPACE(sizeof(4)) + \
	RTA_SPACE(sizeof(4)) + \
	RTA_SPACE(sizeof(4))

#define NETLINK_ADDRESS_SIX_CREATE_REQUEST_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct ifaddrmsg)) + \
	RTA_SPACE(sizeof(16)) + \
	RTA_SPACE(sizeof(16))

#define NETLINK_ADDRESS_SIX_CREATE_REPLY_SIZE \
	NLA_ALIGN(sizeof(struct nlmsghdr)) + \
	NLA_ALIGN(sizeof(struct nlmsgerr)) + \
	NLA_ALIGN(sizeof(struct ifaddrmsg)) + \
	RTA_SPACE(sizeof(16)) + \
	RTA_SPACE(sizeof(16))

int netlink_address_four_create(int _netlink, unsigned _index, const uint32_t _address, const uint32_t _broadcast, const int _prefix, const int _scope)
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
		.msg_iovlen = 8,
		.msg_iov = (struct iovec[]) {
			// 0
			{
				.iov_len = sizeof(struct nlmsghdr),
				.iov_base = (struct nlmsghdr[]) {
					{
						.nlmsg_flags = NLM_F_ACK|NLM_F_REQUEST|NLM_F_CREATE|NLM_F_EXCL,
						.nlmsg_len = NETLINK_ADDRESS_FOUR_CREATE_REQUEST_SIZE,
						.nlmsg_pid = 0,
						.nlmsg_seq = 0,
						.nlmsg_type = RTM_NEWADDR 
					}
				}
			},
			// 1
			{
				.iov_len = sizeof(struct ifaddrmsg),
				.iov_base = (struct ifaddrmsg[]) {
					{
						.ifa_family = AF_INET,
						.ifa_prefixlen = _prefix,
						.ifa_flags = 0,
						.ifa_scope = _scope,
						.ifa_index = _index
					}
				}
			},
			// 2
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFA_LOCAL,
						.nla_len = RTA_LENGTH(4)
					}
				}
			},
			// 3
			{
				.iov_len = NLA_ALIGN(4),
				.iov_base = (uint32_t [1]){_address}
			},
			// 4
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFA_ADDRESS,
						.nla_len = RTA_LENGTH(4)
					}
				}
			},
			// 5
			{
				.iov_len = NLA_ALIGN(4),
				.iov_base = (uint32_t [1]){_address}
			},
			// 6
			{
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFA_BROADCAST,
						.nla_len = RTA_LENGTH(4)
					}
				}
			},
			// 7
			{
				.iov_len = NLA_ALIGN(4),
				.iov_base = (uint32_t [1]){_broadcast}
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
				.iov_len = NETLINK_ADDRESS_FOUR_CREATE_REPLY_SIZE,
				.iov_base = (char[NETLINK_ADDRESS_FOUR_CREATE_REPLY_SIZE]){}
			}
		}
	};

	if(netlink_exchange(_netlink, &request, &reply) < 0)
		return -1;

	return 0;
}

int netlink_address_six_create(int _netlink, unsigned _index, const uint8_t _address[16], const uint8_t _anycast[16], const int _prefix, const int _scope)
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
		.msg_iovlen = 6,
		.msg_iov = (struct iovec[]) {
			{
				// 0
				.iov_len = sizeof(struct nlmsghdr),
				.iov_base = (struct nlmsghdr[]) {
					{
						.nlmsg_flags = NLM_F_ACK|NLM_F_REQUEST|NLM_F_CREATE|NLM_F_EXCL,
						.nlmsg_len = NETLINK_ADDRESS_SIX_CREATE_REQUEST_SIZE,
						.nlmsg_pid = 0,
						.nlmsg_seq = 0,
						.nlmsg_type = RTM_NEWADDR 
					}
				}
			},
			{
				// 1
				.iov_len = sizeof(struct ifaddrmsg),
				.iov_base = (struct ifaddrmsg[]) {
					{
						.ifa_family = AF_INET6,
						.ifa_prefixlen = _prefix,
						.ifa_flags = 0,
						.ifa_scope = _scope,
						.ifa_index = _index
					}
				}
			},
			{
				// 2
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFA_LOCAL,
						.nla_len = RTA_LENGTH(16)
					}
				}
			},
			{
				// 3
				.iov_len = NLA_ALIGN(16),
				.iov_base = (uint8_t [16]){
					/*_address[0], _address[1], _address[2], _address[3], _address[4], _address[5],
					_address[6], _address[7], _address[8], _address[9], _address[10], _address[11],
					_address[12], _address[13], _address[14], _address[15]*/
				}
			},
			{
				// 4
				.iov_len = sizeof(struct nlattr),
				.iov_base = (struct nlattr[]) {
					{
						.nla_type = IFA_ADDRESS,
						.nla_len = RTA_LENGTH(16)
					}
				}
			},
			{
				// 5
				.iov_len = NLA_ALIGN(16),
				.iov_base = (uint8_t [16]){
					/*_address[0], _address[1], _address[2], _address[3], _address[4], _address[5],
					_address[6], _address[7], _address[8], _address[9], _address[10], _address[11],
					_address[12], _address[13], _address[14], _address[15]*/
				}
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
				.iov_len = NETLINK_ADDRESS_SIX_CREATE_REPLY_SIZE,
				.iov_base = (char[NETLINK_ADDRESS_SIX_CREATE_REPLY_SIZE]){}
			}
		}
	};

	char address[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, _address, address, sizeof(address));

	memcpy(request.msg_iov[3].iov_base, _address, 16);
	memcpy(request.msg_iov[5].iov_base, _address, 16);

	if(netlink_exchange(_netlink, &request, &reply) < 0) {
		return -1;
	}

	return 0;
}
