#!/bin/bash

declare -r team_size=8
declare -r task_list=task.txt
declare -r host_list=host.txt
declare -r crew_list=crew.txt
declare -r port_list=port.txt
declare -r aide_list=aide.txt
declare -r ward_list=ward.txt

exec > srvc_code.h

slot=0

echo "enum {"

while read -a line; do
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}
	SLOT=$(printf "%02X" $slot)

	echo -e " _TASK_$NAME,"


	((slot++))
	done < $task_list

echo " _TASK_UNKNOWN"
echo "};"
echo

task=0
slot=0

echo "typedef enum srvc_task_slot_t {"

while read -a line; do
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}
	SLOT=$(printf "%02X" $slot)

	if test $slot -eq 0; then
		echo -e " _TASK_SLOT_MIN = $slot,"
		echo -e " _TASK_SLOT_$SLOT = _TASK_SLOT_MIN,"
	else
		echo -e " _TASK_SLOT_$SLOT,"; fi


	((slot++))
	done < $task_list

echo " _TASK_SLOT_MAX = _TASK_SLOT_$SLOT,"
echo " _TASK_SLOT_SUM = (_TASK_SLOT_MAX + 1)"
echo "} srvc_task_slot_t;"
echo

slot=0

echo "typedef enum srvc_host_slot_t {"

while read -a line; do
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}
	SLOT=$(printf "%02X" $slot)

	if test $slot -eq 0; then
		echo -e " _HOST_SLOT_MIN = _TASK_SLOT_SUM,"
		echo -e " _HOST_SLOT_$SLOT = _HOST_SLOT_MIN,"
	else
		echo -e " _HOST_SLOT_$SLOT,"; fi

	((slot++))
	done < $host_list

echo " _HOST_SLOT_MAX = _HOST_SLOT_$SLOT,"
echo " _HOST_SLOT_SUM = (_HOST_SLOT_MIN - _HOST_SLOT_MAX + 1),"
echo "} srvc_host_slot_t;"
echo

slot=0

echo "typedef enum srvc_host_code_t {"

while read -a line; do
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}
	CODE=$(printf "%02X" $slot)

	if test $slot -eq 0; then
		echo -e " _HOST_CODE_MIN = 0,"
		echo -e " _HOST_CODE_$CODE = _HOST_CODE_MIN,"
	else
		echo -e " _HOST_CODE_$CODE,"; fi

	((slot++))
	done < $host_list

echo " _HOST_CODE_MAX = _HOST_CODE_$CODE,"
echo " _HOST_CODE_SUM = (_HOST_CODE_MAX + 1),"
echo "} srvc_host_code_t;"
echo

code=0
slot=0

echo "typedef enum srvc_task_code_t {"

while read -a line; do
	CODE=$(printf "%02X" $code)
	SLOT=$(printf "%02X" $slot)

	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}

	echo -e " _TASK_CODE_$CODE = _TASK_SLOT_$SLOT,"

	((code++))
	((slot++))
	done < $task_list

slot=0

while read -a line; do
	CODE=$(printf "%02X" $code)
	SLOT=$(printf "%02X" $slot)
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}

	echo -e " _TASK_CODE_$CODE = _HOST_SLOT_$SLOT,"

	((code++))
	((slot++))
	done < $host_list

echo " _TASK_CODE_MAX = _TASK_CODE_$CODE,"
echo " _TASK_CODE_SUM = (_TASK_CODE_MAX + 1)"
echo "} srvc_task_code_t;"
echo

code=0
slot=0

echo "typedef enum srvc_soul_code_t {"

while read -a line; do
	CODE=$(printf "%02X" $code)
	SLOT=$(printf "%02X" $slot)

	#if test $code -eq 0; then
	#	echo " _TASK_SOUL_MIN = 0x80000000,"
	#	echo " _TASK_SOUL_$CODE = _TASK_SOUL_MIN,"
	#else
	#	echo " _TASK_SOUL_$CODE,"; fi

	case ${line[0]} in
	postgres)
		soul=$((code + 0x80000000))
		;;
	*)
		soul=0
		;;
	esac

	printf " _TASK_SOUL_$CODE = 0x%08x, // 0x%08x\n" $soul $((code + 0x80000000))
	((code++))
	((slot++))
	done < $task_list

slot=0

while read -a line; do
	CODE=$(printf "%02X" $code)

	#echo -e " _TASK_SOUL_$CODE,"
	#echo " _TASK_SOUL_$CODE = 0,"
	printf " _TASK_SOUL_$CODE = 0x00000000, // 0x%08x\n" $((code + 0x80000000))

	((code++))
	((slot++))
	done < $host_list

echo " _TASK_SOUL_MAX = _TASK_SOUL_$CODE,"
echo " _TASK_SOUL_SUM = (_TASK_SOUL_MAX + 1)"
echo "} srvc_soul_code_t;"
echo

code=0
slot=0

echo "typedef enum srvc_sect_code_t {"

