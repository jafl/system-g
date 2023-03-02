Summary: System G provides a graphical interface to UNIX file system.
Name: %app_name
Version: %pkg_version
Release: 1
License: GPL
Group: Filemanager
Source: %pkg_name
Requires: libX11, libXinerama, libXpm, libXft, libxml2, gd, libjpeg, libpng, libicu, pcre, boost-devel, curl

%description
System G provides a graphical interface to UNIX file system,
similar to Mac OS 7.

%prep
%setup

%install

%define systemg_doc_dir /usr/local/share/doc/systemg
%define gnome_app_path  /usr/local/share/applications
%define gnome_icon_path /usr/local/share/pixmaps

./install $RPM_BUILD_ROOT/usr/local

%files

%docdir %systemg_doc_dir

/usr/local/bin/systemg
%systemg_doc_dir

%gnome_app_path/systemg.desktop
%gnome_app_path/systemg_choose.desktop
%gnome_icon_path/systemg.xpm
