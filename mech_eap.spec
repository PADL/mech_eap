%global _moonshot_krb5 %{!?_moonshot_krb5:krb5-devel}%{?_moonshot_krb5}
Name:		moonshot-gss-eap
Version:	0.9.5
Release:	1%{?dist}
Summary:	Moonshot GSS-API Mechanism

Group:		Security Tools
License:	BSD
URL:		http://www.project-moonshot.org/
Source0:	mech_eap-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires:	 %{_moonshot_krb5} >= 1.9.1
BuildRequires:	 moonshot-ui-devel
BuildRequires: jansson-devel
Requires:	moonshot-ui
BuildRequires: libradsec-devel >= 0.0.3
BuildRequires: shibboleth-sp-devel >= 2.5
BuildRequires: libshibresolver-devel



%description
Project Moonshot provides federated access management.


%prep
%setup -q -n mech_eap-%{version}


%build
%configure  --with-libmoonshot=%{_prefix} --with-krb5=%{_prefix} --enable-reauth LDFLAGs="${LDFLAGS} -L/opt/shibboleth/%{_lib} -Wl,--rpath=/opt/shibboleth/%{_lib}" CPPFLAGS="${CPPFLAGS} -I/opt/shibboleth/include"
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT

%post
oid_aes128=1.3.6.1.5.5.15.1.1.17
oid_aes256=1.3.6.1.5.5.15.1.1.18
mechfile=%{_sysconfdir}/gss/mech
mkdir -p %{_sysconfdir}/gss
test -e $mechfile |touch $mechfile
fgrep $oid_aes128 $mechfile ||echo eap-aes128 $oid_aes128 mech_eap.so >>$mechfile
fgrep $oid_aes256 $mechfile ||echo eap-aes256 $oid_aes256 mech_eap.so >>$mechfile


%files
%defattr(-,root,root,-)
%doc mech_eap/README
%doc mech_eap/LICENSE
%doc mech_eap/AUTHORS
%{_libdir}/gss/mech_eap.so
%exclude %{_libdir}/gss/mech_eap.la
%{_includedir}/gssapi/*.h
%exclude %{_libdir}/krb5/plugins/authdata/*
#%{_libdir}/krb5/plugins/authdata/*.so



%changelog
* Wed Sep 28 2011  <hartmans@moonbuildcentos.dev.ja.net> - 0.9.5-2
- Add radius_ad plugin

