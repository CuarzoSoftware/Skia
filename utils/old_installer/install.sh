#!/bin/bash

ARCH_X64=("x64" "x86_64" "amd64")
ARCH_X86=("x86" "i386" "i486" "i586" "i686")
ARCH_ARM=("arm" "armel" "armhf")
ARCH_ARM64=("arm64" "aarch64")
BIN_DEPS=("git" "wget" "tar" "python3" "clang" "clang++")
LIB_DEPS=("egl" "gl" "glesv2" "harfbuzz" "icu-uc" "fontconfig" "freetype2" "zlib" "libpng" "libwebp" "libjpeg")

help() {
    echo -e "\nUsage: SK_ARCH=ARCH SK_PREFIX=PREFIX SK_LIBDIR=LIBDIR SK_INCDIR=INCDIR $0"
    echo -e "Where:"
    echo -e "- ARCH: Target CPU architecture. Supported values:"
    echo -e "    x64   or alias [x86_64, amd64]"
    echo -e "    x86   or alias [i386, i486, i586, i686]"
    echo -e "    arm   or alias [armel, armhf]"
    echo -e "    arm64 or alias [aarch64]"
    
    echo -e "- PREFIX: Install prefix path. For example SK_PREFIX=/"
    echo -e "- LIBDIR: Libraries install path relative to SK_PREFIX. For example SK_LIBDIR=/usr/lib -> final path SK_PREFIX/usr/lib"
    echo -e "- INCDIR: Headers install path relative to SK_PREFIX. For example SK_INCDIR=/usr/include -> final path SK_PREFIX/usr/include/skia\n"
}

summary() {
	echo -e "\n**************************** SUMMARY ****************************\n"
	echo "  Skia Version:                      $SK_VERSION"
    echo "  Skia Commit:                       $SK_COMMIT"
	echo "  Target Arch:                       $SK_ARCH"
	echo "  Install Prefix:                    $SK_PREFIX"
	echo "  Final Library Install Path:        $SK_FINAL_LIBDIR"
	echo "  Final Headers Install Path:        $SK_FINAL_INCDIR"
	echo "  Final PKGCONFIG File Install Path: $SK_FINAL_PKG_DIR"  
	echo -e "\n*****************************************************************\n"
}

concat_paths() {
    local path1="$1"
    local path2="$2"
    
    # Remove any trailing slash from the first path
    path1="${path1%/}"
    
    # Remove any leading slash from the second path
    path2="${path2#/}"

    # Concatenate the paths
    echo "$path1/$path2"
}

in_array() {
    local element
    for element in "${@:2}"; do
        if [[ "$element" == "$1" ]]; then
            return 0
        fi
    done
    return 1
}

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SK_VERSION=$(cat $SCRIPT_DIR/VERSION)
SK_COMMIT=$(cat $SCRIPT_DIR/COMMIT)
SK_OPTIONS=$(<$SCRIPT_DIR/OPTIONS)

if [ -z "${SK_PREFIX}" ]; then
    echo -e "\nError: Missing install prefix."
    help
    exit 1
