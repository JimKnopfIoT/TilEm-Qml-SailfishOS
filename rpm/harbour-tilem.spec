Name:       harbour-tilem
%define _buildhost reproducible-builder
Summary:    TI Calculator Emulator for SailfishOS
Version:    1.0.3
Release:    1
Group:      Applications/Productivity
License:    GPLv3+ and LGPLv2+
URL:        https://github.com/JimKnopfIoT/TilEm-Qml-SailfishOS
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gobject-2.0)
BuildRequires:  desktop-file-utils

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   qt5-qtdeclarative-import-folderlistmodel
# Skin images are JPEG and are decoded by Qt (gdk-pixbuf on SFOS has no JPEG loader)
Requires:   qt5-plugin-imageformat-jpeg

%description
TilEm is an emulator for Texas Instruments Z80-based graphing calculators.

Supported models:
- TI-73 / TI-73 Explorer
- TI-76.fr
- TI-81
- TI-82 / TI-82 STATS
- TI-83 / TI-83 Plus / TI-83 Plus Silver Edition
- TI-84 Plus / TI-84 Plus Silver Edition
- TI-85
- TI-86

Note: You need a calculator ROM image to use this emulator. ROM images are
copyrighted by Texas Instruments and cannot be distributed.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 VERSION=%{version} RELEASE=%{release}
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

# Validate desktop file (it's already installed by qmake)
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop || echo "Desktop file validation warning"

# Install icons
for size in 86x86 108x108 128x128 172x172; do
  install -D -m 0644 src/icons/${size}/%{name}.png \
    %{buildroot}%{_datadir}/icons/hicolor/${size}/apps/%{name}.png
done

# Install skins
mkdir -p %{buildroot}%{_datadir}/%{name}/skins
install -m 0644 data/skins/*.skn %{buildroot}%{_datadir}/%{name}/skins/

# Install keybindings
mkdir -p %{buildroot}%{_datadir}/%{name}/data
install -m 0644 data/keybindings.ini %{buildroot}%{_datadir}/%{name}/data/

# The Sailfish qmake macro passes QMAKE_STRIP=: so the binary reaches the
# buildroot with its symbol table intact. Strip it for the shipped package.
strip --strip-unneeded %{buildroot}%{_bindir}/%{name}

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%changelog
* Wed Sep 02 2026 harbour-tilem contributors 1.0.3-1
- Decode the calculator skins with Qt instead of gdk-pixbuf. SailfishOS 5.2
  ships gdk-pixbuf without a JPEG loader, so the skin failed to load and the
  application came up as a black screen with only the bare LCD drawn. Drops the
  gdk-pixbuf dependency entirely and adds qt5-plugin-imageformat-jpeg.
- Fix the skin settings object keeping stale, uninitialised data after a failed
  load, and leaking the previous skin on every reload.

* Tue Aug 18 2026 harbour-tilem contributors 1.0.1-1
- Release build with a neutral build host and the fork's repository in the
  package header, and incidental author metadata scrubbed from the bundled
  calculator skins. First version offered on OpenRepos. No change to the
  emulator itself.
