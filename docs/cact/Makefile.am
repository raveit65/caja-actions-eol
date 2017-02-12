# Caja-Actions
# A Caja extension which offers configurable context menu actions.
#
# Copyright (C) 2005 The GNOME Foundation
# Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
# Copyright (C) 2012-2017 Wolfgang Ulbrich and others (see AUTHORS)
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
#
# --enable-html-manuals (resp. --enable-pdf-manuals) are configure options
# which produce HTML (resp. PDF) formats. These options are always enabled
# when making distcheck, so that all output formats for all known locales
# are always included in the distributed tarball.
#
# Do not use the $(NULL) syntax here as this may prevent Damned-Lies
# to correctly generate POT files (cf. mail of Claude Paroz 2010-08-29
# http://mail.gnome.org/archives/mate-i18n/2010-August/msg00229.html).

@YELP_HELP_RULES@

HELP_ID = caja-actions-config-tool

HELP_FILES = \
	appendix-gnu-fdl-1.3.xml \
	articleinfo.xml \
	cact-bugs.xml \
	cact-conditions.xml \
	cact-edit.xml \
	cact-execution.xml \
	cact-export.xml \
	cact-import.xml \
	cact-menus.xml \
	cact-misc.xml \
	cact-prefs.xml \
	cact-profiles.xml \
	cact-properties.xml \
	cact-save.xml \
	cact-sorting.xml \
	cact-starting.xml \
	cact-where.xml \
	index.docbook \
    legalnotice.xml

