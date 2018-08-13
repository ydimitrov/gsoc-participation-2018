TEMPLATE = app

load(configure)

qtCompileTest(libhomescreen)
qtCompileTest(qlibwindowmanager)

config_libhomescreen {
    CONFIG += link_pkgconfig
    PKGCONFIG += libhomescreen
    DEFINES += HAVE_LIBHOMESCREEN
}

config_qlibwindowmanager {
    CONFIG += link_pkgconfig
    PKGCONFIG += qlibwindowmanager
    DEFINES += HAVE_QLIBWINDOWMANAGER
}

DESTDIR = $${OUT_PWD}/../package/root/bin
