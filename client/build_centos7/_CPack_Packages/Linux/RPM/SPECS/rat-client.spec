# Restore old style debuginfo creation for rpm >= 4.14.
%undefine _debugsource_packages
%undefine _debuginfo_subpackages

# -*- rpm-spec -*-
BuildRoot:      %_topdir/rat-client-1.0.0-Linux
Summary:        RATClient built using CMake
Name:           rat-client
Version:        1.0.0
Release:        1
License:        MIT
Group:          Utilities
Vendor:         quang

Url: https://quang.com


















%define _rpmdir %_topdir/RPMS
%define _srcrpmdir %_topdir/SRPMS

%define _unpackaged_files_terminate_build 0



%define _binary_payload w7.xzdio

%description
DESCRIPTION
===========

This is an installer created using CPack (https://cmake.org). No additional installation instructions provided.



# This is a shortcutted spec file generated by CMake RPM generator
# we skip _install step because CPack does that for us.
# We do only save CPack installed tree in _prepr
# and then restore it in build.
%prep
mv $RPM_BUILD_ROOT %_topdir/tmpBBroot

%install
if [ -e $RPM_BUILD_ROOT ];
then
  rm -rf $RPM_BUILD_ROOT
fi
mv %_topdir/tmpBBroot $RPM_BUILD_ROOT



%clean








%files
%defattr(-,root,root,-)
%dir /usr/local
%dir /usr/local/bin
/usr/local/bin/rat_client
%dir /usr/local/etc
%dir /usr/local/etc/rat-client
/usr/local/etc/rat-client/ca.crt




%changelog
* Sun Jul 4 2010 Eric Noulard <eric.noulard@gmail.com> - 1.0.0-1
  Generated by CPack RPM (no Changelog file were provided)


