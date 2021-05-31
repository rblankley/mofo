QT       += core gui network networkauth sql concurrent xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += link_pkgconfig

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Uncomment this to use clio logging library. You may need to adjust
# include and lib paths below.
#DEFINES += HAVE_CLIO_H


SOURCES += \
    abstractdaemon.cpp \
    apibase/abstractapi.cpp \
    apibase/serializedapi.cpp \
    apibase/serializedjsonapi.cpp \
    apibase/serializedxmlapi.cpp \
    configdialog.cpp \
    db/appdb.cpp \
    db/optionchaintablemodel.cpp \
    db/quotetablemodel.cpp \
    db/sqldb.cpp \
    db/sqltablemodel.cpp \
    db/symboldb.cpp \
    gridtableheadermodel.cpp \
    gridtableheaderview.cpp \
    gridtableview.cpp \
    hoveritemdelegate.cpp \
    mainwindow.cpp \
    optionchainview.cpp \
    optionviewertabwidget.cpp \
    optionviewerwidget.cpp \
    tableheaderitem.cpp \
    tda/dbadaptertd.cpp \
    tda/tdapi.cpp \
    tda/tdoauthapi.cpp \
    main.cpp \
    networkaccess.cpp \
    tddaemon.cpp \
    usdot/dbadapterusdot.cpp \
    usdot/usdotapi.cpp \
    util/abstractoptionpricing.cpp \
    util/baroneadesiwhaley.cpp \
    util/binomial.cpp \
    util/bjerksundstensland.cpp \
    util/blackscholes.cpp \
    util/cbnd.cpp \
    util/cnd.cpp \
    util/coxrossrubinstein.cpp \
    util/equalprobbinomial.cpp \
    util/montecarlo.cpp \
    util/newtonraphson.cpp \
    util/phelimboyle.cpp \
    util/rollgeskewhaley.cpp \
    util/stats.cpp \
    util/tests.cpp \
    watchlistdialog.cpp

HEADERS += \
    abstractdaemon.h \
    apibase/abstractapi.h \
    apibase/serializedapi.h \
    apibase/serializedjsonapi.h \
    apibase/serializedxmlapi.h \
    configdialog.h \
    db/appdb.h \
    db/optionchaintablemodel.h \
    db/quotetablemodel.h \
    db/sqldb.h \
    db/sqltablemodel.h \
    db/stringsdb.h \
    db/symboldb.h \
    gridtableheadermodel.h \
    gridtableheaderview.h \
    gridtableview.h \
    hoveritemdelegate.h \
    mainwindow.h \
    optionchainview.h \
    optionviewertabwidget.h \
    optionviewerwidget.h \
    tableheaderitem.h \
    tda/dbadaptertd.h \
    tda/stringsjson.h \
    tda/stringsoauth.h \
    tda/tdapi.h \
    tda/tdoauthapi.h \
    common.h \
    networkaccess.h \
    tddaemon.h \
    usdot/dbadapterusdot.h \
    usdot/stringsxml.h \
    usdot/usdotapi.h \
    util/abstractoptionpricing.h \
    util/baroneadesiwhaley.h \
    util/binomial.h \
    util/bisection.h \
    util/bjerksundstensland.h \
    util/blackscholes.h \
    util/coxrossrubinstein.h \
    util/dualmodeoptionpricing.h \
    util/equalprobbinomial.h \
    util/montecarlo.h \
    util/newtonraphson.h \
    util/optiontype.h \
    util/phelimboyle.h \
    util/rollgeskewhaley.h \
    util/stats.h \
    util/tests.h \
    watchlistdialog.h

RESOURCES += \
    mofo.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# libclio
contains(DEFINES, HAVE_CLIO_H) {
    win32 {
        CONFIG(release, debug|release): LIBS += -L$$PWD/../libclio/x64/release/ -lclio
        else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libclio/x64/debug/ -lclio

        INCLUDEPATH += $$PWD/../libclio/src
        DEPENDPATH += $$PWD/../libclio/src
    } else {
        PKGCONFIG += clio
    }
}
