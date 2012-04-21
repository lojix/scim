#!/bin/bash

declare bit=`getconf LONG_BIT`
declare glibc=`getconf GNU_LIBC_VERSION|cut -f2 -d\ `
declare linux=`uname -r`
declare machine=`uname -m`
declare name=`hostname`
declare backup=false
declare root=/
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

	install -d -m 0755 initrd/{boot,bin,dev,lib/{firmware,scim},lib$bit/{modules/$linux,udev},mnt,net,proc,run,root,sbin,srv,sys,var/{cache,db,lib,log,run,spool,state},vol}
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

squashfs-image-create()
{
	declare subdir=$1
	declare image=${2:-$subdir}

	mksquashfs $subdir boot/$image \
		-b 1048576 \
		-noappend \
		-no-progress \
		-comp xz \
		-Xbcj x86 \
		-Xdict-size 1024K \
		2> /dev/null

	return
}

extlinux-stanza-create()
{
	cat <<-EOF > boot/syslinux/${1}.conf.$$
	LABEL $1
	 MENU DEFAULT
	 MENU LABEL linux $linux - glibc $glibc $bit bit ${2:-$1}
	 APPEND quiet rdinit=/lib/scim/rdinit
	 INITRD /boot/initrd,/boot/${1}.cpio.xz
	 KERNEL /boot/kernel
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
	 INITRD /boot/initrd
	 KERNEL /boot/kernel

	EOF

	if test -f boot/server/${HOSTNAME}; then
		cat <<-EOF >> boot/syslinux/${1:-extlinux}.conf.$$
		LABEL ${HOSTNAME}
		 MENU DEFAULT
		 MENU LABEL linux $linux - glibc $glibc $bit bit ${HOSTNAME}
		 APPEND quiet rdinit=/lib/scim/rdinit
		 INITRD /boot/initrd,/boot/server/${HOSTNAME}
		 KERNEL /boot/kernel
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
	 INITRD /boot/initrd
	 KERNEL /boot/kernel
	EOF

	mv boot/pxelinux.cfg/${1:-default}{.$$,}

	return
}

data-content-gather()
{
	find etc var -xdev -path var/lib/pckg/$machine-linux -prune -o -print
	find root -maxdepth 1
	find srv -maxdepth 1 -xdev
	find srv/{bind,dhcp,ipsec,nfs,quagga}
	return
}

data-archive-create()
{
	declare zram=zram3
	declare size=$((1048576 * 32))
	declare cylinder=$((size / 4096))
	declare unit=1

	echo 1 > /sys/block/$zram/reset
	echo $size > /sys/block/$zram/disksize

	sfdisk --no-reread -f -u S -C $cylinder -H 8 -S 1 /dev/$zram <<-EOF
	1,16376,,
	16384,16384,,
	32768,16384,,
	49152,16384,,
	EOF

	loop=`losetup -P -f --show /dev/$zram`
	install -d -m 0755 initrd/{boot,etc,root,srv,var}

	for subdir in etc root srv var; do
		mke2fs -b 1024 -L ares:$subdir -q -t ext4 ${loop}p$unit
		mount ${loop}p$unit initrd/$subdir || exit 1
		((unit++))
		done

	cpio -p initrd < <(data-content-gather)
	sync

	for subdir in etc root srv var; do
		mountpoint -q initrd/$subdir || continue
		until umount initrd/$subdir; do sleep 1; done; done

	$xz < /dev/$zram > initrd/boot/sysdata

	if test ${#1} -ne 0; then
		(cd initrd && $cpio < <(echo boot/sysdata)) > boot/$1; fi

	echo 1 > /sys/block/$zram/reset
	echo 0 > /sys/block/$zram/disksize

	losetup -d $loop

	return
}

boot-archive-create()
{
	initrd-create
	data-archive-create

	mksquashfs bin lib lib$bit sbin initrd/boot/sysroot \
		-b 1048576 \
		-noappend \
		-no-progress \
		-comp xz \
		-Xbcj x86 \
		-Xdict-size 1024K \
		-p "adm d 755 root root" \
		-p "boot d 755 root root" \
		-p "cell d 755 root root" \
		-p "config d 755 root root" \
		-p "dev d 755 root root" \
		-p "etc d 755 root root" \
		-p "home d 755 root root" \
		-p "lib/firmware d 755 root root" \
		-p "lib$bit/modules d 755 root root" \
		-p "media d 755 root root" \
		-p "mnt d 755 root root" \
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

	(cd initrd && $cpio < <(find -mindepth 1 -printf "%P\n")) | $xz > boot/initrd.$$
	mv boot/initrd{.$$,}

	cp --link --update lib$bit/modules/$linux/bzImage boot/kernel

	return
}

ware-image-create()
{
	squashfs-image-create usr
	squashfs-image-create usr$bit x$bit

	declare sector=(`stat -c %b boot/{usr,x64}`)
	declare offset=(8 $((${sector[0]} + 8)))
	declare count=$((${sector[0]} + ${sector[1]} + 8))
	declare cylinder=$((count / 8))

	dd if=/dev/zero of=boot/system bs=512 count=$count

	sfdisk --no-reread -f -u S -C $cylinder -H 8 -S 1 boot/system <<-EOF
	${offset[0]},${sector[0]},,
	${offset[1]},,,
	EOF

	dd if=boot/usr of=boot/system bs=512 seek=${offset[0]}
	dd if=boot/x$bit of=boot/system bs=512 seek=${offset[1]}

	rm -f boot/{usr,x$bit}
 
	return
}

all-archive-create()
{
	boot-archive-create
	extlinux-create
	pxelinux-create

	return
}

while (($# > 0)); do
	case $1 in
	--all|-A)
		function=all-archive-create
		;;

	--boot|-B)
		function=boot-archive-create
		;;

	--data|-D)
		function=data-archive-create
		;;

	--ware|-W)
		function=ware-image-create
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
bootdir=boot/$os

cd $root
install -d -m 0755 boot
$function
exit
