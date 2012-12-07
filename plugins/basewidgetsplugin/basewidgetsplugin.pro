TEMPLATE = lib
TARGET = BaseWidgets

DEFINES += BASEFORMWIDGETS_LIBRARY
with-pad { DEFINES += WITH_PAD }

BUILD_PATH_POSTFIXE = FreeMedForms

include(../fmf_plugins.pri)
include(basewidgetsplugin_dependencies.pri)

HEADERS += basewidgetsplugin.h \
    baseformwidgets.h \
    baseformwidgetsoptionspage.h \
    texteditorfactory.h \
    identitywidgetfactory.h \
    calculationwidgets.h \
    frenchsocialnumber.h \
    basedetailswidget.h \
    basedatecompleterwidget.h \
    constants.h

SOURCES += basewidgetsplugin.cpp \
    baseformwidgets.cpp \
    baseformwidgetsoptionspage.cpp \
    texteditorfactory.cpp \
    identitywidgetfactory.cpp \
    frenchsocialnumber.cpp \
    calculationwidgets.cpp \
    basedetailswidget.cpp \
    basedatecompleterwidget.cpp \
    constants.cpp

FORMS += baseformwidgetsoptionspage.ui \
    baseformwidget.ui \
    frenchsocialnumber.ui \
    austriansocialnumber.ui


OTHER_FILES = BaseWidgets.pluginspec


