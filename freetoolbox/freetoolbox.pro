include(../buildspecs/checkqtversion.pri)
TEMPLATE = subdirs
CONFIG *= ordered

SUBDIRS = appnamedefine
# Do not build required libs
# (ex: linux integrated of apps dependent of the freemedforms-libs package)
!CONFIG(dontbuildlibs): SUBDIRS+=libs

SUBDIRS += plugins freetoolbox-src
