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

declare -r bin_programs=(bash cat chroot cp cpio cut dd getconf grep head install ln ls mount mount.ocfs2 mountpoint mv rm rxadm stat stty truncate udevd udevadm umount uname xz)
declare -r sbin_programs=(arping depmod ip insmod kmod losetup mke2fs modprobe sulogin switch_root swapon sysctl init)
declare -r programs=(${bin_programs[@]} ${sbin_programs[@]})
declare -r modules=(configfs crc16 dummy ext4 fuse ipv6 jbd2 loop ocfs2_stackglue quota_tree ocfs2_nodemanager ocfs2 mbcache rtc-cmos rxdsk squashfs tun unix zram)

shopt -s nullglob

install-program-library()
{
	declare library
	declare libdir=initrd/lib$bit
	declare type=`file -b --mime-type $1`
	declare strip=`type -p strip`
	export LD_LIBRARY_PATH=$PWD/lib$bit:$PWD/usr$bit/lib

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
			if test -f ${library:1}; then
				cp --link ${library:1} $libdir/${library##*/}
			elif test -f $library; then
				cp --link $library $libdir/${library##*/}
			fi

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

	install -d -m 0755 initrd/{boot/linux,bin,dev,lib/{firmware,scim},lib$bit/{modules/$linux,udev},mnt,net,proc,run,root,sbin,srv,sys,var/{cache,db,lib,log,run,spool,state},vol}
	install -d -m 1777 initrd/tmp
	
	cp --archive --link etc initrd/
	cp --link lib/scim/rdinit initrd/lib/scim/
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

extlinux-stanza-create()
{
	cat <<-EOF > boot/syslinux/${1}.conf.$$
	LABEL $1
	 MENU DEFAULT
	 MENU LABEL linux $linux - glibc $glibc $bit bit ${2:-$1}
	 APPEND quiet rdinit=/lib/scim/rdinit
	 INITRD /boot/linux/root.cpio.xz,/boot/linux/${1}.cpio.xz
	 KERNEL /boot/linux/kernel.xz
	EOF

	mv boot/syslinux/${1}.conf{.$$,}

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
	 MENU LABEL linux $linux - glibc $glibc $bit bit ${2:-default}
	 APPEND quiet rdinit=/lib/scim/rdinit
	 INITRD /boot/linux/root.cpio.xz
	 KERNEL /boot/linux/kernel.xz

	EOF

	if test -f boot/linux/${HOSTNAME}.cpio.xz; then
		cat <<-EOF >> boot/syslinux/${1}.conf.$$
		LABEL ${HOSTNAME}
		 MENU DEFAULT
		 MENU LABEL linux $linux - glibc $glibc $bit bit ${HOSTNAME}
		 APPEND quiet rdinit=/lib/scim/rdinit
		 INITRD /boot/linux/root.cpio.xz,/boot/linux/${HOSTNAME}.cpio.xz
		 KERNEL /boot/linux/kernel.xz
		EOF
		fi

	mv boot/syslinux/${1:-extlinux}.conf{.$$,}

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
	 APPEND quiet rdinit=/lib/scim/rdinit
	 INITRD /linux/root.cpio.xz,/linux/data.cpio.xz
	 KERNEL /linux/kernel.xz
	EOF

	mv boot/pxelinux.cfg/${1:-default}{.$$,}

	return
}

change-archive-create()
{
	install -d /tmp/boot/linux
	mount --bind /dev/system /tmp/boot/linux

	(cd /tmp; echo boot/linux/data|$cpio) | $xz > $bootdir/${HOSTNAME}.cpio.xz.$$

	umount /tmp/boot/linux
	rm -r /tmp/boot/linux

	mv $bootdir/${HOSTNAME}.cpio.xz{.$$,}
	extlinux-create

	return
}

kernel-archive-create()
{
	cp --link --update lib$bit/modules/$linux/bzImage $bootdir/kernel.xz
	return
}

system-image-create()
{
	mksquashfs ${image[0]} $bootdir/${image[1]:-${image[0]}} \
		-b 1048576 \
		-noappend \
		-no-progress \
		-comp xz \
		-Xbcj x86 \
		-Xdict-size 1024K \
		2> /dev/null

	return
}

data-content-gather()
{
	echo run
	find etc var -xdev -path var/lib/pckg/$machine-linux -prune -o -print
	find root -maxdepth 1
	find srv -maxdepth 1 -xdev
	find srv/{bind,dhcp,ipsec,nfs,quagga}
	return
}

data-image-create()
{
	declare unit=0
	declare size=$((1048576 * 48))

	install -d -m 0755 dat
	rm -f boot/linux/data

	echo $size > /sys/block/zram$unit/disksize

	sbin/mkfs.ocfs2 -b 4096 -F -J size=4M -L data -M local -T mail --fs-feature-level=max-features -q /dev/zram$unit || exit 1
	sbin/mount.ocfs2 /dev/zram$unit dat || exit 1

	install -d -m 0755 dat/localhost
	cpio -p dat/localhost < <(data-content-gather)

	umount dat

	#dd if=/dev/zram$unit of=boot/linux/data
	xz -9 < /dev/zram$unit > boot/linux/data

	echo 1 > /sys/block/zram$unit/reset
	echo 0 > /sys/block/zram$unit/disksize

	rm -f initrd/boot/linux/data

	return
}

data-archive-create()
{
	$cpio < <(echo boot/linux/data) | $xz > $bootdir/data.cpio.xz
	return
}

root-image-create()
{
	initrd-create

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

	return
}

root-archive-create()
{
	rm -f initrd/boot/linux/{data,root}
	ln boot/linux/{data,root} initrd/boot/linux/

	cd initrd
	$cpio -O ../$bootdir/root.cpio.$$ < <(find -mindepth 1 -printf "%P\n")
	cd ..

	$xz $bootdir/root.cpio.$$
	mv $bootdir/root.cpio.{$$.,}xz

	cp --link --update lib$bit/modules/$linux/bzImage $bootdir/kernel.xz

	return
}

all-archive-create()
{
	data-image-create
	data-archive-create

	root-image-create
	root-archive-create

	extlinux-create
	pxelinux-create

	return
}

while (($# > 0)); do
	case $1 in
	--all|-A)
		function=all-archive-create
		;;

	--kernel|-K)
		function=kernel-archive-create
		;;

	--maiden|-M)
		function=maiden-archive-create
		;;

	--system|-S)
		function=root-archive-create
		;;

	--image|-I)
		function=system-image-create
		image=($1 $2)
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
