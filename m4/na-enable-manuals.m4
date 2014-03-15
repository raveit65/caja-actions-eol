# Caja-Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The MATE Foundation
# Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
#
# Caja-Actions is free software; you can redistribute it and/or
# modify it under the terms of the GNU General  Public  License  as
# published by the Free Software Foundation; either  version  2  of
# the License, or (at your option) any later version.
#
# Caja-Actions is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even  the  implied  warranty  of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  the  GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public  License
# along with Caja-Actions; see the file  COPYING.  If  not,  see
# <http://www.gnu.org/licenses/>.
#
# Authors:
#   Frederic Ruaudel <grumz@grumz.net>
#   Rodrigo Moya <rodrigo@mate-db.org>
#   Pierre Wieser <pwieser@trychlos.org>
#   ... and many others (see AUTHORS)

# serial 3 add 'msg_' prefixed messages

dnl --enable-html-manuals[=db2html]
dnl   generates HTML manuals for all locales
dnl   --enable-html-manuals=db2html
dnl   in this case, fail if the specified tool is not found
dnl
dnl --enable-pdf-manuals[=dblatex]
dnl   output PDF manuals for all locales
dnl   only use dblatex for now
dnl   only '=dblatex' option is recognized for now.
dnl
dnl usage:  NA_ENABLE_MANUALS

AC_DEFUN([NA_ENABLE_MANUALS],[
	AC_REQUIRE([_AC_ARG_NA_ENABLE_HTML_MANUALS])dnl
	AC_REQUIRE([_AC_ARG_NA_ENABLE_PDF_MANUALS])dnl
	
	_CHECK_FOR_HTML_MANUALS
	_CHECK_FOR_PDF_MANUALS
])

AC_DEFUN([_AC_ARG_NA_ENABLE_HTML_MANUALS],[
	AC_ARG_ENABLE(
		[html-manuals],
		AC_HELP_STRING(
			[--enable-html-manuals@<:@=db2html@:>@],
			[build HTML user's manuals @<:@db2html@:>@]),
			[enable_html_manuals=$enableval],
			[enable_html_manuals="no"])
])

AC_DEFUN([_CHECK_FOR_HTML_MANUALS],[
	AC_MSG_CHECKING([whether to build HTML manuals])
	msg_html_manuals="disabled"
	if test "x${enable_html_manuals}" = "xno"; then
		AC_MSG_RESULT([no])
	else
		AC_MSG_RESULT([yes])
		if test "x${enable_html_manuals}" = "xyes"; then
			AC_CHECK_PROG([with_db2html],[db2html],[yes],[no])
		else
			AC_MSG_ERROR([${enable_html_manuals} is not a known tool, must be 'db2html'])
		fi
		if test "x${with_db2html}" = "xno"; then
			AC_MSG_ERROR([db2html have not been found, unable to generate HTML manuals])
		fi
		msg_html_manuals="enabled with"
		msg_html_manuals="${msg_html_manuals} db2html"
	fi

	AC_SUBST([WITH_DB2HTML],[${with_db2html}])
	AM_CONDITIONAL([ENABLE_HTML_MANUALS], [test "x${enable_html_manuals}" != "xno"])
])

AC_DEFUN([_AC_ARG_NA_ENABLE_PDF_MANUALS],[
	AC_ARG_ENABLE(
		[pdf-manuals],
		AC_HELP_STRING(
			[--enable-pdf-manuals@<:@=dblatex@:>@],
			[build PDF user's manuals @<:@dblatex@:>@]),
			[enable_pdf_manuals=$enableval],
			[enable_pdf_manuals="no"])
])

AC_DEFUN([_CHECK_FOR_PDF_MANUALS],[
	AC_MSG_CHECKING([whether to build PDF manuals])
	msg_pdf_manuals="disabled"
	if test "x${enable_pdf_manuals}" = "xno"; then
		AC_MSG_RESULT([no])
	else
		AC_MSG_RESULT([yes])
		if test "x${enable_pdf_manuals}" = "xyes"; then
			AC_CHECK_PROG([with_dblatex],[dblatex],[yes],[no])
			msg_pdf_manuals="enabled with dblatex"
		elif test "x${enable_pdf_manuals}" = "xdblatex"; then
			AC_CHECK_PROG([with_dblatex],[dblatex],[yes],[no])
			msg_pdf_manuals="enabled with dblatex"
		else
			AC_MSG_ERROR([${enable_pdf_manuals} is not a known tool, must be 'dblatex'])
		fi
		if test "x${with_dblatex}" = "xno"; then
			AC_MSG_ERROR([dblatex has not been found, unable to generate PDF manuals])
		fi
	fi

	AM_CONDITIONAL([ENABLE_PDF_MANUALS], [test "x${enable_pdf_manuals}" != "xno"])
])
