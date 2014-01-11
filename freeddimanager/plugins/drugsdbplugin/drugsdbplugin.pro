TEMPLATE        = lib
TARGET          = DrugsDb

DEFINES += DRUGSDB_LIBRARY

include(../../../plugins/fmf_plugins.pri)
include($${PWD}/drugsdbplugin_dependencies.pri)

INCLUDEPATH += ../
DEPENDPATH += ../

HEADERS += \
    constants.h \
    drugsdbplugin.h \


SOURCES += \
    drugsdbplugin.cpp \
    constants.cpp \


OTHER_FILES += DrugsDb.pluginspec

#equals(TEST, 1) {
#    SOURCES += \
#        tests/test_.cpp \
#        tests/test_.cpp \
#        tests/test_init_clean.cpp
#}

# include translations
TRANSLATION_NAME = drugsdb
include($${SOURCES_ROOT_PATH}/buildspecs/translations.pri)
