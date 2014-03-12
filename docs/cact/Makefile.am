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
#
# from Migrating your documentation to mate-doc-utils
# http://live.gnome.org/GnomeDocUtilsMigrationHowTo
#
# The mate-doc-utils packages produces the Caja-Actions Configuration
# Tool (CACT) Users's Manual as DocBook XML documents, suitable for Yelp,
# the Mate help system.
#
# --enable-html-manuals (resp. --enable-pdf-manuals) are configure options
# which produce HTML (resp. PDF) formats. These options are always enabled
# when making distcheck, so that all output formats for all known locales
# are always included in the distributed tarball.
#
# At install time, mate-doc-utils installs these help files
# in <datadir>/mate/help/caja-actions-config-tool/<lang>.
# Missing tranlated images are replaced with symlinks to
# ../../C/figures/<image.png>.
#
# Do not confuse mate-doc-utils which targets the help documentation
# of the Mate (i.e. gui) applications, and that we are using here to
# generate html and pdf CACT user's manuals, with gtk-doc which rather
# targets the developer documentation (see docs/reference).
#
# Do not use the $(NULL) syntax here as this may prevent Damned-Lies
# to correctly generate POT files (cf. mail of Claude Paroz 2010-08-29
# http://mail.gnome.org/archives/mate-i18n/2010-August/msg00229.html).

include $(srcdir)/mate-doc-utils-ca.make

DOC_MODULE = caja-actions-config-tool

#DOC_ENTITIES = $(patsubst $(srcdir)/C/%,%,$(shell \ls -1 $(srcdir)/C/*.xml | $(GREP) -v $(DOC_MODULE)))
DOC_ENTITIES = \
	appendix-gnu-fdl-1.3.xml \
	articleinfo.xml \
	legalnotice.xml \
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
	caja-actions-config-tool.xml

#DOC_INCLUDES =

#DOC_FIGURES = $(patsubst $(srcdir)/C/%,%,$(shell \ls -1 $(srcdir)/C/figures/*.png))
DOC_FIGURES = \
	add.png \
	cact-action-tab.png \
	cact-basenames-tab.png \
	cact-capabilities-tab.png \
	cact-command-tab.png \
	cact-desktop-environment-frame.png \
	cact-environment-tab.png \
	cact-exec-mode-frame.png \
	cact-execute-as-frame.png \
	cact-execution-environment-frame.png \
	cact-execution-tab.png \
	cact-export-done.png \
	cact-export-format.png \
	cact-export-home.png \
	cact-export-items.png \
	cact-export-summary.png \
	cact-export-target.png \
	cact-folders-tab.png \
	cact-import-done.png \
	cact-import-files.png \
	cact-import-home.png \
	cact-import-mode.png \
	cact-import-resultat.png \
	cact-import-summary.png \
	cact-legend.png \
	cact-main-empty.png \
	cact-mimetypes-tab.png \
	cact-new-capability.png \
	cact-new-scheme.png \
	cact-preferences-desktop.png \
	cact-preferences-execution.png \
	cact-preferences-export.png \
	cact-preferences-import.png \
	cact-preferences-io-providers.png \
	cact-preferences-runtime.png \
	cact-preferences-schemes.png \
	cact-preferences-ui.png \
	cact-properties-tab.png \
	cact-schemes-tab.png \
	cact-selection-count-frame.png \
	cact-startup-mode-frame.png \
	cact-target-location.png \
	cact-working-directory-frame.png \
	read-only.png \
	remove.png \
	sort-asc.png \
	sort-buttons-frame.png \
	sort-desc.png \
	sort-manual.png \
	start-context-menu-caja.png \
	start-matedialog-result.png \
	stock-icon-about.png

#DOC_LINGUAS = $(patsubst $(srcdir)/%,%,$(shell find $(srcdir) -mindepth 1 -maxdepth 1 -type d | $(GREP) -v $(srcdir)/C))
DOC_LINGUAS = \
	de \
	es \
	fr \
	sl

if ENABLE_MANUALS
# _DOC_LC_DOCS is the list of the translated (not C) DOC_MODULE.xml files
# we so are sure that they exist and are up to date
# (path is .po ->[msgfmt]-> .mo ->[xml2po]-> .xml)
MANUALS_XML = C/$(DOC_MODULE).xml $(_DOC_LC_DOCS)
endif

