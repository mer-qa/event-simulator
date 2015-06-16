Name:           event-simulator
Summary:        event-simulator
Version:        0.1
Release:        1
Group:          Tools
License:        GPLv2+
URL:            https://github.com/mer-qa/event-simulator
Source0:        %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(Qt5Core)

%description
Event simulator for QA

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%prep
%setup -q -n %{name}-%{version}


%build
%qtc_qmake5
%qtc_make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%{_bindir}/simulate-event
