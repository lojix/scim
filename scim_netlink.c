#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

int netlink_open(void)
{
	int fd;

	if((fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
		return -1;

	if(fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
		return -1;
	
	return fd;
}

int netlink_exchange(int _socket, struct msghdr* _request, struct msghdr* _reply)
{
	ssize_t result;
	struct nlmsghdr* header;
	struct nlmsgerr* message;

	if(sendmsg(_socket, _request, 0) < 0)
		return -1;

	if((result = recvmsg(_socket, _reply, 0)) < 0)
		return -1;

	header = _reply->msg_iov->iov_base;
	message = NLMSG_DATA(header);

	/*
	* Check if message->error is non-zero.
	*/
	if(message->error) {
		errno = -message->error;
		return -1;
	}

	return result;
}