EXTRA_DIST = \
	mate-doc-utils-ca.make				\
	$(NULL)

MAINTAINERCLEANFILES = \
	mate-doc-utils-ca.make				\
	$(NULL)

pkgdocdir = $(datarootdir)/doc/@PACKAGE@-@VERSION@

# set to '-v' to be verbose (much verbose!)
opt_verbose =

all-local: all-manuals

all-manuals: install-C-xml all-html-manuals all-pdf-manuals

install-data-local: install-manuals

install-manuals: install-html-manuals install-pdf-manuals

uninstall-local: uninstall-manuals

uninstall-manuals: uninstall-html-manuals uninstall-pdf-manuals

# doc-dist-hook is the dist hook from mate-doc-utils.make
# it distributes both C (original) and generated (translated) .xml files,
# but not generated .omf files which so will have to be re-generated from
# the distributed .omf.in file
dist-hook: doc-dist-hook dist-manuals-hook dist-omf-hook

dist-manuals-hook: dist-html-manuals-hook dist-pdf-manuals-hook

# when building the manuals, C source files are in srcdir, while all
# other xml files are in builddir - installing C xml sources in builddir
# so that all rules apply
install-C-xml:
if ENABLE_MANUALS
	@if test ! -f C/$(DOC_MODULE).xml; then \
		echo "Installing C xml source files in $(abs_builddir)"; \
		$(MKDIR_P) C; cp -v $(srcdir)/C/*.xml C/; \
	fi
endif

remove-C-xml:
if ENABLE_MANUALS
	@echo "Removing C xml source files from $(abs_builddir)"
	@if test "$(abs_srcdir)" != "$(abs_builddir)"; then \
		rm -vf C/*.xml; \
	fi
endif

# Build HTML manuals as $(builddir)/<lang>/<module>.html
#
# We have two alternative for generating HTML manual:
# - mate-doc-tool (from mate-doc-utils package)
# - db2html (from docbook-utils package)
# The actual tool to be used may be specified as an argument of
# --enable-html-manuals configure option. It defaults to mate-doc-tool.
#
# mate-doc-tool <format> -o <dir> -d0 caja-actions-config-tool.xml
# - format: output format (html)
# - dir: output directory
# -d0: nochunks
#
# The produced output is very close of what Yelp gives, with in particular
# a summary in each chapter while db2html does not produce it.
#
# As of v 0.20.1:
# - does not produce the legal and abstract sections in top of the document
# - does not honor external links: they are converted to simple links
#   which target the same frame window.
# - embeds references to 'admon-xxx' images (which are the equivalent
#   of db2html's 'stylesheet-images' ones), but does not provide them
#   (see https://bugzilla.gnome.org/show_bug.cgi?id=636175)
#   and so we 'manually' copy them as a "admon/" subdirectory
#   the same way db2html does
# - does not honor the base directory of the source document, only
#   searching for extern entities in the current working directory
#   (see https://bugzilla.gnome.org/show_bug.cgi?id=664784)
#
# mate-doc-tool is so our preferred tool for its proximity with Yelp
# and the summary in top of the chapters.
#
# db2html --nochunks caja-actions-config-tool.xml
# Generate the html document(s).
#
# As of 0.6.14.29:
# - does produce the legal and abstract sections in top of the document
# - creates a stylesheet-images/ subdirectory
# - does not honor external links: they are converted to simple links
#   which target the same frame window
# - may create docbook2html-dir(.junk) subdirectories when it is run
#   without argument
#   (see https://bugzilla.redhat.com/show_bug.cgi?id=756930)
# - begins by emptying the target directory, even if it also contains
#   the source file
#
# When building HTML and PDF formats from XML sources, note that the
# .xml documents default to be read from <srcdir> for the C version,
# and from <builddir> for translated ones.

if ENABLE_HTML_MANUALS
MANUALS_HTML = $(MANUALS_XML:%.xml=%.html)

MAINTAINERCLEANFILES += $(MANUALS_HTML)

all-html-manuals: $(MANUALS_HTML)
else
all-html-manuals:
endif

# The rule here is: only rebuild html/ files if corresponding xml has changed
# .xml themselves are distributed, so are not modified when building from a
# distributed package.

$(MANUALS_HTML): %.html: %.xml
	@ftarget=$@; \
	if test "$(opt_verbose)" = ""; then echo "  HTML   $$ftarget"; fi; \
	lc=$${ftarget%/*}; \
	if test "x$(WITH_DB2HTML)" = "xyes"; then $(call exec_db2html,$$lc); fi; \
	if test "x$(WITH_GDT)" = "xyes"; then $(call exec_g_d_t,$$lc); fi; \
	if test "$(opt_verbose)" = ""; then echo "  HTML   $$ftarget done"; fi

# execute mate-doc-tool
# 1. locale
exec_g_d_t = \
	fxml=$(DOC_MODULE).xml; \
	(cd $(1); \
	 cmd="mate-doc-tool html -o . -d0 $$fxml"; \
	 if test "$(opt_verbose)" != ""; then echo "$$cmd"; fi; \
	 eval $$cmd >/dev/null; \
	 fhtml=$(DOC_MODULE).html; \
	 if test -f $$fhtml; then \
		sed -i -e 's?url("admon?url("admon/admon?' $$fhtml; \
		admon_dir=/usr/share/mate-doc-utils/icons/hicolor/48x48/status; \
		if test -d $$admon_dir; then \
			$(MKDIR_P) admon; \
			cmd="cp $(opt_verbose) $$admon_dir/*.png admon/"; eval $$cmd; \
		fi; \
	 fi; \
	)

# execute db2html
# 1. locale
exec_db2html = \
	cddir="$(1)"; \
	output="."; \
	fxml=$(DOC_MODULE).xml; \
	cmd="(cd $$cddir && db2html --output $$output --nochunks $$fxml)"; \
	if test "$(opt_verbose)" != ""; then echo "$$cmd"; fi; \
	eval $$cmd >/dev/null; \
	find . -type d -name '*.junk' | xargs rm -fr

# install source .xml docs in builddir to be able to build C locale
# 1. locale
# 2. "yes" if build in source
#install_xml = \
#	if test "$(1)" = "C" -a "$(2)" = "no"; then $(MKDIR_P) C; cp $(srcdir)/C/*.xml C/; fi

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
	for lc in C $(DOC_LINGUAS); do \
		_instdir="$$_instd/$$lc/cact"; \
		if test -f "$(builddir)/$$lc/$(DOC_MODULE).html"; then \
			d="$(builddir)"; \
		elif test -f "$(srcdir)/$$lc/$(DOC_MODULE).html"; then \
			d="$(srcdir)"; \
		fi; \
		if test -f "$$d/$$lc/$(DOC_MODULE).html"; then \
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
	if test "$(opt_verbose)" = ""; then echo "    $(2)/cact/$(DOC_MODULE).html"; fi; \
	$(MKDIR_P) $(3); \
	cp -p $(opt_verbose) "$(1)/$(2)/$(DOC_MODULE).html" $(3)/

# installing html images
# 1: source dir
# 2: locale
# 3: target dir
# 4: stylesheet-images or admon
install_html_images = \
	if grep -lq $(4) "$(1)/$(2)/$(DOC_MODULE).html"; then \
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
	for fig in $(patsubst C/%,%,$(_DOC_C_FIGURES)); do \
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

# locale figures (_DOC_LC_FIGURES) are distributed by mate-doc-utils
# when they exist
# - distribute admon/ or stylesheet-images/ icons
# - distribute *.html manuals

dist-html-manuals-hook:
	@for lc in C $(DOC_LINGUAS); do \
		if test -f "$(builddir)/$$lc/$(DOC_MODULE).html"; then d=$(builddir); \
		elif test -f "$(srcdir)/$$lc/$(DOC_MODULE).html"; then d=$(srcdir); fi; \
		if test -f "$$d/$$lc/$(DOC_MODULE).html"; then \
			if test "$(opt_verbose)" = ""; then echo "  Distributing $$lc/$(DOC_MODULE).html"; fi; \
			cmd="cp $(opt_verbose) $$d/$$lc/$(DOC_MODULE).html $(distdir)/$$lc/"; eval $$cmd; \
			$(call dist_html_images,$$d,$$lc,"stylesheet-images"); \
			$(call dist_html_images,$$d,$$lc,"admon"); \
		fi; \
	done

# distribute html images
# 1. source dir
# 2. locale
# 3. image type (stylesheet-images or admon)
dist_html_images = \
	if grep -lq $(3) "$(1)/$(2)/$(DOC_MODULE).html"; then \
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

# workaround against l10n bug #664768
# pdflatex cannot generate some localized pdf
#
# As of dblatex 0.3.3:
# - always create the output pdf in the same directory that the
#   source xml
#   (but we need to cd $$lc to mkdir figures)

$(MANUALS_PDF): %.pdf: %.xml
	@ftarget=$@; \
	if test "$(opt_verbose)" = ""; then echo "  PDF    $$ftarget"; fi; \
	lc=$${ftarget%/*}; \
	list='$(patsubst C/%,%,$(_DOC_C_FIGURES))'; \
	(cd $$lc; \
	 $(MKDIR_P) figures; \
	 for fig in $$list; do \
		if test -f "$(srcdir)/$$lc/$$fig"; then d=$$lc; else d="C"; fi; \
		if ! test -f $$fig; then \
			cmd="(cd figures && $(LN_S) -f $(srcdir)/$$d/$$fig)"; eval $$cmd; \
		fi; \
	 done; \
	 cmd="dblatex $(DOC_MODULE).xml"; eval $$cmd >/dev/null; \
	 if ! test -f $(DOC_MODULE).pdf; then \
		echo "  PDF    $$ftarget recovering from dblatex error..."; \
		tmpdir=`dblatex -d $(DOC_MODULE).xml 2>&1 | grep 'not removed' | awk '{ print $$1 }'`; \
		cp $$tmpdir/$(DOC_MODULE).pdf . || :; \
		rm -fr $$tmpdir; \
	 fi; \
	 find figures -type l | xargs rm -f; \
	 rmdir figures 2>/dev/null || :; \
	); \
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
	for lc in C $(DOC_LINGUAS); do \
		if test -f $(srcdir)/$$lc/$(DOC_MODULE).pdf; then d=$(srcdir); fi; \
		if test -f $(builddir)/$$lc/$(DOC_MODULE).pdf; then d=$(builddir); fi; \
		if test -f $$d/$$lc/$(DOC_MODULE).pdf; then \
			if test "$(opt_verbose)" = ""; then echo "    $$lc/$(DOC_MODULE).pdf"; fi; \
			if ! test -d $$_instdir/$$lc; then cmd="$(MKDIR_P) $$_instdir/$$lc"; eval $$cmd; fi; \
			cmd="$(INSTALL_DATA) \"$$d/$$lc/$(DOC_MODULE).pdf\" \"$$_instdir/$$lc/\""; eval $$cmd; \
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
	@for lc in C $(DOC_LINGUAS); do \
		if test -f $(builddir)/$$lc/$(DOC_MODULE).pdf; then d=$(builddir); \
		elif test -f $(srcdir)/$$lc/$(DOC_MODULE).pdf; then d=$(srcdir); fi; \
		if test -f "$$d/$$lc/$(DOC_MODULE).pdf"; then \
			if test "$(opt_verbose)" = ""; then echo "  Distributing $$lc/$(DOC_MODULE).pdf"; fi; \
			cmd="cp $$d/$$lc/$(DOC_MODULE).pdf $(distdir)/$$lc/"; eval $$cmd; \
			if test "$(opt_verbose)" != ""; then echo $$cmd; fi; \
		fi; \
	done

# Distributing .omf files is required if we want the user be able to
# get rid of mate-doc-utils
dist-omf-hook:
	@for lc in C $(DOC_LINGUAS); do \
		omf="$(DOC_MODULE)-$$lc.omf"; \
		if test -f $(builddir)/$$omf; then d=$(builddir); \
		elif test -f $(srcdir)/$$lc/$$omf; then d=$(srcdir)/$$lc; fi; \
		if test -f "$$d/$$omf"; then \
			if test "$(opt_verbose)" = ""; then echo "  Distributing $$omf"; fi; \
			cmd="cp $$d/$$omf $(distdir)/$$lc/"; eval $$cmd; \
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
clean-local: remove-C-xml
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

# we have to override mate-doc-utils.make:clean-doc-dir recipe
# as it tries to remove directories which are not empty as we have
# not yet deleted the files we putted on
clean-doc-dir:
	@:

.PHONY: \
	all-manuals install-manuals uninstall-manuals dist-manuals-hook \
	all-html-manuals install-html-manuals uninstall-html-manuals dist-html-manuals-hook \
	all-pdf-manuals install-pdf-manuals uninstall-pdf-manuals dist-pdf-manuals-hook
