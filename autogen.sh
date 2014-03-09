#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="caja-actions"
REQUIRED_INTLTOOL_VERSION=0.35.5

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

gtkdocize || exit 1

which mate-autogen || {
    echo "You need to install mate-common from the MATE Git"
    exit 1
}

REQUIRED_AUTOMAKE_VERSION=1.9
USE_MATE2_MACROS=1
USE_COMMON_DOC_BUILD=yes

. mate-autogen

# pwi 2012-10-12
# starting with NA 3.2.3, we let the MATE-DOC-PREPARE do its stuff, but
# get rid of the mate-doc-utils.make standard file, as we are using our
# own hacked version
# (see full rationale in docs/cact/mate-doc-utils-na.make)
rm -f mate-doc-utils.make

