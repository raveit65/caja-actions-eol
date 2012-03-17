Summary:	Caja extension for customizing the context menu
Name:		caja-actions
Version:	1.1.0
Release:	1%{?dist}
Group:		User Interface/Desktops
License:	GPLv2+
URL:		http://www.grumz.net/node/8
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	caja-devel dbus-glib-devel libxml2-devel mate-conf-devel
BuildRequires:	libuuid-devel, unique-devel, libSM-devel
BuildRequires:	gettext, perl(XML::Parser)
BuildRequires:	desktop-file-utils intltool

Provides: caja-actions = %{version}-%{release}
Obsoletes: caja-actions = 2.30.3

%description
Caja actions is an extension for Caja, the MATE file manager.
It provides an easy way to configure programs to be launch on files 
selected in Nautilus interface

%package	devel
Summary:	Development tools for the caja-actions
Group:		Development/Libraries
Requires:	%{name} = %{version}-%{release}

Provides: caja-actions-devel = %{version}-%{release}
Obsoletes: caja-actions-devel = 2.30.3

%description	devel
This package contains headers and shared libraries needed for development
with caja-actions.

%prep
%setup -q

%build
%configure --disable-schemas-install --enable-compile-warnings=minimum
make %{?_smp_mflags} 

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

#rm -rf %{buildroot}%{_datadir}/applications/fedora-nact.desktop

#desktop-file-install --delete-original			\
#	--vendor fedora					\
#	--dir %{buildroot}%{_datadir}/applications	\
#	--mode 0644					\
#	--remove-category Application			\
#	--remove-category AdvancedSettings		\
#	--add-category MATE				\
#	--add-category Settings				\
#	%{buildroot}%{_datadir}/applications/nact.desktop

find %{buildroot} -type f -name "*.la" -exec rm -f {} ';'

%find_lang %{name}

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig
touch --no-create %{_datadir}/icons/hicolor || :

%postun
/sbin/ldconfig
if [ $1 -eq 0 ] ; then
    touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files -f %{name}.lang
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog NEWS README TODO
%{_bindir}/caja-actions-run
%{_bindir}/caja-actions-config-tool
%{_bindir}/caja-actions-new
%{_bindir}/caja-actions-schemas
%{_libdir}/%{name}/
%{_libdir}/caja/extensions-2.0/libcaja-actions-menu.so
%{_libdir}/caja/extensions-2.0/libcaja-actions-tracker.so
%{_datadir}/%{name}/
%{_datadir}/icons/hicolor/*/apps/caja-actions.*
%{_datadir}/applications/nact.desktop

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/

%changelog
* Sun Feb 12 2012 Wolfgang Ulbrich <info@raveit.de> - 1.1.0-1
- change version to mate release version

* Wed Jan 04 2012 Wolfgang Ulbrich <info@raveit.de> - 2.30.3-1
- start building for the MATE-Desktop
- caja-actions.spec based on nautilus-actions-2.30.3-1.fc14 spec

* Tue Jun 15 2010 Deji Akingunola <dakingun@gmail.com> - 2.30.3-1
- Update to 2.30.3

