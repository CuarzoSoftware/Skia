%global basever 0.1.0
%global origrel 1
%global somajor 0

Name:           cz-skia
Version:        %{basever}%{?origrel:_%{origrel}}
Release:        1%{?dist}
Summary:        A customized build of Skia tailored for Linux systems.

License:        BSD-3
URL:            https://github.com/CuarzoSoftware/Louvre

BuildRequires:  meson
BuildRequires:  gcc-c++
BuildRequires:  git
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  pkgconfig(vulkan)
BuildRequires:  pkgconfig(harfbuzz)
BuildRequires:  pkgconfig(icu-uc)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(libwebp)
BuildRequires:  pkgconfig(libwebpdemux)
BuildRequires:  pkgconfig(libwebpmux)
BuildRequires:  pkgconfig(libjpeg)
BuildRequires:  pkgconfig(epoxy)
BuildRequires:  pkgconfig(SPIRV-Tools)
BuildRequires:  pkgconfig(expat)

%description
A customized build of Skia tailored for Linux systems.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
rm -rf Skia
git clone --depth 1 https://github.com/CuarzoSoftware/Skia.git

%build
pushd Skia
%meson
%meson_build

%install
pushd Skia
%meson_install

%files
%license Skia/LICENSE
%doc Skia/BUILD Skia/CHANGES Skia/VERSION
%{_libdir}/libcz-skia.so.%{somajor}

%files devel
%doc Skia/README.md
%{_includedir}/CZ/skia
%{_libdir}/libcz-skia.so
%{_libdir}/pkgconfig/cz-skia.pc

%changelog
* Sun Jun 15 2025 Eduardo Hopperdietzel <ehopperdietzel@gmail.com> - %{basever}-%{origrel}
- First release.
