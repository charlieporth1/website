Summary:        Vadmin Plugin is an interface to vmailmgr.
Name:           squirrelmail-vadmin
Version:        1.9.3
Release:        1
Epoch:          0
License:        GPL
Group:          Applications/Internet
URL:            http://www.sf.net/projects/vadmin-plugin
Source0:        vadmin-%{version}.tar.gz
Requires:       squirrelmail >= 1.4.0, httpd >= 2.0, php >= 4.2
Requires:       vmailmgr-daemon >= 0.96.9, qmail-autoresponder >= 0.96.1
Obsoletes:      vadmin
BuildRequires:  perl
BuildArch:      noarch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
Vadmin is a front-end interface to VmailMgr and allows administrators
to add/edit/delete users right from the convenience of their
SquirrelMail system. It incorporates tight security features,
encryption, and many convenient options to make the life of an
administrator easier and make him feel very wise for using
Qmail-Vmailmgr-Courier-SquirrelMail tie-together.

#------------------------------------------------------------------------------

%prep
%setup -q -n vadmin-%{version}
##
# Change config file location
#
%{__perl} -pi -e \
   "s|config_file\s*=.*?;|config_file='%{_sysconfdir}/vadmin/vadmin.conf';|g" \
    *.php
##
# Change paths
#
%{__perl} -pi -e "s|squirrelmail/plugins/||g" conf/vadmin.conf
%{__perl} -pi -e \
    "s|plugin\s*=.*?$|plugin = %{_datadir}/squirrelmail/plugins/vadmin|m" \
    conf/vadmin.conf
##
# Fix version
#
%{__perl} -pi -e \
    "s|VADMIN_VERSION\s*=.*?;|VADMIN_VERSION = \"%{version}-%{release}\";|" \
    includes/vadmin_functions.inc

#------------------------------------------------------------------------------

%install
%{__rm} -rf %{buildroot}
%{__mkdir_p} -m 755 \
    %{buildroot}%{_datadir}/squirrelmail/plugins/vadmin \
    %{buildroot}%{_datadir}/vadmin/modules/admin \
    %{buildroot}%{_datadir}/vadmin/modules/user \
    %{buildroot}%{_datadir}/vadmin/includes \
    %{buildroot}%{_datadir}/vadmin/locale \
    %{buildroot}%{_localstatedir}/lib/vadmin \
    %{buildroot}%{_sysconfdir}/vadmin \
    %{buildroot}%{_sysconfdir}/httpd/conf.d

%{__install} -m 644 *.php %{buildroot}%{_datadir}/squirrelmail/plugins/vadmin/
%{__install} -m 644 includes/*.inc %{buildroot}%{_datadir}/vadmin/includes/
%{__install} -m 644 locale/*.po %{buildroot}%{_datadir}/vadmin/locale/
##
# Just in case there are some translations there
#
for LCDIR in `find locale/* -type d`; do
    %{__cp} -rp $LCDIR %{buildroot}%{_datadir}/vadmin/locale/
done
%{__install} -m 644 modules/user/*.mod \
    %{buildroot}%{_datadir}/vadmin/modules/user/
%{__install} -m 644 modules/admin/*.mod \
    %{buildroot}%{_datadir}/vadmin/modules/admin/
##
# Do configs
#
%{__install} -m 600 conf/apache.conf \
    %{buildroot}%{_sysconfdir}/httpd/conf.d/vadmin.conf
%{__install} -m 644 conf/vadmin.conf %{buildroot}%{_sysconfdir}/vadmin/

#------------------------------------------------------------------------------

%clean
%{__rm} -rf %{buildroot}

#------------------------------------------------------------------------------

%files
%defattr(-,root,root,-)
%doc doc/*
%config %dir %{_sysconfdir}/vadmin
%config(noreplace) %{_sysconfdir}/vadmin/*
%config(noreplace) %{_sysconfdir}/httpd/conf.d/*
%{_datadir}/vadmin
%{_datadir}/squirrelmail/plugins/vadmin
%attr(0770, root, apache) %dir %{_localstatedir}/lib/vadmin

#------------------------------------------------------------------------------

%changelog
* Sun Jul 04 2004 Konstantin Ryabitsev <icon@linux.duke.edu>
- Version 1.9.3

* Wed Jul 09 2003 Konstantin Riabitsev <icon@linux.duke.edu>
- Removing libmcrypt, php-mcrypt requirements, as builtin rc4 now provided
- Rebuilding for 1.9.2

* Sun Jul 06 2003 Konstantin Riabitsev <icon@linux.duke.edu>
- Version 1.9.1

* Wed Jun 18 2003 Konstantin Riabitsev <icon@fleur.hogwarts.jk> 
- Initial build.


