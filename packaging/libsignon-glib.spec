Name:       libsignon-glib
Summary:    GLib API for the SSO framework
Version:    1.9.0
Release:    4
Group:      System/Libraries
License:    LGPL
Source:	    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(check)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gobject-2.0)
BuildRequires:  pkgconfig(gio-2.0)

%description
%{summary}.


%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
%{summary}.


%prep
%setup -q -n %{name}-%{version}
if [ -f = "gtk-doc.make" ]
then
rm gtk-doc.make
fi
touch gtk-doc.make
autoreconf -f -i


%build
%configure
make #%{?_smp_mflags}


%install
rm -rf %{buildroot}
%make_install


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README
%{_libdir}/%{name}.so.*


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*.h
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/%{name}.pc
%{_datadir}/vala/vapi/signon.vapi


%changelog
* Mon Feb 11 2013 Jussi Laako <jussi.laako@linux.intel.com> - 1.9
- Update to latest intel branch version

* Thu Feb 07 2013 Jussi Laako <jussi.laako@linux.intel.com> - 1.7
- Update to latest intel branch version

* Tue Mar 27 2012 Jussi Laako <jussi.laako@linux.intel.com> - 1.1
- Update to latest upstream version

* Wed Aug 03 2011 Jussi Laako <jussi.laako@linux.intel.com> - 1.0
- Update to latest upstream version