HELP_MEDIA = $(patsubst $(srcdir)/C/%,%,$(shell \ls -1 $(srcdir)/C/figures/*.png))

HELP_LINGUAS = de el es fr sl

# just an empty value to be able to add something for html and pdf manuals
MAINTAINERCLEANFILES =

pkgdocdir = $(datarootdir)/doc/@PACKAGE@

# set to '-v' to be verbose (much verbose!)
opt_verbose =

all-local: all-manuals all-html-manuals all-pdf-manuals

all-manuals: all-html-manuals all-pdf-manuals

install-data-local: install-manuals

install-manuals: install-html-manuals install-pdf-manuals

uninstall-local: uninstall-manuals

uninstall-manuals: uninstall-html-manuals uninstall-pdf-manuals

dist-hook: dist-manuals-hook

dist-manuals-hook: dist-html-manuals-hook dist-pdf-manuals-hook

# Build HTML manuals as $(builddir)/<lang>/<module>.html
#
# For generating generating HTML manual, we use
# db2html (from docbook-utils package)
#
# The produced output is very close of what Yelp gives, with in particular
# a summary in each chapter while db2html does not produce it.
#
# When building HTML and PDF formats from XML sources, note that the
# .xml documents default to be read from <srcdir> for the C version,
# and from <builddir> for translated ones.

MANUALS_HTML = $(MANUALS_XML:%.xml=%.html)

MAINTAINERCLEANFILES += $(MANUALS_HTML)

if ENABLE_HTML_MANUALS
all-html-manuals: $(MANUALS_HTML)
else
all-html-manuals:
endif

# The rule here is: only rebuild html/ files if corresponding xml has changed
# .xml themselves are distributed, so are not modified when building a
# distribution.
#
# 'bis' (Build In Source) variable is set to 'yes' if builddir = srcdir.

$(MANUALS_HTML): %.html: %.xml
	@ftarget=$@; \
	if test "$(opt_verbose)" = ""; then echo "  HTML   $$ftarget"; fi; \
	if test "$(abs_srcdir)" = "$(abs_builddir)"; then bis="yes"; else bis="no"; fi; \
	lc=$${ftarget%/*}; \
	if test "x$(WITH_DB2HTML)" = "xyes"; then $(call exec_db2html,$$ftarget,$$lc,$$bis); fi; \
	if test "$(opt_verbose)" = ""; then echo "  HTML   $$ftarget done"; fi

# execute db2html
# 1. make target
# 2. locale
# 3. "yes" if build in source
exec_db2html = \
	cddir="$(2)"; \
	output="."; \
	fxml=$(HELP_ID).xml; \
	if test "$(2)" = "C" -a "$(3)" = "no"; then \
		cddir="."; \
		output="C"; \
		fxml=$(srcdir)/C/$$fxml; \
	fi; \
	cmd="(cd $$cddir && db2html --output $$output --nochunks $$fxml)"; \
	if test "$(opt_verbose)" != ""; then echo "$$cmd"; fi; \
	eval $$cmd >/dev/null; \
	find . -type d -name '*.junk' | xargs rm -fr

# install source .xml docs in builddir to be able to build C locale
# 1. locale
# 2. "yes" if build in source
install_xml = \
	if test "$(1)" = "C" -a "$(2)" = "no"; then $(MKDIR_P) C; cp $(srcdir)/C/*.xml C/; fi

# remove .xml docs from builddir after having built C locale
# 1. locale
# 2. "yes" if build in source
#remove_xml = \
#	if test "$(1)" = "C" -a "$(2)" = "no"; then rm -f C/*.xml; fi

# Install HTML manuals in <htmldir>/<lang>/cact/ subdirectory;
# where <htmldir> defaults to <pkgdocdir>/html
#
# Note: we do honor here --htmldir autotools configuration option.
#
# Manuals may be installed from builddir, if they just have been built,
# or from srcdir, if they were not built but come from distribution.

install-html-manuals:
	@if test "x$(htmldir)" = "x$(docdir)"; then \
		_instd="$(DESTDIR)$(pkgdocdir)/html"; \
	else \
		_instd="$(DESTDIR)$(htmldir)"; \
	fi; \
	echo "  Installing in $$_instd ..."; \
	for lc in C $(_HELP_REAL_LINGUAS); do \
		_instdir="$$_instd/$$lc/cact"; \
		if test -f "$(builddir)/$$lc/$(HELP_ID).html"; then \
			d="$(builddir)"; \
		elif test -f "$(srcdir)/$$lc/$(HELP_ID).html"; then \
			d="$(srcdir)"; \
		fi; \
		if test -f "$$d/$$lc/$(HELP_ID).html"; then \
			$(call install_html_doc,$$d,$$lc,$$_instdir); \
			$(call install_html_images,$$d,$$lc,$$_instdir,"stylesheet-images"); \
			$(call install_html_images,$$d,$$lc,$$_instdir,"admon"); \
			$(call install_html_figs,$$d,$$lc,$$_instdir); \
		fi; \
	done

# installing html document
# 1: source dir
# 2: locale
# 3: target dir
install_html_doc = \
	if test "$(opt_verbose)" = ""; then echo "    $(2)/cact/$(HELP_ID).html"; fi; \
	$(MKDIR_P) $(3); \
	cp -p $(opt_verbose) "$(1)/$(2)/$(HELP_ID).html" $(3)/

# installing html images
# 1: source dir
# 2: locale
# 3: target dir
# 4: stylesheet-images or admon
install_html_images = \
	if grep -lq $(4) "$(1)/$(2)/$(HELP_ID).html"; then \
		if test -d "$(1)/$(2)/$(4)"; then \
			if test "$(opt_verbose)" = ""; then echo "    $(2)/cact/$(4)"; fi; \
			cmd="cp -rp $(opt_verbose) \"$(1)/$(2)/$(4)\" $(3)/"; eval $$cmd; \
			if test "$(opt_verbose)" != ""; then echo $$cmd; fi; \
		fi; \
	fi

# installing user's manual figures
# rebuilding list of figures here because I'm unable to pass it as a parameter
# (it comes here as a single string in which I cannot walk through)
# 1: source dir
# 2: locale
# 3: target dir
install_html_figs = \
	if test "$(opt_verbose)" = ""; then echo "    $(2)/cact/figures"; fi; \
	figdir="$(3)/figures"; \
	if ! test -d "$$figdir"; then $(MKDIR_P) "$$figdir"; fi; \
	for fig in $(patsubst C/%,%,$(_HELP_C_MEDIA)); do \
		figsrc=$(srcdir)/$(2)/$$fig; \
		if test -f $${figsrc} -a ! -h $${figsrc}; then \
			cmd="$(INSTALL_DATA) $${figsrc} $${figdir}/"; \
		else \
			cmd="(cd $${figdir} && $(LN_S) -f ../../../C/cact/$$fig)"; \
		fi; \
		if test "$(opt_verbose)" != ""; then echo $$cmd; fi; \
		eval $$cmd; \
	done

uninstall-html-manuals:
	@if test "x$(htmldir)" = "x$(docdir)"; then \
		_instdir="$(DESTDIR)$(pkgdocdir)/html"; \
	else \
		_instdir="$(DESTDIR)$(htmldir)"; \
	fi; \
	if test -d $$_instdir; then \
		chmod -R u+w $$_instdir; \
		find $$_instdir -type d -name 'admon' | xargs rm -fr $(opt_verbose); \
		find $$_instdir -type d -name 'figures' | xargs rm -fr $(opt_verbose); \
		find $$_instdir -type d -name 'stylesheet-images' | xargs rm -fr $(opt_verbose); \
		find $$_instdir -type f -name '*.html' | xargs rm -f $(opt_verbose); \
		find $$_instdir -type d | sort -r | xargs rmdir $(opt_verbose); \
	fi

# locale figures (_HELP_LC_FILES) are distributed by mate-doc-utils
# when they exist
# - distribute admon/ or stylesheet-images/ icons
# - distribute *.html manuals

dist-html-manuals-hook:
	@for lc in C $(_HELP_REAL_LINGUAS); do \
		if test -f "$(builddir)/$$lc/$(HELP_ID).html"; then d=$(builddir); \
		elif test -f "$(srcdir)/$$lc/$(HELP_ID).html"; then d=$(srcdir); fi; \
		if test -f "$$d/$$lc/$(HELP_ID).html"; then \
			if test "$(opt_verbose)" = ""; then echo "  Distributing $$lc/$(HELP_ID).html"; fi; \
			cmd="cp $(opt_verbose) $$d/$$lc/$(HELP_ID).html $(distdir)/$$lc/"; eval $$cmd; \
			$(call dist_html_images,$$d,$$lc,"stylesheet-images"); \
			$(call dist_html_images,$$d,$$lc,"admon"); \
		fi; \
	done

# distribute html images
# 1. source dir
# 2. locale
# 3. image type (stylesheet-images or admon)
dist_html_images = \
	if grep -lq $(3) "$(1)/$(2)/$(HELP_ID).html"; then \
		if test -d "$(1)/$(2)/$(3)"; then \
			if test "$(opt_verbose)" = ""; then echo "  Distributing $(2)/$(3)"; fi; \
			cmd="cp -rp $(opt_verbose) \"$(1)/$(2)/$(3)\" $(distdir)/$(2)/"; eval $$cmd; \
		fi; \
	fi

# Build pdf manuals as $(srcdir)/<lang>/<module>.pdf
#
# The rule here is: only rebuild .pdf files if corresponding xml has changed
# .xml themselves are distributed, so are not modified when compiling from a
# distribution
#
# We only use dblatex for now

MANUALS_PDF = $(MANUALS_XML:%.xml=%.pdf)

MAINTAINERCLEANFILES += $(MANUALS_PDF)

if ENABLE_PDF_MANUALS
all-pdf-manuals: $(MANUALS_PDF)
else
all-pdf-manuals:
endif

# _HELP_LC_FILES is the list of the translated (not C) HELP_ID.xml files
# we so are sure that they exist and are up to date
# (path is .po ->[msgfmt]-> .mo ->[xml2po]-> .xml)
#
# workaround against l10n bug #664768
# pdflatex cannot generate some localized pdf
#
# As of dblatex 0.3.3:
# - always create the output pdf in the same directory that the
#   source xml
#   (but we need to cd $$lc to mkdir figures)
#
# 'bis' (Build In Source) variable is set to 'yes' if builddir = srcdir.

$(MANUALS_PDF): %.pdf: %.xml
	@ftarget=$@; \
	if test "$(opt_verbose)" = ""; then echo "  PDF    $$ftarget"; fi; \
	if test "$(abs_srcdir)" = "$(abs_builddir)"; then bis="yes"; else bis="no"; fi; \
	lc=$${ftarget%/*}; \
	$(call install_xml,$$lc,$$bis); \
	list='$(patsubst C/%,%,$(_HELP_C_MEDIA))'; \
	(cd $$lc; \
	 $(MKDIR_P) figures; \
	 for fig in $$list; do \
		if test -f "$(srcdir)/$$lc/$$fig"; then d=$$lc; else d="C"; fi; \
		if ! test -f $$fig; then \
			cmd="(cd figures && $(LN_S) -f $(srcdir)/$$d/$$fig)"; eval $$cmd; \
		fi; \
	 done; \
	 cmd="dblatex $(HELP_ID).xml"; eval $$cmd >/dev/null; \
	 if ! test -f $(HELP_ID).pdf; then \
		echo "  PDF    $$ftarget recovering from dblatex error..."; \
		tmpdir=`dblatex -d $(HELP_ID).xml 2>&1 | grep 'not removed' | awk '{ print $$1 }'`; \
		cp $$tmpdir/$(HELP_ID).pdf . || :; \
		rm -fr $$tmpdir; \
	 fi; \
	 find figures -type l | xargs rm -f; \
	 rmdir figures 2>/dev/null || :; \
	); \
	$(call remove_xml,$$lc,$$bis); \
	if test "$(opt_verbose)" = ""; then echo "  PDF    $$ftarget done"; fi

# if --pdfdir has not been specified, then we default to PKGDOCDIR/pdf
# in all case, we append the locale as a subdirectory level
#
# as pdf are monobloc documents, we do not create a per-document subdir

install-pdf-manuals:
	@if test "$(pdfdir)" = "$(docdir)"; then \
		_instdir=$(DESTDIR)$(pkgdocdir)/pdf; \
	else \
		_instdir=$(DESTDIR)$(pdfdir); \
	fi; \
	echo "  Installing in $$_instdir ... "; \
	for lc in C $(_HELP_REAL_LINGUAS); do \
		if test -f $(srcdir)/$$lc/$(HELP_ID).pdf; then d=$(srcdir); fi; \
		if test -f $(builddir)/$$lc/$(HELP_ID).pdf; then d=$(builddir); fi; \
		if test -f $$d/$$lc/$(HELP_ID).pdf; then \
			if test "$(opt_verbose)" = ""; then echo "    $$lc/$(HELP_ID).pdf"; fi; \
			if ! test -d $$_instdir/$$lc; then cmd="$(MKDIR_P) $$_instdir/$$lc"; eval $$cmd; fi; \
			cmd="$(INSTALL_DATA) \"$$d/$$lc/$(HELP_ID).pdf\" \"$$_instdir/$$lc/\""; eval $$cmd; \
			if test "$(opt_verbose)" != ""; then echo $$cmd; fi; \
		fi; \
	done

uninstall-pdf-manuals:
	@if test "$(pdfdir)" = "$(docdir)"; then \
		_instdir=$(DESTDIR)$(pkgdocdir)/pdf; \
	else \
		_instdir=$(DESTDIR)$(pdfdir); \
	fi; \
	if test -d $$_instdir; then \
		chmod -R u+w $$_instdir; \
		find $$_instdir -name '*.pdf' | xargs rm -f $(opt_verbose); \
		find $$_instdir -type d | sort -r | xargs rmdir $(opt_verbose); \
	fi

dist-pdf-manuals-hook:
	@for lc in C $(HELP_LINGUAS); do \
		if test -f $(builddir)/$$lc/$(HELP_ID).pdf; then d=$(builddir); \
		elif test -f $(srcdir)/$$lc/$(HELP_ID).pdf; then d=$(srcdir); fi; \
		if test -f "$$d/$$lc/$(HELP_ID).pdf"; then \
			if test "$(opt_verbose)" = ""; then echo "  Distributing $$lc/$(HELP_ID).pdf"; fi; \
			cmd="cp $$d/$$lc/$(HELP_ID).pdf $(distdir)/$$lc/"; eval $$cmd; \
			if test "$(opt_verbose)" != ""; then echo $$cmd; fi; \
		fi; \
	done

# According to GNU Make manual (§ 13 What Gets Cleaned):
#
# - mostlyclean deletes files we typically want to rebuild
# - clean deletes other files built with make
# - distclean deletes files built by configure (none here)
##
##  None of these target should delete any distributed file
##
# - maintainer-clean should delete other files
clean-local:
	rm -f *~
	rm -f *.stamp

distclean-local: distclean-html distclean-pdf

distclean-html:

distclean-pdf:

maintainer-clean-local: maintainer-clean-html maintainer-clean-pdf

maintainer-clean-html:
	find $(srcdir) -type d -name 'admon' | xargs rm -fr
	find $(builddir) -type l -name 'admon' | xargs rm -f
	find $(srcdir) -type d -name 'stylesheet-images' | xargs rm -fr
	find $(builddir) -type d -name 'stylesheet-images' | xargs rm -fr

maintainer-clean-pdf:

.PHONY: \
	all-manuals install-manuals uninstall-manuals dist-manuals-hook \
	all-html-manuals install-html-manuals uninstall-html-manuals dist-html-manuals-hook \
	all-pdf-manuals install-pdf-manuals uninstall-pdf-manuals dist-pdf-manuals-hook
