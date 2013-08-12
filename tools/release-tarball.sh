#!/bin/ksh
# Nautilus-Actions
# A Nautilus extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The GNOME Foundation
# Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
#
# Nautilus-Actions is free software; you can redistribute it and/or
# modify it under the terms of the GNU General  Public  License  as
# published by the Free Software Foundation; either  version  2  of
# the License, or (at your option) any later version.
#
# Nautilus-Actions is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even  the  implied  warranty  of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  the  GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public  License
# along with Nautilus-Actions; see the file  COPYING.  If  not,  see
# <http://www.gnu.org/licenses/>.
#
# Authors:
#   Frederic Ruaudel <grumz@grumz.net>
#   Rodrigo Moya <rodrigo@gnome-db.org>
#   Pierre Wieser <pwieser@trychlos.org>
#   ... and many others (see AUTHORS)

errs=0										# will be the exit code of the script
my_cmd="${0}"								# e.g. "./make-ks.sh"
my_parms="$*"								# e.g. "-host toaster"
my_cmdline="${my_cmd} ${my_parms}"
me="${my_cmd##*/}"							# e.g. "make-ks.sh"
											# used in msg and msgerr functions
my_tmproot="/tmp/$(echo ${me} | sed 's?\..*$??').$$"
											# e.g. "/tmp/make-ks.1978"

# These three functions must be defined using the name() syntax in order
# to share traps with the caller process (cf. man (1) ksh).
#
trap_exit()
{
	clear_tmpfiles
	[ "${opt_verbose}" = "yes" -o ${errs} -gt 0 ] && msg "exiting with code ${errs}"
	exit ${errs}
}

trap_int()
{
	msg "quitting on keyboard interrupt"
	let errs+=1
	exit
}

trap_term()
{
	[ "${opt_verbose}" = "yes" ] && msg "quitting on TERM signal"
	exit
}

# setup the different trap functions
trap 'trap_term' TERM
trap 'trap_int'  INT
trap 'trap_exit' EXIT

clear_tmpfiles()
{
	\rm -f ${my_tmproot}.*
}

msg()
{
	typeset _eol="\n"
	[ $# -ge 2 ] && _eol="${2}"
	printf "[%s] %s${_eol}" ${me} "${1}"
	return 0
}

msgerr()
{
	msg "error: ${1}" 1>&2
	return $?
}

msgwarn()
{
	msg "warning: ${1}" 1>&2
	return $?
}

msg_help()
{
	msg_version
	echo "
 This script releases a new Nautilus-Actions version.

 Usage: ${my_cmd} [options]
   --[no]help                print this message, and exit [${opt_help_def}]
   --[no]version             print script version, and exit [${opt_version_def}]
   --[no]dummy               dummy execution [${opt_dummy_def}]
   --[no]verbose             runs verbosely [${opt_verbose_def}]
   --tarname=<tarname>       the tarname to be released [${opt_tarname_def}]
   --[no]stable              whether this is a stable version [${opt_stable_def}]"
}

msg_version()
{
	pck_name="$(grep AC_INIT configure.ac | sed -e 's?,.*$??' -e 's?^.*\[\(.*\)\]?\1?')"
	pck_version=$(grep AC_INIT configure.ac | cut -d, -f2 | sed 's?^.*\[\(.*\)\]?\1?')
	echo "
 ${pck_name} v ${pck_version}
 Copyright (C) 2010, 2011, 2012 Pierre Wieser."
}

# returns version number from a tarball
# (E): 1. tarball
# (stdout): version number
#
get_version()
{
	typeset _f="${1}"
	typeset _base="${_f##*/}"
	typeset _version=$(echo ${_base} | sed "s?^.*-\(.*\)\.${sufix}?\1?")
	echo "${_version}"
}

# is the given tarball is named as a stable version ?
# yes if even (0, 2, 4, ...), no if odd
# (E): tarball
# (stdout) yes/no
#
is_stable()
{
	typeset _version="$(get_version ${1})"
	typeset _minor=$(echo ${_version} | cut -d. -f2)
	typeset -i _rest
	if [ -z "${_minor}" ]; then
		_rest=0
	else
		_rest=${_minor}%2
	fi
	typeset _stable
	[ ${_rest} -eq 0 ] && _stable="yes" || _stable="no"
	echo "${_stable}"
}

# initialize common command-line options
nbopt=$#
opt_help=
opt_help_def="no"
opt_dummy=
opt_dummy_def="yes"
opt_version=
opt_version_def="no"
opt_verbose=
opt_verbose_def="no"

# a first loop over command line arguments to detect verbose mode
while :
do
	# break when all arguments have been read
	case $# in
		0)
			break
			;;
	esac

	# get and try to interpret the next argument
	_option=$1
	shift

	# make all options have two hyphens
	_orig_option=${_option}
	case ${_option} in
		--*)
			;;
		-*)
			_option=-${_option}
				;;
		esac

	# now process options and their argument
	case ${_option} in
		--noverb | --noverbo | --noverbos | --noverbose)
			opt_verbose="no"
			;;
		--verb | --verbo | --verbos | --verbose)
			opt_verbose="yes"
				;;
	esac
