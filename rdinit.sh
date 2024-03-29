#!/bin/bash

export PATH=/bin:/sbin:/usr/bin:/usr/sbin
shopt -s nullglob

declare bit=`getconf LONG_BIT`
declare glibc=`getconf GNU_LIBC_VERSION|cut -f2 -d\ `
declare linux=`uname -r`
declare machine=`uname -m`
declare system=x$bit-linux-$linux-glibc-$glibc
declare modules=(dummy ipv6 loop ocfs2 rtc-cmos rxdsk squashfs unix zram)

system-init()
{
	echo Booting. Please wait ...

	#
	# Make sure we are at the root directory.
	#
	cd /

	mount -t devtmpfs none dev
	mount -t proc none proc
	mount -t sysfs none sys
	mount -o mode=0755 -t tmpfs none run

	install -d -m 0755 dev/pts
	install -d -m 1777 dev/{mqueue,shm}

	ln -s /proc/self/fd dev/fd
	ln -s fd/0 dev/stdin
	ln -s fd/1 dev/stdout
	ln -s fd/2 dev/stderr
	ln -s /var/run/log dev/log

	#
	# Set kernel parameters.
	#
	sysctl -q -p

	#
	# Override the configuration file hostname value.
	#
	if test -n "$hostname" && ! grep -qw "$hostname" /etc/sysctl.conf; then
		echo $hostname > proc/sys/kernel/hostname; fi

	#
	# Export the hostname.
	#
	read HOSTNAME < proc/sys/kernel/hostname
	read DOMAINNAME < proc/sys/kernel/domainname

	export HOSTNAME DOMAINNAME

	return
}

module-start()
{
	local module
	local OPTIONS

	type -p modprobe > proc/sys/kernel/modprobe
	echo -n "" > proc/sys/kernel/hotplug

	#
	# Update module dependencies.
	#
	depmod -a

	#
	# Load modules that can't be auto loaded.
	#
	if test ! -d /etc/module; then
		install -d -m 0755 /etc/module; fi

	for module in ${modules[@]}; do
		test -f /etc/module/$module || : > /etc/module/$module; done

	for file in /etc/module/*; do
		source $file
		modprobe ${file##*/} ${OPTIONS[@]}
		done

	return
}

network-setup()
{
	declare -i lane=1

	for bone in lan san dmz isp; do
		ip link add name ${bone}loop type dummy
		ip link set group $lane ${bone}loop
		((lane++))
		done

	return
}

rootfs-setup()
{
	declare unit=1

	echo "rxdsk attach 0 $(stat -c %b boot/sysroot)" > proc/rxctl
	echo "$((32 * 1048576))" > sys/block/zram0/disksize

	dd if=boot/sysroot of=dev/rxd0
	mount -r -t squashfs /dev/rxd0 mnt

	xz -dc < boot/sysdata > dev/zram0
	losetup -P /dev/loop0 /dev/zram0

	for dir in etc root srv var; do
		mount -n /dev/loop0p$unit mnt/$dir
		((unit++))
		done

	return
}

rootfs-switch()
{
	#
	# Move already mounted /boot to the new /.
	#
	#for mountpoint in boot; do
	#	if mountpoint -q $mountpoint; then
	#		mount --move mountpoint mnt/$mountpoint
	#		fi
	#	done

	#
	# Switch to the new root filesystem.
	#
	if mountpoint -q /mnt; then
		exec /sbin/switch_root /mnt ${init:-/sbin/init} "$@"
		fi

	#
	# Cannot run init.
	#
	sulogin -e -p /dev/console

	return
}

#
# Setup the console properly.
#
stty -nl -ixon ignbrk -brkint 0>&1

system-init
module-start
rootfs-setup
network-setup
rootfs-switch
