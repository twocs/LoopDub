#!/bin/bash

# Package locations

PKG_LIBSNDFILE_TAR="libsndfile-1.0.17.tar.gz"
PKG_LIBSNDFILE_MD5="2d126c35448503f6dbe33934d9581f6b"
PKG_LIBSNDFILE_DIR="libsndfile-1.0.17"
PKG_LIBSNDFILE_URL="http://www.mega-nerd.com/libsndfile/libsndfile-1.0.17.tar.gz"
PKG_LIBSNDFILE_LIB="$PKG_LIBSNDFILE_DIR/src/.libs/libsndfile.a"

PKG_LIBSAMPLERATE_TAR="libsamplerate-0.1.2.tar.gz"
PKG_LIBSAMPLERATE_MD5="06861c2c6b8e5273c9b80cf736b9fd0e"
PKG_LIBSAMPLERATE_DIR="libsamplerate-0.1.2"
PKG_LIBSAMPLERATE_URL="http://www.mega-nerd.com/SRC/libsamplerate-0.1.2.tar.gz"
PKG_LIBSAMPLERATE_LIB="$PKG_LIBSAMPLERATE_DIR/src/.libs/libsamplerate.a"

PKG_SDL_TAR="SDL-1.2.11.tar.gz"
PKG_SDL_MD5="418b42956b7cd103bfab1b9077ccc149"
PKG_SDL_DIR="SDL-1.2.11"
PKG_SDL_LIB="$PKG_SDL_DIR/build/.libs/libSDL.a"
PKG_SDL_URL="http://www.libsdl.org/release/SDL-1.2.11.tar.gz"

PKG_SDLDX_TAR="directx-devel.tar.gz"
PKG_SDLDX_MD5="d8080934f6d46080eda1f74dfb95b172"
PKG_SDLDX_URL="http://www.libsdl.org/extras/win32/cygwin/directx-devel.tar.gz"



case $(uname) in
	CYGWIN*)
		echo "Bootstrapping for Cygwin..."

DXDIR="$1"

if [ "$DXDIR"x == "x" ]; then
	echo
	echo "This script bootstraps a development environment for LoopDub under"
	echo "under Cygwin on Windows. That is, it will download tarballs of"
	echo "needed libraries from various places on the web and compile them."
	echo "It won't install anything outside the current folder, so you do"
	echo "not need to worry about damaging your system."
	echo
	echo "It needs the following packages  pre-installed:"
	echo
	echo "make"
	echo "gcc"
	echo "python"
	echo "w32api"
	echo
	echo "You must also install the DirectX SDK somewhere, and provide"
	echo "the location as the first argument to this script."
	echo 'For example, "/cygdrive/c/Program Files/DXSDK",'
	echo 'or possibly, "/cygdrive/c/Program Files/Microsoft DirectX SDK (August 2006)"'
	echo
	echo "Usage: ./bootstrap_cygwin.sh <location of DirectX SDK root>"
	echo
	exit
fi


# Build everything
make -C libdeps -f ./bootstrap_cygwin.mk DXDIR="$DXDIR" \
 PKG_LIBSNDFILE_TAR="$PKG_LIBSNDFILE_TAR" PKG_LIBSNDFILE_DIR="$PKG_LIBSNDFILE_DIR" PKG_LIBSNDFILE_URL="$PKG_LIBSNDFILE_URL" PKG_LIBSNDFILE_MD5="$PKG_LIBSNDFILE_MD5" PKG_LIBSNDFILE_LIB="$PKG_LIBSNDFILE_LIB" \
 PKG_LIBSAMPLERATE_TAR="$PKG_LIBSAMPLERATE_TAR" PKG_LIBSAMPLERATE_DIR="$PKG_LIBSAMPLERATE_DIR" PKG_LIBSAMPLERATE_URL="$PKG_LIBSAMPLERATE_URL" PKG_LIBSAMPLERATE_MD5="$PKG_LIBSAMPLERATE_MD5" PKG_LIBSAMPLERATE_LIB="$PKG_LIBSAMPLERATE_LIB" \
 PKG_SDL_TAR="$PKG_SDL_TAR" PKG_SDL_DIR="$PKG_SDL_DIR" PKG_SDL_URL="$PKG_SDL_URL" PKG_SDL_MD5="$PKG_SDL_MD5" PKG_SDL_LIB="$PKG_SDL_LIB" \
 PKG_SDLDX_TAR="$PKG_SDLDX_TAR" PKG_SDLDX_URL="$PKG_SDLDX_URL" PKG_SDLDX_MD5="$PKG_SDLDX_MD5"

    ;;
    Linux*)
		echo "Bootstrapping for Linux..."

if ! [ "$1"x == "x" ]; then
	echo
	echo "This script bootstraps a development environment for LoopDub under"
	echo "under Linux. That is, it will download tarballs of needed libraries"
	echo "from various places on the web and compile them."
	echo "It won't install anything outside the current folder, so you do"
	echo "not need to worry about damaging your system."
	echo
	echo "It needs the following packages  pre-installed:"
	echo
	echo "build-essential"
	echo "python"
	echo "md5sum"
	echo "libSDL-dev"
	echo "libsndfile-dev"
	echo "libsamplerate-dev"
	echo "libasound-dev"
	echo "libjack-dev"
	echo
	echo "Usage: ./bootstrap_linux.sh"
	echo
	exit
fi


# Build everything
make -C libdeps -f ./bootstrap_linux.mk
    ;;

    Darwin*)
		echo "Bootstrapping for Darwin..."

if ! [ "$1"x == "x" ]; then
	echo
	echo "This script bootstraps a development environment for LoopDub under"
	echo "under Darwin/OS X. That is, it will download tarballs of needed"
	echo "libraries from various places on the web and compile them."
	echo "It won't install anything outside the current folder, so you do"
	echo "not need to worry about damaging your system."
	echo
	echo "It needs only XCode to be installed."
	echo
	echo "Usage: ./bootstrap_linux.sh"
	echo
	exit
fi

# Build everything
make -C libdeps -f ./bootstrap_darwin.mk \
 PKG_LIBSNDFILE_TAR="$PKG_LIBSNDFILE_TAR" PKG_LIBSNDFILE_DIR="$PKG_LIBSNDFILE_DIR" PKG_LIBSNDFILE_URL="$PKG_LIBSNDFILE_URL" PKG_LIBSNDFILE_MD5="$PKG_LIBSNDFILE_MD5" PKG_LIBSNDFILE_LIB="$PKG_LIBSNDFILE_LIB" \
 PKG_SDL_TAR="$PKG_SDL_TAR" PKG_SDL_DIR="$PKG_SDL_DIR" PKG_SDL_URL="$PKG_SDL_URL" PKG_SDL_MD5="$PKG_SDL_MD5" PKG_SDL_LIB="$PKG_SDL_LIB" \
 PKG_LIBSAMPLERATE_TAR="$PKG_LIBSAMPLERATE_TAR" PKG_LIBSAMPLERATE_DIR="$PKG_LIBSAMPLERATE_DIR" PKG_LIBSAMPLERATE_URL="$PKG_LIBSAMPLERATE_URL" PKG_LIBSAMPLERATE_MD5="$PKG_LIBSAMPLERATE_MD5" PKG_LIBSAMPLERATE_LIB="$PKG_LIBSAMPLERATE_LIB"
    ;;

	*)
		echo "Could not identify platform" $(uname)
	;;

esac
