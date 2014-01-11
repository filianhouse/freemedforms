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
    drugdatabasedescription.h \
#    drugdatatabasestep.h \
#    idrugdatabasestep.h \
#    idrugdatabasestepwidget.h \
#    countries/be/belgishdrugsdatabase.h \
#    countries/ca/canadiandrugsdatabase.h \
#    countries/fr/frenchdrugsdatabasecreator.h \
#    countries/pt/portuguesedrugsdatabase.h \
#    countries/us/fdadrugsdatabasecreator.h \
#    countries/za/southafricandrugsdatabase.h \


SOURCES += \
    drugsdbplugin.cpp \
    constants.cpp \
    drugdatabasedescription.cpp \
#    drugdatatabasestep.cpp \
#    idrugdatabasestep.cpp \
#    idrugdatabasestepwidget.cpp \
#    countries/be/belgishdrugsdatabase.cpp \
#    countries/ca/canadiandrugsdatabase.cpp \
#    countries/fr/frenchdrugsdatabasecreator.cpp \
#    countries/pt/portuguesedrugsdatabase.cpp \
#    countries/us/fdadrugsdatabasecreator.cpp \
#    countries/za/southafricandrugsdatabase.cpp \


FORMS += \
#    idrugdatabasestepwidget.ui \
#    countries/za/southafricandrugsdatabase.ui \
#    countries/us/fdadrugsdatabasewidget.ui \
#    countries/pt/portuguesedrugsdatabase.ui \
#    countries/fr/frenchdrugsdatabasewidget.ui \
#    countries/be/belgishdrugsdatabase.ui \
#    countries/ca/canadiandrugsdatabasewidget.ui \


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
