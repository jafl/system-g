Summary: System G provides a graphical interface to UNIX file system.
Name: %app_name
Version: %pkg_version
Release: 1
License: GPL
Group: Filemanager
Source: %pkg_name
Requires: libX11, libXinerama, libXpm, libXft, libxml2, gd, libjpeg, libpng, libicu, pcre

%description
System G provides a graphical interface to UNIX file system,
similar to Mac OS 7.

%prep
%setup

%install

%define systemg_doc_dir /usr/share/doc/systemg
%define gnome_app_path  /usr/share/applications
%define gnome_icon_path /usr/share/pixmaps

./install "$RPM_BUILD_ROOT"/usr

%files

%docdir %systemg_doc_dir

/usr/bin/systemg
%systemg_doc_dir

%gnome_app_path/systemg.desktop
%gnome_app_path/systemg_choose.desktop
%gnome_icon_path/systemg.xpm
