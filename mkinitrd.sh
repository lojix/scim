#!/bin/bash

declare bit=`getconf LONG_BIT`
declare glibc=`getconf GNU_LIBC_VERSION|cut -f2 -d\ `
declare linux=`uname -r`
declare machine=`uname -m`
declare name=`hostname`
declare backup=false
declare root=/
declare bootdir=boot/linux
declare xz="xz -z9 --check=crc32 --lzma2=dict=1MiB"
declare cpio="cpio --create --format=newc --quiet"

declare -r bin_programs=(bash cat chroot cp cpio cut getconf grep head install ln ls mount mountpoint mv rm stat stty truncate udevd udevadm umount uname xz)
declare -r sbin_programs=(arping depmod ip insmod kmod losetup mke2fs modprobe sulogin switch_root swapon sysctl init)
declare -r programs=(${bin_programs[@]} ${sbin_programs[@]})
declare -r modules=(configfs crc16 dummy ext4 fuse ipv6 jbd2 loop mbcache rtc-cmos squashfs tun unix)

shopt -s nullglob

install-program-library()
{
	declare library
	declare libdir=initrd/lib$bit
	declare type=`file -b --mime-type $1`
	declare strip=`type -p strip`

	test ${type// } = application/x-executable || return

	lib$bit/ld-$glibc.so --verify $1 || return

	if test ${#strip} -eq 0; then strip=:; fi

	while read -a line; do
		test ${#line[@]} -eq 4 || continue

		if test -L ${line[2]}; then
			 library=`readlink -f ${line[2]}`
		else
			library=${line[2]}
			fi

		if test ! -f $libdir/${library##*/}; then
			cp --link ${library:1} $libdir/${library##*/}
			ldconfig -l $libdir/${library##*/}
			fi

		done < <(LD_TRACE_LOADED_OBJECTS=1 $1)

	return
}

install-program()
{
	declare type=`file -b --mime-type $1`
	declare path=initrd/${1//usr$bit\/}

	cp --link $1 $path

	return
}   
	
initrd-create()
{   
	declare dynamic_linker=(`LD_TRACE_LOADED_OBJECTS=1 lib$bit/libc-$glibc.so|head -1`)

	PATH=bin:sbin:lib$bit/udev:$PATH

	rm -fr initrd

	install -d -m 0755 initrd/{boot/linux,bin,dev,lib/{firmware,init},lib$bit/{modules/$linux,udev},mnt,net,proc,run,root,sbin,srv,sys,var/{cache,db,lib,log,run,spool,state},vol}
	install -d -m 1777 initrd/tmp
	
	cp --archive --link etc initrd/
	cp --link lib/init/rdinit initrd/lib/init/
	cp --link lib$bit/ld-$glibc.so initrd/lib$bit/
	cp --link ${dynamic_linker:1} initrd/lib$bit/
	cp --link bin/sh initrd/bin/sh
	cp --link lib$bit/modules/$linux/modules.{order,builtin} initrd/lib$bit/modules/$linux

	ln -s ../lib$bit/modules initrd/lib/modules

	for file in ${programs[@]}; do
		if path=`type -p $file`; then
			install-program $path
			install-program-library $path
			fi
		done

	for module in ${modules[@]}; do
		file=$(find lib$bit/modules/$linux -name $module.ko.xz)
		install -D -m 0644 $file initrd/$file
		done

	sbin/depmod -b initrd $linux

	return
}

extlinux-create()
{
	mountpoint -q boot || cp --archive --link --update lib/syslinux boot

	cat <<-EOF > boot/syslinux/${1:-extlinux}.conf.$$
	DEFAULT menu.c32
	PROMPT 0
	TIMEOUT 50
	TOTALTIMEOUT 600

	MENU AUTOBOOT Please wait ...
	MENU TITLE SYSLINUX Disk Bootloader [$(pckg --root=$PWD --list syslinux)]

	LABEL maiden
	 MENU LABEL linux $linux - glibc $glibc $bit bit ${2:-maiden}
	 APPEND quiet rdinit=/lib/init/rdinit
	 INITRD /boot/linux/system.cpio.xz,/boot/linux/kernel.cpio.xz,/boot/linux/${2:-maiden}.cpio.xz
	 KERNEL /boot/linux/kernel.xz

	EOF


	for serial in $(cat /sys/class/net/eth[0-9]*/address|sort); do
		test -r boot/linux/${serial//:}.cpio.xz || continue
		echo "INCLUDE ${serial//:}.conf" >> boot/syslinux/${1:-extlinux}.conf.$$
		done

	mv boot/syslinux/${1:-extlinux}.conf{.$$,}

	return
}

extlinux-stanza-create()
{
	cat <<-EOF > boot/syslinux/${1}.conf.$$
	LABEL $1
	#MENU DEFAULT
	 MENU LABEL linux $linux - glibc $glibc $bit bit ${2:-$1}
	 APPEND quiet rdinit=/lib/init/rdinit
	 INITRD /boot/linux/system.cpio.xz,/boot/linux/kernel.cpio.xz,/boot/linux/$1.cpio.xz
	 KERNEL /boot/linux/kernel.xz
	EOF

	mv boot/syslinux/${1}.conf{.$$,}

	return
}

pxelinux-create()
{
	install -d -m 0755 boot/pxelinux.cfg
	mountpoint -q boot || cp --archive --link --update lib/syslinux/*.0 boot

	cat <<-EOF > boot/pxelinux.cfg/${1:-default}.$$
	DEFAULT syslinux/menu.c32
	PROMPT 0
	TIMEOUT 50
	TOTALTIMEOUT 600

	MENU AUTOBOOT Please wait ...
	MENU TITLE SYSLINUX PXE Bootloader [$(pckg --root=$PWD --list syslinux)]

	LABEL linux
	 MENU LABEL linux $linux - glibc $glibc $bit bit
	 APPEND quiet rdinit=/lib/init/rdinit
	 INITRD /linux/system.cpio.xz,/linux/kernel.cpio.xz,/linux/${2:-maiden}.cpio.xz
	 KERNEL /linux/kernel.xz
	EOF

	mv boot/pxelinux.cfg/${1:-default}{.$$,}

	return
}

maiden-content-gather()
{
	echo run
	find etc var -xdev -path var/lib/pckg/$machine-linux -prune -o -print
	find root -maxdepth 1
	find srv -maxdepth 1 -xdev
	find srv/{bind,dhcp,ipsec,nfs,quagga}
	return
}

maiden-archive-create()
{
	$cpio < <(maiden-content-gather) | $xz > $bootdir/maiden.cpio.xz
	ln  $bootdir/maiden.cpio.xz{,.$$}
	mv  $bootdir/maiden.cpio.xz.$$ lib/maiden.cpio.xz
	return
}

change-content-gather()
{
	#maiden-content-gather
	#find run/host \( -path run/host/$HOSTNAME -o -path run/host/localhost \) -prune -o -print
	#find run/host/localhost/adm
	cat <<-EOF
	boot/linux/data
	boot/linux/root
	EOF
	return
}

change-archive-create()
{
	declare index=1
	declare serial=(`cat /sys/class/net/eth[0-9]*/address|sort`)

	install -d /tmp/boot/linux
	mount --bind /dev/sytem /tmp/boot/linux

	(cd /tmp; $cpio) < <(change-content-gather) | $xz > $bootdir/${serial[0]//:}.cpio.xz.$$

	umount /tmp/boot/linux
	rm -r /tmp/boot/linux

	mv $bootdir/${serial[0]//:}.cpio.xz{.$$,}

	extlinux-stanza-create ${serial[0]//:}

	while ((index < ${#serial[@]})); do
		ln $bootdir/${serial[0]//:}.cpio.xz $bootdir/${serial[index]//:}.cpio.xz.$$
		mv $bootdir/${serial[index]//:}.cpio.xz{.$$,}

		extlinux-stanza-create ${serial[index]//:}
		((index++))
	done

	extlinux-create

	sed -i -e '/MENU DEFAULT/s/#//' boot/syslinux/${serial[0]//:}.conf

	return
}

kernel-archive-create()
{
	$cpio < <(find lib/firmware lib$bit/modules/$linux) | \
		$xz > $bootdir/kernel.cpio.xz

	cp --link --update lib$bit/modules/$linux/bzImage $bootdir/kernel.xz

	return
}

app-image-create()
{
	mksquashfs usr usr32 usr64 $bootdir/app.squashfs \
		-b 1048576 \
		-noappend \
		-no-progress \
		-comp xz \
		-Xbcj x86 \
		-Xdict-size 1024K \
		2> /dev/null

	return
}

data-image-create()
{
	truncate -s 32M boot/linux/data
	mke2fs -F -L data -q -t ext4 boot/linux/data

	install -d -m 0755 dat
	mount -o loop boot/linux/data dat
	install -d -m 0755 dat/localhost
	cpio -p dat/localhost < <(maiden-content-gather)
	umount dat
	return
}

root-image-create()
{
	initrd-create

	#mksquashfs bin lib lib$bit sbin $bootdir/system.squashfs \
	mksquashfs bin lib lib$bit sbin $bootdir/root \
		-b 1048576 \
		-noappend \
		-no-progress \
		-comp xz \
		-Xbcj x86 \
		-Xdict-size 1024K \
		-p "adm d 755 root root" \
		-p "dat d 755 root root" \
		-p "boot d 755 root root" \
		-p "config d 755 root root" \
		-p "dev d 755 root root" \
		-p "etc d 755 root root" \
		-p "home d 755 root root" \
		-p "lib/firmware d 755 root root" \
		-p "lib$bit/modules d 755 root root" \
		-p "media d 755 root root" \
		-p "mnt d 755 root root" \
		-p "net d 755 root root" \
		-p "proc d 755 root root" \
		-p "root d 755 root root" \
		-p "run d 755 root root" \
		-p "srv d 755 root root" \
		-p "sys d 755 root root" \
		-p "tmp d 1777 root root" \
		-p "usr d 755 root root" \
		-p "usr32 d 755 root root" \
		-p "usr64 d 755 root root" \
		-p "var d 755 root root" \
		-p "vol d 755 root root" \
		2> /dev/null
#		-e modules \
#		-e firmware \

	return
}

system-archive-create()
{
	(cd initrd && $cpio -O ../$bootdir/system.cpio.$$) < <(find initrd -mindepth 1 -xdev -printf "%P\n")
	$cpio --append -O $bootdir/system.cpio.$$ < <(change-content-gather)
	$xz $bootdir/system.cpio.$$
	mv $bootdir/system.cpio.{$$.,}xz

	return
}

all-archive-create()
{
	maiden-archive-create

	data-image-create
	root-image-create

	system-archive-create
	kernel-archive-create

	extlinux-create
	pxelinux-create

	return
}

while (($# > 0)); do
	case $1 in
	--all|-A)
		function=all-archive-create
		;;

	--app|-P)
		function=app-image-create
		;;

	--kernel|-K)
		function=kernel-archive-create
		;;

	--maiden|-M)
		function=maiden-archive-create
		;;

	--system|-S)
		function=system-archive-create
		;;

	--change|-U)
		function=change-archive-create
		;;

	--backup)
		backup=true
		;;

	--root)
		shift
		root=$1
		;;

	--name)
		shift
		name=$1
		;;

	--linux)
		shift
		linux=$1
		;;

	--glibc)
		shift
		glibc=$1
		;;
	esac
	shift
	done

os=x$bit-linux-$linux-glibc-$glibc

cd $root
install -d -m 0755 $bootdir
$function
exit
