#!/bin/sh
PKG_CONFIG_SYSROOT_DIR=/opt/poky-agl/5.0.3/sysroots/armv7vehf-neon-vfpv4-agl-linux-gnueabi
export PKG_CONFIG_SYSROOT_DIR
PKG_CONFIG_LIBDIR=/opt/poky-agl/5.0.3/sysroots/armv7vehf-neon-vfpv4-agl-linux-gnueabi/usr/lib/pkgconfig:/opt/poky-agl/5.0.3/sysroots/armv7vehf-neon-vfpv4-agl-linux-gnueabi/usr/share/pkgconfig
export PKG_CONFIG_LIBDIR
exec pkg-config "$@"