done

[ "${opt_verbose}" = "yes" ] && msg "setting opt_verbose to 'yes'"

# we have scanned all command-line arguments in order to detect an
# opt_verbose option;
# reset now arguments so that they can be scanned again in main script
set -- ${my_parms}

# interpreting command-line arguments
# pck_name:  Nautilus-Actions
# product:   nautilus-actions
# version:   most recent found in builddir
thisdir=$(cd ${0%/*}; pwd)
rootdir=${thisdir%/*}
builddir="${rootdir}/_build"
sufix="tar.gz"
pck_name="$(grep AC_INIT configure.ac | sed -e 's?,.*$??' -e 's?^.*\[\(.*\)\]?\1?')"
product=$(echo ${pck_name} | tr '[:upper:]' '[:lower:]')
opt_tarname=
f="$(\ls -1 ${builddir}/${product}-*.${sufix} 2>/dev/null | head -1)"
opt_tarname_def="$(echo ${f##*/})"
version="$(get_version ${opt_tarname_def})"
opt_stable=
opt_stable_def="$(is_stable ${opt_tarname_def})"

# loop over command line arguments
pos=0
while :
do
	# break when all arguments have been read
	case $# in
		0)
			break
			;;
	esac

	# get and try to interpret the next argument
	option=$1
	shift

	# make all options have two hyphens
	orig_option=${option}
	case ${option} in
		--*)
			;;
		-*)
			option=-${option}
			;;
	esac

	# split and extract argument for options that take one
	case ${option} in
		--*=*)
			optarg=$(echo ${option} | sed -e 's/^[^=]*=//')
			option=$(echo ${option} | sed 's/=.*//')
			;;
		# these options take a mandatory argument
		# since, we didn't find it in 'option', so it should be
		# next word in the command line
		--t | --ta | --tar | --tarn | --tarna | --tarnam | --tarname)
			optarg=$1
			shift
			;;
	esac

	# now process options and their argument
	case ${option} in
		--d | --du | --dum | --dumm | --dummy)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_dummy to 'yes'"
			opt_dummy="yes"
			;;
		--h | --he | --hel | --help)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_help to 'yes'"
			opt_help="yes"
			;;
		--nod | --nodu | --nodum | --nodumm | --nodummy)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_dummy to 'no'"
			opt_dummy="no"
			;;
		--noh | --nohe | --nohel | --nohelp)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_help to 'no'"
			opt_help="no"
			;;
		--nos | --nost | --nosta | --nostab | --nostabl | --nostable)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_stable to 'no'"
			opt_stable="no"
			;;
		--noverb | --noverbo | --noverbos | --noverbose)
			;;
		--novers | --noversi | --noversio | --noversion)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_version to 'no'"
			opt_version="no"
			;;
		--s | --st | --sta | --stab | --stabl | --stable)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_stable to 'yes'"
			opt_stable="yes"
			;;
		--t | --ta | --tar | --tarn | --tarna | --tarnam | --tarname)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_tarname to '${optarg}'"
			opt_tarname="${optarg}"
			;;
		--verb | --verbo | --verbos | --verbose)
			;;
		--vers | --versi | --versio | --version)
			[ "${opt_verbose}" = "yes" ] && msg "setting opt_version to 'yes'"
			opt_version="yes"
			;;
		--*)
			msgerr "unrecognized option: '${orig_option}'"
			let errs+=1
			;;
		# positional parameters
		*)
			let pos+=1
			#if [ ${pos} -eq 1 ]; then
			#	[ "${opt_verbose}" = "yes" ] && msg "setting opt_output to '${option}'"
			#	opt_output=${option}
			#else
				msgerr "unexpected positional parameter #${pos}: '${option}'"
				let errs+=1
			#fi
			;;
	esac
done

# set option defaults
# does not work with /bin/sh ??
#set | grep -e '^opt_' | cut -d= -f1 | while read _name; do
#	if [ "$(echo ${_name} | sed 's/.*\(_def\)/\1/')" != "_def" ]; then
#		_value="$(eval echo "$"${_name})"
#		if [ "${_value}" = "" ]; then
#			eval ${_name}="$(eval echo "$"${_name}_def)"
#		fi
#	fi
#done

opt_help=${opt_help:-${opt_help_def}}
opt_dummy=${opt_dummy:-${opt_dummy_def}}
opt_verbose=${opt_verbose:-${opt_verbose_def}}
opt_version=${opt_version:-${opt_version_def}}

# if one forces the tarball without also forcing the 'stable' option
# then reconsider the default associated to the forced tarball
if [ ! -z ${opt_tarname} -a -z ${opt_stable} ]; then
	opt_stable="$(is_stable ${opt_tarname})"
fi
opt_tarname=${opt_tarname:-${opt_tarname_def}}
opt_stable=${opt_stable:-${opt_stable_def}}

if [ "${opt_help}" = "yes" -o ${nbopt} -eq 0 ]; then
	msg_help
	echo ""
	exit
fi

if [ "${opt_version}" = "yes" ]; then
	msg_version
	echo ""
	exit
