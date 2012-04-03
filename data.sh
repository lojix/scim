#!/bin/bash

declare -r srcdir=$1
declare -r team_size=8
declare -r task_list=$srcdir/task.txt
declare -r host_list=$srcdir/host.txt
declare -r crew_list=$srcdir/crew.txt
declare -r port_list=$srcdir/port.txt
declare -r aide_list=$srcdir/aide.txt
declare -r ward_list=$srcdir/ward.txt

exec > $srcdir/scim/code.h

echo "typedef enum scim_task_name_t {"

while read -a line; do
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}
	echo -e " _TASK_$NAME,"
	done < $task_list

code=0

while read -a line; do
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}
	echo -e " _TASK_$NAME,"
	((code++))
	done < $host_list

echo " _TASK_NAME_MAX = _TASK_$NAME,"
echo " _TASK_NAME_END,"
echo " _TASK_NAME_SUM"
echo "} scim_task_name_t;"
echo

code=0

echo "typedef enum scim_task_code_t {"

while read -a line; do
	CODE=$(printf "%04X" $code)
	echo -e " _TASK_$CODE,"
	((code++))
	done < $task_list

cell=0

while read -a line; do
	CODE=$(printf "%04X" $code)

	if ((cell == 0)); then
		echo " _CELL_CODE_MIN,"
		echo " _TASK_$CODE = _CELL_CODE_MIN,"
	else
		echo -e " _TASK_$CODE,"
		fi

	((cell++))
	((code++))
	done < $host_list

echo " _TASK_CODE_MAX = _TASK_$CODE,"
echo " _TASK_CODE_END,"
echo " _TASK_CODE_SUM"
echo "} scim_task_code_t;"
echo

code=0
echo "typedef enum scim_cell_code_t {"

