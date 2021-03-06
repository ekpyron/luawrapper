#!/bin/sh -e

cd ../../..

if [ "$1" = "clean" ]; then
  rm -rf LuaJIT.framework
  exit 0
fi

IXCODE=`xcode-select -print-path`

if [ -d LuaJIT.framework ]; then
  exit 0
fi

if [ ! -d luajit ]; then
  echo "Cannot find luajit. Download luajit a folder named 'luajit' in the same directory as luawrapper." 1>&2
  exit 1
fi

cd luajit

if [ ! -d lib/temp ]; then
    mkdir -p lib/temp
fi

echo "Build for MacOS (x86_64)"
ISDK=$IXCODE/Platforms/MacOSX.platform/Developer
ISDKVER=MacOSX.sdk
ISDKP=/usr/bin/
ISDKF="-arch x86_64 -isysroot $ISDK/SDKs/$ISDKVER"

make clean
make TARGET_FLAGS="-arch x86_64"
#mv src/libluajit.a lib/temp/libluajit-macos-x86_64.a

echo "Build for iOS device (armv7)"
ISDK=$IXCODE/Platforms/iPhoneOS.platform/Developer
ISDKVER=iPhoneOS.sdk
ISDKP=/usr/bin/
ISDKF="-arch armv7 -miphoneos-version-min=6.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CC="gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" \
     TARGET_SYS=iOS CFLAGS="-DLJ_NO_SYSTEM"
mv src/libluajit.a lib/temp/libluajit-ios-armv7.a

echo "Build for iOS device (armv7s)"
ISDKF="-arch armv7s -miphoneos-version-min=6.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CC="gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" \
     TARGET_SYS=iOS BUILDMODE="static" CFLAGS="-DLJ_NO_SYSTEM"
mv src/libluajit.a lib/temp/libluajit-ios-armv7s.a

echo "Build for iOS device (arm64)"
ISDKF="-arch arm64 -miphoneos-version-min=6.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make CROSS=$ISDKP TARGET_FLAGS="$ISDKF" \
     TARGET_SYS=iOS BUILDMODE="static" CFLAGS="-DLJ_NO_SYSTEM"
mv src/libluajit.a lib/temp/libluajit-ios-arm64.a

echo "Build for iOS simulator"
ISDK=$IXCODE/Platforms/iPhoneSimulator.platform/Developer
ISDKVER=iPhoneSimulator.sdk
ISDKP=/usr/bin/
ISDKF="-arch i386 -mios-simulator-version-min=6.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CFLAGS="-arch i386" HOST_LDFLAGS="-arch i386" TARGET_SYS=iOS TARGET=x86 CROSS=$ISDKP TARGET_FLAGS="$ISDKF" \
     TARGET_SYS=iOS BUILDMODE="static" CFLAGS="-DLJ_NO_SYSTEM"
mv src/libluajit.a lib/temp/libluajit-simulator.a

echo "Build for iOS simulator (64-bit)"
ISDK=$IXCODE/Platforms/iPhoneSimulator.platform/Developer
ISDKVER=iPhoneSimulator.sdk
ISDKP=/usr/bin/
ISDKF="-arch x86_64 -mios-simulator-version-min=6.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make TARGET_SYS=iOS TARGET=x86_64 CROSS=$ISDKP TARGET_FLAGS="$ISDKF" \
     TARGET_SYS=iOS BUILDMODE="static" CFLAGS="-DLJ_NO_SYSTEM"
mv src/libluajit.a lib/temp/libluajit-simulator64.a

# Combine all archives to one.
mkdir ../LuaJIT.framework
libtool -o ../LuaJIT.framework/LuaJIT lib/temp/*.a 2> /dev/null
rm -rf lib/temp
make clean

mkdir ../LuaJIT.framework/Headers
cp src/lua.h src/lauxlib.h src/lualib.h src/luajit.h src/lua.hpp src/luaconf.h ../LuaJIT.framework/Headers