while read -a line; do
	CODE=$(printf "%02X" $code)
	SLOT=$(printf "%02X" $slot)

	#if test $code -eq 0; then
	#	echo " _TASK_SECT_MIN = 0x80000000,"
	#	echo " _TASK_SECT_$CODE = _TASK_SECT_MIN,"
#	else
#		echo " _TASK_SECT_$CODE,"; fi

	#echo " _TASK_SECT_$CODE = 0,"
	printf " _TASK_SECT_$CODE = 0, // 0x%08x\n" $((code + 0x80000000))
	((code++))
	((slot++))
	done < $task_list

slot=0

while read -a line; do
	CODE=$(printf "%02X" $code)

	#echo -e " _TASK_SECT_$CODE,"
	#echo " _TASK_SECT_$CODE = 0,"
	printf " _TASK_SECT_$CODE = 0, // 0x%08x\n" $((code + 0x80000000))

	((code++))
	((slot++))
	done < $host_list

echo " _TASK_SECT_MAX = _TASK_SECT_$CODE,"
echo " _TASK_SECT_SUM = (_TASK_SECT_MAX + 1)"
echo "} srvc_sect_code_t;"
echo

host=0
team_sum=0

echo "typedef enum srvc_team_code_t {"

while read; do
	if ((++host % team_size == 0)); then
		CODE=$(printf "%02X" $team_sum)
		echo " _TEAM_$CODE,"
		((team_sum++))
		fi
	done < $host_list

echo " _TEAM_MAX = _TEAM_$CODE,"
echo " _TEAM_SUM = (_TEAM_MAX + 1)"
echo "} srvc_team_code_t;"
echo

exec > srvc_data.h

#
# Task Defines.
#
i=0