fi

if [ ! -f "${builddir}/${opt_tarname}" ]; then
	msgerr "${builddir}/${opt_tarname} not found, do you have 'make distcheck' ?"
	let errs+=1
fi

if [ ${errs} -gt 0 ]; then
	msg "${errs} error(s) have been detected"
	msg "try '${my_cmd} --help' for usage"
	exit
fi

# returns the last return code which happens to be the eval one
# (E): 1. command to be executed/evaluated/displayed
# (return): return code of the command
#
command()
{
	typeset _cmd="${1}"
	typeset -i _ret=0

	if [ "${opt_dummy}" = "yes" -o "${opt_verbose}" = "yes" ]; then
		typeset _prefix=""
		[ "${opt_dummy}" = "yes" ] && _prefix="[dummy] "
		msg "  ${_prefix}${_cmd}..." " "
	fi

	if [ "${opt_dummy}" = "no" ]; then
		eval ${_cmd}
	fi

	if [ "${opt_verbose}" = "yes" ]; then
		eval ${_cmd}
		let _ret=$?
	fi
	
	if [ "${opt_dummy}" = "yes" -o "${opt_verbose}" = "yes" ]; then
		[ ${_ret} -eq 0 ] && echo "OK" || echo "NOT OK"
	fi
	
	let errs+=${_ret}
	return ${_ret}
}

# ---------------------------------------------------------------------
# MAIN CODE

[ "${opt_stable}" = "yes" ] && lib_stable="stable" || lib_stable="unstable"
msg "releasing ${lib_stable} ${opt_tarname}"

msg "  are you OK to release (y/N) ?" " "
while [ 1 ]; do
	read -n1 -s key
	key=$(echo $key | tr '[:upper:]' '[:lower:]')
	[ "$key" = "y" -o "$key" = "n" -o "$key" = "" ] && break
done
[ "$key" = "y" ] && echo "Yes" || echo "No"
[ "$key" != "y" ] && exit

# recomputing version of the released tarball
version="$(get_version ${opt_tarname})"

# are we local ?
destdir="/net/data/tarballs/${product}"
desthost="stormy.trychlos.org"
[ "$(ls ${destdir} 2>/dev/null)" = "" ] && local="no" || local="yes"
[ "${local}" = "yes" ] && lib_desthost="" || lib_desthost="${desthost}:"
[ "${opt_verbose}" = "yes" ] && msg "stormy tarballs repository is local: ${local}"

# installing on stormy tarballs repository
msg "installing in ${lib_desthost}${destdir}"
cmd="mkdir -p "${destdir}""
[ "${local}" = "yes" ] && command "${cmd}" || command "ssh ${desthost} '${cmd}'"
command "scp "${builddir}/${opt_tarname}" "${lib_desthost}${destdir}/""
cmd="sha1sum ${destdir}/${opt_tarname} > ${destdir}/${opt_tarname}.sha1sum"
[ "${local}" = "yes" ] && command "${cmd}" || command "ssh ${desthost} '${cmd}'"
if [ "${opt_stable}" = "yes" ]; then
	msg "updating ${lib_desthost}${destdir}/latest.tar.gz"
	cmd="(cd ${destdir}; rm -f latest.tar.gz; ln -s ${opt_tarname} latest.tar.gz; ls -l latest.tar.gz ${opt_tarname}*)"
	[ "${local}" = "yes" ] && command "${cmd}" || command "ssh ${desthost} '${cmd}'"
fi

# installing on gnome.org
msg "installing on gnome.org"
command "scp "${builddir}/${opt_tarname}" pwieser@master.gnome.org:"
command "ssh pwieser@master.gnome.org ftpadmin install --unattended ${opt_tarname}"

# installing on kimsufi
msg "installing on kimsufi"
destdir="/home/www/${product}/tarballs"
command "scp "${builddir}/${opt_tarname}" maintainer@kimsufi:${destdir}/"
command "ssh maintainer@kimsufi 'sha1sum ${destdir}/${opt_tarname} > ${destdir}/${opt_tarname}.sha1sum'"
if [ "${opt_stable}" = "yes" ]; then
	msg "updating kimsufi:${destdir}/latest.tar.gz"
	command "ssh maintainer@kimsufi 'cd ${destdir}; rm -f latest.tar.gz; ln -s ${opt_tarname} latest.tar.gz; ls -l latest.tar.gz ${opt_tarname}*'"
	msg "installing manuals on kimsufi"
	command "ssh maintainer@kimsufi 'tools/kimsufi-install-manuals.sh -nodummy'"
fi

# tagging git
msg "tagging git"
tag="$(echo ${product}-${version} | tr '[:lower:]' '[:upper:]' | sed 's?[-\.]?_?g')"
msg="Releasing ${pck_name} ${version}"
msg "git tag -s '${tag}' -m '${msg}'"
command "git tag -s '${tag}' -m '${msg}'"
command "git pull --rebase && git push && git push --tags"

# compressing git local repository
msg "compressing local git repository"
command "git gc"

msg "Successfully ended. You may now send your mail."

exit

