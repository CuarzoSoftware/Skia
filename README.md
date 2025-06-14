<h1 style="margin-top:0px;padding-top:0px">cz-skia</h1>

<p align="left">
  <a href="https://github.com/CuarzoSoftware/Skia/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-BSD--3-blue.svg" alt="Skia is released under the BSD-3 license." />
  </a>
  <a href="https://github.com/CuarzoSoftware/Skia">
    <img src="https://img.shields.io/badge/version-0.1.0-brightgreen" alt="Current cz-skia version." />
  </a>
</p>

This repository contains a **slightly modified** version of [Skia C++](https://github.com/google/skia), adapted for building with **Meson** on Linux.

The entire library and its modules are compiled into a **single shared library**.

## Modules

- core
- skcms
- skresources
- skunicode
- skparagraph
- skshaper
- svg
- ganesh
- graphite
- jpeg, png, webp, ico, bmp

## Fedora

Install directly from the [cuarzo/software](https://copr.fedorainfracloud.org/coprs/cuarzo/software/) COPR:

```bash
$ sudo dnf copr enable cuarzo/software
$ sudo dnf install cz-skia cz-skia-devel
```

## Linking

The pkg-config package name is `cz-skia`.

Headers can be included like this:

```cpp
#include <skia/core/SkSurface.h>
#include <skia/modules/skparagraph/include/Paragraph.h>
```

## Manual Building

### Dependencies

Install Meson and the dependencies listed in `meson.build`.

### Build & Install

```bash
$ cd Skia # This repo's root dir
$ meson setup builddir -Dbuildtype=release
$ cd builddir
$ meson install

# To remove later
$ cd builddir
$ sudo ninja uninstall
```