else
    if [[ "$SK_PREFIX" != /* ]]; then
        echo -e "\nError: The install prefix must be an absolute path."
        help
        exit 1
    fi
fi

mkdir -p $SK_PREFIX

if [ -z "${SK_LIBDIR}" ]; then
    echo -e "\nError: Missing library install path."
    help
    exit 1
else
    if [[ "$SK_LIBDIR" != /* ]]; then
        echo -e "\nError: SK_LIBDIR must start with /"
        help
        exit 1
    fi
fi

if [ -z "${SK_INCDIR}" ]; then
    echo -e "\nError: Missing headers install path."
    help
    exit 1
else
    if [[ "$SK_INCDIR" != /* ]]; then
        echo -e "\nError: SK_INCDIR must start with /"
        help
        exit 1
    fi
fi

if [ -z "${SK_ARCH}" ]; then
    echo -e "\nError: Missing target CPU arch."
    help
    exit 1
else
    TEST_ARCH=""
    
    if in_array "$SK_ARCH" "${ARCH_X64[@]}"; then
        TEST_ARCH="x64"
    fi
    
    if  [ -z "$TEST_ARCH" ] && in_array "$SK_ARCH" "${ARCH_X86[@]}"; then
        TEST_ARCH="x86"
    fi
    
    if  [ -z "$TEST_ARCH" ] && in_array "$SK_ARCH" "${ARCH_ARM[@]}"; then
        TEST_ARCH="arm"
    fi
    
    if  [ -z "$TEST_ARCH" ] && in_array "$SK_ARCH" "${ARCH_ARM64[@]}"; then
        TEST_ARCH="arm64"
    fi
    
    if  [ -z "$TEST_ARCH" ]; then
        echo -e "\nError: Invalid CPU arch: $SK_ARCH."
        help
        exit 1
    fi
    
    SK_ARCH=$TEST_ARCH
fi

# Check dependencies

echo -e "\nChecking binary dependencies:"

for DEP in "${BIN_DEPS[@]}"; do
    if command -v "$DEP" > /dev/null 2>&1; then
        echo "    Found $DEP."
    else
        echo "Error: $DEP not found."
        exit 1
    fi
done

echo -e "\nChecking library dependencies:"

for DEP in "${LIB_DEPS[@]}"; do
    if pkg-config --exists "$DEP"; then
        echo "    Found $DEP."
    else
        echo "Error: $DEP not found."
        exit 1
    fi
done

# Summary
SK_FINAL_LIBDIR=$(concat_paths $SK_PREFIX  $SK_LIBDIR)
SK_FINAL_PKG_DIR=$(concat_paths $SK_FINAL_LIBDIR "/pkgconfig")
SK_FINAL_INCDIR=$(concat_paths $SK_PREFIX $SK_INCDIR)
SK_FINAL_INCDIR=$(concat_paths $SK_FINAL_INCDIR "/skia")
mkdir -p $SK_FINAL_INCDIR
mkdir -p $SK_FINAL_LIBDIR
mkdir -p $SK_FINAL_PKG_DIR
summary

TMP_DIR=${SCRIPT_DIR}/tmp
mkdir -p ${TMP_DIR}/build
mkdir -p ${TMP_DIR}/include
cd ${TMP_DIR}/build

if [ ! -e "$TMP_DIR/build/depot_tools" ]; then
    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
else
    echo -e "\ndepot_tools already cloned, skipping..."
fi

export PATH="${PWD}/depot_tools:${PATH}"

if [ ! -e "$TMP_DIR/build/skia" ]; then
    git clone --depth 1 --single-branch --branch main https://skia.googlesource.com/skia
    cd skia
    git fetch --depth 1 origin $SK_COMMIT
    git reset --hard $SK_COMMIT
    cd ..
else
    echo -e "\nSkia repo already cloned, skipping..."
fi

cd skia

# For some reason this sometimes fails the first time

max_retries=5
attempt=0

if [ ! -e "$TMP_DIR/build/skia/bin/gn" ]; then
    until python3 tools/git-sync-deps; do
        attempt=$((attempt+1))
        if [ $attempt -ge $max_retries ]; then
            echo "tools/git-sync-deps failed after $max_retries attempts."
            break
        fi
        echo "tools/git-sync-deps failed. Retrying ($attempt/$max_retries)..."
    done
else
    echo -e "\nbin/gn already downloaded, skipping..."
fi

source "$TMP_DIR/build/skia/third_party/externals/emsdk/emsdk_env.sh"

bin/gn gen out/Shared --args="target_os=\"linux\" target_cpu=\"$SK_ARCH\" $SK_OPTIONS"

ninja -C out/Shared

if [ $? -ne 0 ]; then
    echo -e "Error: Skia compilation failed."
    exit 1
fi

echo -e "\nInstalling libraries into $SK_FINAL_LIBDIR:"
mkdir -p $SK_FINAL_LIBDIR

exit 0 # ---------------------------------------------------------------------------------------- REMOVE THIS

# Add the "libcz_" prefix to avoid conflicts with other Skia installations
for file in $TMP_DIR/build/skia/out/Shared/*.a $TMP_DIR/build/skia/out/Shared/*.so; do
    new_name=$(basename "$file" | sed 's/^lib/libcz_/')
    cp -v "$file" "$SK_FINAL_LIBDIR/$new_name"
done

echo -e "\nFixing headers include prefix..."

mkdir -p $TMP_DIR/build/skia/fixed_headers/modules
mkdir -p $TMP_DIR/build/skia/fixed_headers/src

mkdir -p $SK_FINAL_INCDIR

cd $TMP_DIR/build/skia/include
find . -name "*.h" -exec cp --parents {} $TMP_DIR/build/skia/fixed_headers \;
cd $TMP_DIR/build/skia/modules
find . -name "*.h" -exec cp --parents {} $TMP_DIR/build/skia/fixed_headers/modules \;
cd $TMP_DIR/build/skia/src
find . -name "*.h" -exec cp --parents {} $TMP_DIR/build/skia/fixed_headers/src \;

cd $TMP_DIR/build/skia/fixed_headers
find . -type f -exec sed -i 's|#include "include/|#include "skia/|g' {} +
find . -type f -exec sed -i 's|#include "modules/|#include "skia/modules/|g' {} +
find . -type f -exec sed -i 's|#include "src/core/|#include "skia/src/core/|g' {} +
find . -type f -exec sed -i 's|#include "src/base/|#include "skia/src/base/|g' {} +
find . -type f -exec sed -i 's|#include "src/xml/|#include "skia/src/xml/|g' {} +
find . -type f -exec sed -i 's|#include "src/utils/|#include "skia/src/utils/|g' {} +

echo -e "\nInstalling headers into $SK_FINAL_INCDIR:"
find . -name "*.h" -exec cp -v --parents {} $SK_FINAL_INCDIR \;


# Show configuration
cd $TMP_DIR/build/skia
bin/gn args out/Shared --list

# Gen pkgconfig file
cat <<EOF > $TMP_DIR/build/skia/out/Shared/cuarzo-skia.pc
includedir=$SK_INCDIR
libdir=$SK_LIBDIR

Name: cuarzo-skia
Description: Skia is a complete 2D graphic library for drawing Text, Geometries, and Images.
Version: $SK_VERSION
Libs: -L$SK_LIBDIR -lcz_skia -lcz_skunicode_core -lcz_skunicode_icu -lcz_skparagraph -lcz_compression_utils_portable -lcz_pathkit -lcz_skcms -lcz_skshaper -lcz_dng_sdk -lcz_piex -lcz_wuffs -lcz_bentleyottmann
Cflags: -I$SK_INCDIR -DSK_GL -DSK_GANESH -DSK_VULKAN -DSK_UNICODE_ICU_IMPLEMENTATION
EOF

# -lcz_skia -lcz_skunicode_core -lcz_skunicode_icu -lcz_skshaper -lcz_bentleyottmann -lcz_skparagraph -lcz_skcms

# Libs: -L$SK_LIBDIR -lcz_skia -lcz_skunicode_core -lcz_skunicode_icu -lcz_skparagraph -lcz_compression_utils_portable -lcz_pathkit -lcz_skcms -lcz_skshaper -lcz_dng_sdk -lcz_piex -lcz_wuffs -lcz_bentleyottmann

echo -e "\nInstalling cuarzo-skia.pc into $SK_FINAL_PKG_DIR."
cp $TMP_DIR/build/skia/out/Shared/cuarzo-skia.pc $SK_FINAL_PKG_DIR
cd $SCRIPT_DIR

summary
echo -e "Installation complete.\n"


