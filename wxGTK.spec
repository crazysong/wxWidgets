%define pref /usr
%define ver 2.3.2
%define ver2 2.3
%define rel 0

Summary: The GTK+ 1.2 port of the wxWindows library
Name: wxGTK
Version: %{ver}
Release: %{rel}
Copyright: wxWindows Licence
Group: X11/Libraries
Source: wxGTK-%{ver}.tar.bz2
URL: http://www.wxwindows.org
Packager: Vadim Zeitlin <vadim@wxwindows.org>
Prefix: %{pref}
BuildRoot: /var/tmp/%{name}-root

# all packages providing an implementation of wxWindows library (regardless of
# the toolkit used) should provide the (virtual) wxwin package, this makes it
# possible to require wxwin instead of requiring "wxgtk or wxmotif or wxuniv..."
Provides: wxwin

# in addition, we should provide libwx_gtk as automatic generator only notices
# libwx_gtk-%{ver}-%{rel}
Provides: libwx_gtk.so
Provides: libwx_gtk-%{ver2}.so

%description
wxWindows is a free C++ library for cross-platform GUI development.
With wxWindows, you can create applications for different GUIs (GTK+,
Motif/LessTif, MS Windows, Mac) from the same source code.

%package devel
Summary: The GTK+ 1.2 port of the wxWindows library
Group: X11/Libraries
Requires: wxGTK = %{ver}

%description devel
Header files for wxGTK, the GTK+ 1.2 port of the wxWindows library.

%package gl
Summary: The GTK+ 1.2 port of the wxWindows library, OpenGL add-on.
Group: X11/Libraries
Requires: wxGTK = %{ver}

%description gl
OpenGL add-on library for wxGTK, the GTK+ 1.2 port of the wxWindows library.

%package static
Summary: wxGTK static libraries
Group: Development/Libraries

%description static
Static libraries for wxGTK. You need them if you want to link statically against wxGTK.

%prep
%setup -n wxGTK-%{ver}

%build
if [ "$SMP" != "" ]; then
  export MAKE="make -j$SMP"
else
  export MAKE="make"
fi

(cd locale; make allmo)

mkdir obj-shared
cd obj-shared
../configure --prefix=%{pref} --enable-soname --with-odbc --with-opengl
$MAKE
cd ..

mkdir obj-static
cd obj-static
../configure --prefix=%{pref} --disable-shared --with-odbc --with-opengl
$MAKE
cd ..

%install
rm -rf $RPM_BUILD_ROOT
(cd obj-static; make prefix=$RPM_BUILD_ROOT%{pref} install)
(cd obj-shared; make prefix=$RPM_BUILD_ROOT%{pref} install)

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%post gl
/sbin/ldconfig

%postun gl
/sbin/ldconfig

%post devel
# Install wx-config if there isn't any
if test ! -f %{_bindir}/wx-config ; then
    ln -sf wxgtk-%{ver2}-config %{_bindir}/wx-config
fi

%preun devel
# Remove wx-config if it points to this package
if test -f %{_bindir}/wx-config -a -f /usr/bin/md5sum ; then
  SUM1=`md5sum %{_bindir}/wxgtk-%{ver2}-config | cut -c 0-32`
  SUM2=`md5sum %{_bindir}/wx-config | cut -c 0-32`
  if test "x$SUM1" = "x$SUM2" ; then
    rm -f %{_bindir}/wx-config
  fi
fi


%files
%defattr(-,root,root)
%doc COPYING.LIB *.txt
%dir %{_datadir}/wx
%{_datadir}/wx/*
%{_datadir}/locale/*/*/*.mo
%{_libdir}/libwx_gtk-%{ver2}*.so.*

%files devel
%defattr(-,root,root)
%{_libdir}/libwx_gtk-%{ver2}*.so
%dir %{_includedir}/wx
%{_includedir}/wx/*
%dir %{_libdir}/wx
%{_libdir}/wx/*
%{_bindir}/wxgtk-%{ver2}-config

%files gl
%defattr(-,root,root)
%{_libdir}/libwx_gtk_gl*

%files static
%defattr (-,root,root)
%{_libdir}/lib*.a