while read -a line; do
	CODE=$(printf "%04X" $code)
	NAME=$(echo ${line[0]##*/}|tr '[:lower:]' '[:upper:]')
	NAME=${NAME%%-*}

	echo -e " _CELL_$CODE,"
	((code++))
	done < $host_list

echo " _CELL_CODE_MAX = _CELL_$CODE,"
echo " _CELL_CODE_END,"
echo " _CELL_CODE_SUM"
echo "} scim_cell_code_t;"
echo

code=0

echo "typedef enum scim_soul_code_t {"

while read -a line; do
	CODE=$(printf "%04X" $code)

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

	printf " _SOUL_$CODE = 0x%08x, // 0x%08x\n" $soul $((code + 0x80000000))
	((code++))
	done < $task_list

while read -a line; do
	CODE=$(printf "%04X" $code)

	#echo -e " _TASK_SOUL_$CODE,"
	#echo " _TASK_SOUL_$CODE = 0,"
	printf " _SOUL_$CODE = 0x00000000, // 0x%08x\n" $((code + 0x80000000))

	((code++))
	done < $host_list

echo " _SOUL_CODE_MAX = _SOUL_$CODE,"
echo " _SOUL_CODE_END,"
echo " _SOUL_CODE_SUM"
echo "} scim_soul_code_t;"
echo

code=0

echo "typedef enum scim_sect_code_t {"

while read -a line; do
	CODE=$(printf "%04X" $code)

	#if test $code -eq 0; then
	#	echo " _TASK_SECT_MIN = 0x80000000,"
	#	echo " _TASK_SECT_$CODE = _TASK_SECT_MIN,"
	#else
	#	echo " _TASK_SECT_$CODE,"; fi

	#echo " _TASK_SECT_$CODE = 0,"
	printf " _SECT_$CODE = 0, // 0x%08x\n" $((code + 0x80000000))
	((code++))
	done < $task_list

while read -a line; do
	CODE=$(printf "%04X" $code)

	#echo -e " _TASK_SECT_$CODE,"
	#echo " _TASK_SECT_$CODE = 0,"
	printf " _SECT_$CODE = 0, // 0x%08x\n" $((code + 0x80000000))
	((code++))
	done < $host_list

echo " _SECT_CODE_MAX = _SECT_$CODE,"
echo " _SECT_CODE_END,"
echo " _SECT_CODE_SUM"
echo "} scim_sect_code_t;"
echo

host=0
team_sum=0

echo "typedef enum scim_team_code_t {"

while read; do
	if ((++host % team_size == 0)); then
		CODE=$(printf "%04X" $team_sum)
		echo " _TEAM_$CODE,"
		((team_sum++))
		fi
	done < $host_list

echo " _TEAM_CODE_MAX = _TEAM_$CODE,"
echo " _TEAM_CODE_END,"
echo " _TEAM_CODE_SUM"
echo "} scim_team_code_t;"
echo

#
# Data definations.
#
exec > $srcdir/scim/data.h

echo "#define CELL_TASK(__code__) __task + _CELL_CODE_MIN + _CELL_ ## __code__"
echo

#
# Fact names.
#
while read name path part form; do
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')
	name=${name##*/}

	printf "#define %-24s \"%s\"\n" _TASK_NAME_$NAME $name
	done < $task_list

while read name kind role duty zone; do
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s \"%s\"\n" _TASK_NAME_$NAME $name
	done < $host_list
echo

#
# Fact path.
#
while read name path part form flag; do
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s %11s \"$name\"\n" _TASK_PATH_$NAME $path
	done < $task_list

while read name kind role duty zone; do
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	printf "#define %-24s ROOTSBINDIR \"srvcd\"\n" _TASK_PATH_$NAME
	done < $host_list
echo


#
# Fact flag.
#
while read task path part form flag; do
	name=${task%%-*}
	tag=$(echo ${name##*/}|tr '[:lower:]' '[:upper:]')

	if test ${#flag} -ne 0; then
		printf "#define %-24s \"%s\"\n" _TASK_FLAG_${tag} "$flag"
	else
		echo -e "#define _TASK_FLAG_${tag}"; fi
	done < $task_list

while read name kind role duty zone; do
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	echo -e "#define _TASK_FLAG_$NAME"
	done < $host_list
echo

#
# Task codes
#
for FIELD in NAME PATH FLAG; do
	code=0

	while read task path part form flag; do
		CODE=$(printf "%04X" $code)
		NAME=${task%%-*}
		NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

		echo -e "#define _TASK_${FIELD}_$CODE _TASK_${FIELD}_$NAME"
		((code++))
		done < $task_list

	while read name kind role duty zone; do
		CODE=$(printf "%04X" $code)
		NAME=${name%%-*}
		NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

		echo -e "#define _TASK_${FIELD}_$CODE _TASK_${FIELD}_$NAME"
		((code++))
		done < $host_list
	echo
	done
#
# Aide lists.
#
echo "#define __AIDE__ \\"

while read TASK list; do
	TASK=$(echo $TASK|tr '[:lower:]' '[:upper:]')
	aide=0

	printf " %-24s = {(scim_task_t[]){" "[_TASK_$TASK]"

	for AIDE in $list; do
		AIDE=$(echo $AIDE|tr '[:lower:]' '[:upper:]')

		test $aide -ne 0 && echo -n ", "
		echo -n "__task + _TASK_$AIDE"
		((aide++))
		done

	echo ", NULL}, $aide}, \\"
	done < $aide_list

printf " %-24s = {}\n\n" "[_TASK_CODE_END]"

#
# Ward lists.
#
echo "#define __WARD__ \\"

while read TASK list; do
	TASK=$(echo $TASK|tr '[:lower:]' '[:upper:]')
	ward=0

	printf " %-24s = {(scim_task_t[]){" "[_TASK_$TASK]"

	for WARD in $list; do
		WARD=$(echo $WARD|tr '[:lower:]' '[:upper:]')

		test $ward -ne 0 && echo -n ", "
		echo -n "__task + _TASK_$WARD"
		((ward++))
		done

	echo ", NULL}, $ward}, \\"
	done < $ward_list

printf " %-24s = {}\n\n" "[_TASK_CODE_END]"

#
# Task state.
#
code=0

echo "#define __STAT__ \\"

while read -a line; do
	CODE=$(printf "%04X" $code)

	echo -e " {__mode + _MODE_WARD, __step + _STEP_DOWN, __test + _TEST_OKAY}, \\"

	((code++))
	done < $task_list


while read -a line; do
	CODE=$(printf "%04X" $code)

	echo -e " {__mode + _MODE_WARD, __step + _STEP_DOWN, __test + _TEST_OKAY}, \\"

	((code++))
	done < $host_list

echo " {}"
echo

#
# Task list.
#
code=0

echo "#define __TASK__ \\"

while read name path FORM MODE flag; do
	CODE=$(printf "%04X" $code)
	NAME=${name%%-*}
	NAME=$(echo ${NAME##*/}|tr '[:lower:]' '[:upper:]')

	echo -e " {_TASK_$CODE, _TASK_NAME_$CODE, _TASK_PATH_$CODE \" \" _TASK_FLAG_$CODE, _SOUL_$CODE, _SECT_$CODE, __form + _FORM_$FORM, __mode + _MODE_$MODE, __aide + _TASK_$CODE, __ward + _TASK_$CODE, __stat + _TASK_$CODE}, \\"
	((code++))
	done < $task_list

cell=0

while read name kind role duty zone; do
	CODE=$(printf "%04X" $code)
	CELL=$(printf "%04X" $cell)
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

	echo -e " {_TASK_$CODE, _TASK_NAME_$CODE, _TASK_PATH_$CODE \" \" _TASK_FLAG_$CODE, _SOUL_$CODE, _SECT_$CODE, __form + _FORM_$FORM, __mode + _MODE_WARD, __aide + _TASK_$CODE, __ward + _TASK_$CODE, __stat + _TASK_$CODE, __cell + _CELL_$CELL}, \\"
	((cell++))
	((code++))
	done < $host_list

echo " {}"
echo

#
# Lane sets.
#
echo "#define __LANES__ \\"

while read POST list; do
	lane=0

	echo -n " [_POST_$POST] = {"

	for LANE in $list; do
		test $lane -ne 0 && echo -n ", "
		echo -n "__lane + _LANE_$LANE"
		((lane++))
		done

	echo "}, \\"
	done < $port_list

echo " {}"
echo

#
# Task sets.
#
echo "#define __TASKS__ \\"

while read POST crew; do
	task=0

	echo -n " [_POST_$POST] = {(scim_task_t[]){"

	for TASK in $crew; do
		TASK=$(echo $TASK|tr '[:lower:]' '[:upper:]')

		test $task -ne 0 && echo -n ", "
		echo -n "__task + _TASK_$TASK"
		((task++))
		done

	echo ", NULL}, $task}, \\"
	done < $crew_list

printf " %-24s = {}\n\n" "[_POST_CODE_END]"

#
# Cell sets.
#
team=0

echo "#define __CELLS__ \\"

while ((team < team_sum)); do
	host=0
	TEAM=$(printf "%04X" $team)

	echo -n " [_CELL_$TEAM] = {(scim_task_t[]){"

	while ((host < team_size)); do
		HOST=$(printf "%04X" $((host * team_sum + team)))

		test $host -ne 0 && echo -n ", "
		echo -n "CELL_TASK($HOST)"
		((host++))
		done
	echo ", NULL}, $host}, \\"
	((team++))
	done < $host_list

printf " %-24s = {}\n\n" "[_POST_CODE_END]"

#
# Cell lists.
#
code=0
team=0

echo "#define __CELL__ \\"

while read name KIND ROLE DUTY ZONE; do
	CODE=$(printf "%04X" $code)
	TEAM=$(printf "%04X" $team)
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

	echo -e " {_CELL_$CODE, __kind + _KIND_$KIND, __role + _ROLE_$ROLE, __duty + _DUTY_$DUTY, __zone + _ZONE_$ZONE, CELL_TASK($CODE), __tasks + _POST_${ZONE}_$DUTY, __lanes + _POST_${ZONE}_$DUTY}, \\"
	((code++))
	((++team == team_sum)) && team=0
	done < $host_list

echo " {}"