while read task path part form; do
	id=$(printf "%02X" $i)
	name=${task%%-*}
	name=${name##*/}
	tag=$(echo ${name##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s \"%s\"\n" _TASK_NAME_${tag} $name
	((i++))
	done < $task_list
echo

i=0

while read task path part form flag; do
	id=$(printf "%02X" $i)
	name=${task%%-*}
	tag=$(echo ${name##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s %s \"$task\"\n" _TASK_PATH_${tag} $path
	((i++))
	done < $task_list
echo

i=0

while read task path part form flag; do
	id=$(printf "%02X" $i)
	name=${task%%-*}
	tag=$(echo ${name##*/}|tr '[:lower:]' '[:upper:]')

	if test ${#flag} -ne 0; then
		printf "#define %-24s \"%s\"\n" _TASK_FLAG_${tag} "$flag"
	else
		echo -e "#define _TASK_FLAG_${tag}"; fi

	((i++))
	done < $task_list
echo

for FIELD in NAME PATH FLAG; do
	code=0

	while read task path part form flag; do
		CODE=$(printf "%02X" $code)
		NAME=${task%%-*}
		NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

		echo -e "#define _TASK_${FIELD}_$CODE _TASK_${FIELD}_$NAME"
		((code++))
		done < $task_list
	echo
	done

#
# Host Defines.
#
code=0

while read name kind role duty zone; do
	CODE=$(printf "%02X" $code)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s \"%s\"\n" _HOST_NAME_$NAME $name
	((i++))
	done < $host_list
echo

code=0

while read name kind role duty zone; do
	CODE=$(printf "%02X" $code)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s ROOTSBINDIR \"srvcd\"\n" _HOST_PATH_$NAME
	((code++))
	done < $host_list
echo

code=0

while read name kind role duty zone; do
	CODE=$(printf "%02X" $code)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	echo -e "#define _HOST_FLAG_$NAME"
	((code++))
	done < $host_list
echo

for FIELD in NAME PATH FLAG; do
	code=0

	while read name kind role duty zone; do
		CODE=$(printf "%02X" $code)
		NAME=${name%%-*}
		NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

		echo -e "#define _HOST_${FIELD}_$CODE _HOST_${FIELD}_$NAME"
		((code++))
		done < $host_list
	echo
	done

#
# Task list.
#
code=0
slot=0

echo "#define __TASK__ \\"

while read name path FORM MODE flag; do
	CODE=$(printf "%02X" $code)
	SLOT=$(printf "%02X" $slot)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	echo -e " {_TASK_CODE_$CODE, _TASK_NAME_$SLOT, _TASK_PATH_$SLOT \" \" _TASK_FLAG_$SLOT, _TASK_SOUL_$CODE, _TASK_SECT_$CODE, __form + _FORM_$FORM, __mode + _MODE_$MODE, __stat + _TASK_CODE_$CODE, NULL, __aide + _TASK_CODE_$CODE, __ward + _TASK_CODE_$CODE}, \\"
	((code++))
	((slot++))
	done < $task_list

slot=0

while read name kind role duty zone; do
	CODE=$(printf "%02X" $code)
	SLOT=$(printf "%02X" $slot)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	case $kind in
	HARDWARE)
		FORM=ROOT
		;;

	EMULATOR)
		FORM=HOST
		;;

	DISTRICT)
		FORM=CELL
		;;
	esac

	echo -e " {_TASK_CODE_$CODE, _HOST_NAME_$SLOT, _HOST_PATH_$SLOT _HOST_FLAG_$SLOT, _TASK_SOUL_$CODE, _TASK_SECT_$CODE, __form + _FORM_$FORM, __mode + _MODE_WARD, __stat + _TASK_CODE_$CODE, __host + _HOST_CODE_$SLOT, __aide + _TASK_CODE_$CODE, __ward + _TASK_CODE_$CODE}, \\"
	((code++))
	((slot++))
	done < $host_list

echo " {}"
echo

#
# Host list.
#
code=0
team=0

echo "#define __HOST__ \\"

while read name KIND ROLE DUTY ZONE; do
	CODE=$(printf "%02X" $code)
	TEAM=$(printf "%02X" $team)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	case $kind in
	HARDWARE)
		FORM=ROOT
		;;

	EMULATOR)
		FORM=HOST
		;;

	DISTRICT)
		FORM=CELL
		;;
	esac

	echo -e " {_HOST_CODE_$CODE, _HOST_NAME_$CODE, __kind + _KIND_$KIND, __role + _ROLE_$ROLE, __duty + _DUTY_$DUTY, __zone + _ZONE_$ZONE, __port + _PORT_${ZONE}_${DUTY}, __task + _HOST_SLOT_$CODE, __crew + _CREW_${ZONE}_$DUTY, __team + _TEAM_$TEAM}, \\"
	((code++))
	((++team == team_sum)) && team=0
	done < $host_list

echo " {}"
echo

#
# Task state.
#
code=0

echo "#define __STAT__ \\"

while read -a line; do
	CODE=$(printf "%02X" $code)

	echo -e " {__task + _TASK_CODE_$CODE, __mode + _MODE_WARD, __step + _STEP_DOWN, __test + _TEST_OKAY}, \\"

	((code++))
	done < $task_list


while read -a line; do
	CODE=$(printf "%02X" $code)

	echo -e " {__task + _TASK_CODE_$CODE, __mode + _MODE_WARD, __step + _STEP_DOWN, __test + _TEST_OKAY}, \\"

	((code++))
	done < $host_list

echo "{}"
echo

#
# Port lists.
#
echo "#define __PORT__ \\"

while read CODE list; do
	lane=0

	echo -n " {_PORT_$CODE, {"

	for LANE in $list; do
		test $lane -ne 0 && echo -n ", "
		echo -n "__lane + _LANE_$LANE"
		((lane++))
		done

	echo "}, $lane}, \\"
	done < $port_list

echo " {}"
echo


#
# Core lists.
#
team=0

echo "#define __TEAM__ \\"

while ((team < team_sum)); do
	host=0
	TEAM=$(printf "%02X" $team)

	echo -n " {_TEAM_$TEAM, {"

	while ((host < team_size)); do
		HOST=$(printf "%02X" $((host * team_sum + team)))

		test $host -ne 0 && echo -n ", "
		#echo -n "__host + _HOST_CODE_$HOST"
		echo -n "__task + _HOST_SLOT_$HOST"
		((host++))
		done

	echo "}, $host}, \\"
	((team++))
	done < $host_list

echo " {}"
echo

#
# Crew lists.
#
echo "#define __CREW__ \\"

while read CREW crew; do
	task=0

	echo -n " {_CREW_$CREW, {"

	for TASK in $crew; do
		TASK=$(echo $TASK|tr '[:lower:]' '[:upper:]')

		test $task -ne 0 && echo -n ", "
		echo -n "__task + _TASK_$TASK"
		((task++))
		done

	echo "}, $task}, \\"
	done < $crew_list

echo " {}"
echo


#
# Aide lists.
#
echo "#define __AIDE__ \\"

while read TASK list; do
	TASK=$(echo $TASK|tr '[:lower:]' '[:upper:]')
	aide=0

	printf " %-24s = {{" "[_TASK_$TASK]"

	for AIDE in $list; do
		AIDE=$(echo $AIDE|tr '[:lower:]' '[:upper:]')

		test $aide -ne 0 && echo -n ", "
		echo -n "__task + _TASK_$AIDE"
		((aide++))
		done

	echo "}, $aide}, \\"
	done < $aide_list

echo " {}"
echo


#
# Aide lists.
#
echo "#define __WARD__ \\"

while read TASK list; do
	TASK=$(echo $TASK|tr '[:lower:]' '[:upper:]')
	ward=0

	printf " %-24s = {{" "[_TASK_$TASK]"

	for WARD in $list; do
		WARD=$(echo $WARD|tr '[:lower:]' '[:upper:]')

		test $ward -ne 0 && echo -n ", "
		echo -n "__task + _TASK_$WARD"
		((ward++))
		done

	echo "}, $ward}, \\"
	done < $ward_list

echo " {}"
echo

