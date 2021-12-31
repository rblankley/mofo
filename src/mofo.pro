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
    analysiswidget.cpp \
    apibase/abstractapi.cpp \
    apibase/serializedapi.cpp \
    apibase/serializedjsonapi.cpp \
    apibase/serializedxmlapi.cpp \
    calc/expectedvaluecalc.cpp \
    calc/montecarlocalc.cpp \
    configdialog.cpp \
    db/appdb.cpp \
    db/fundamentalstablemodel.cpp \
    db/itemmodel.cpp \
    db/optionchaintablemodel.cpp \
    db/optiontradingitemmodel.cpp \
    db/quotetablemodel.cpp \
    db/sqldb.cpp \
    db/sqltablemodel.cpp \
    db/symboldb.cpp \
    filtereditordialog.cpp \
    filtersdialog.cpp \
    filterselectiondialog.cpp \
    gridtableheadermodel.cpp \
    gridtableheaderview.cpp \
    gridtableview.cpp \
    hoveritemdelegate.cpp \
    mainwindow.cpp \
    optionanalyzer.cpp \
    optionanalyzerthread.cpp \
    optionchainview.cpp \
    optionprofitcalc.cpp \
    optionprofitcalcfilter.cpp \
    optiontradingview.cpp \
    optionviewertabwidget.cpp \
    optionviewerwidget.cpp \
    symboldetailsdialog.cpp \
    symbolpricehistorywidget.cpp \
    tableheaderitem.cpp \
    tda/dbadaptertd.cpp \
    tda/tdapi.cpp \
    tda/tdcredentialsdialog.cpp \
    tda/tdoauthapi.cpp \
    main.cpp \
    networkaccess.cpp \
    tddaemon.cpp \
    usdot/dbadapterusdot.cpp \
    usdot/usdotapi.cpp \
    util/abstractoptionpricing.cpp \
    util/alttrinomial.cpp \
    util/baroneadesiwhaley.cpp \
    util/binomial.cpp \
    util/bjerksundstensland02.cpp \
    util/bjerksundstensland93.cpp \
    util/blackscholes.cpp \
    util/cbnd.cpp \
    util/cnd.cpp \
    util/coxrossrubinstein.cpp \
    util/equalprobbinomial.cpp \
    util/fitpoly.cpp \
    util/kamradritchken.cpp \
    util/montecarlo.cpp \
    util/newtonraphson.cpp \
    util/phelimboyle.cpp \
    util/rollgeskewhaley.cpp \
    util/stats.cpp \
    util/tests.cpp \
    util/trinomial.cpp \
    watchlistdialog.cpp \
    watchlistselectiondialog.cpp \
    widgetstatesdialog.cpp

HEADERS += \
    abstractdaemon.h \
    analysiswidget.h \
    apibase/abstractapi.h \
    apibase/serializedapi.h \
    apibase/serializedjsonapi.h \
    apibase/serializedxmlapi.h \
    calc/abstractevcalc.h \
    calc/basiccalc.h \
    calc/binomialcalc.h \
    calc/expectedvaluecalc.h \
    calc/montecarlocalc.h \
    calc/trinomialcalc.h \
    configdialog.h \
    db/appdb.h \
    db/candledata.h \
    db/fundamentalstablemodel.h \
    db/itemmodel.h \
    db/optionchaintablemodel.h \
    db/optiontradingitemmodel.h \
    db/quotetablemodel.h \
    db/sqldb.h \
    db/sqltablemodel.h \
    db/stringsdb.h \
    db/symboldb.h \
    filtereditordialog.h \
    filtersdialog.h \
    filterselectiondialog.h \
    gridtableheadermodel.h \
    gridtableheaderview.h \
    gridtableview.h \
    hoveritemdelegate.h \
    mainwindow.h \
    optionanalyzer.h \
    optionanalyzerthread.h \
    optionchainview.h \
    optionprofitcalc.h \
    optionprofitcalcfilter.h \
    optiontradingview.h \
    optionviewertabwidget.h \
    optionviewerwidget.h \
    symboldetailsdialog.h \
    symbolpricehistorywidget.h \
    tableheaderitem.h \
    tda/dbadaptertd.h \
    tda/stringsjson.h \
    tda/stringsoauth.h \
    tda/tdapi.h \
    tda/tdcredentialsdialog.h \
    tda/tdoauthapi.h \
    common.h \
    networkaccess.h \
    tddaemon.h \
    usdot/dbadapterusdot.h \
    usdot/stringsxml.h \
    usdot/usdotapi.h \
    util/abstractoptionpricing.h \
    util/altbisection.h \
    util/alttrinomial.h \
    util/baroneadesiwhaley.h \
    util/binomial.h \
    util/bisection.h \
    util/bjerksundstensland02.h \
    util/bjerksundstensland93.h \
    util/blackscholes.h \
    util/coxrossrubinstein.h \
    util/dualmodeoptionpricing.h \
    util/equalprobbinomial.h \
    util/fitpoly.h \
    util/kamradritchken.h \
    util/montecarlo.h \
    util/newtonraphson.h \
    util/optiontype.h \
    util/phelimboyle.h \
    util/rollgeskewhaley.h \
    util/stats.h \
    util/tests.h \
    util/trinomial.h \
    watchlistdialog.h \
    watchlistselectiondialog.h \
    widgetstatesdialog.h

RESOURCES += \
    mofo.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# libclio
contains(DEFINES, HAVE_CLIO_H) {
    win32 {
        CONFIG(release, debug|release): LIBS += -L$$PWD/../../libclio/x64/release/ -lclio
        else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../libclio/x64/debug/ -lclio

        INCLUDEPATH += $$PWD/../../libclio/src
        DEPENDPATH += $$PWD/../../libclio/src

        CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libclio/x64/release/clio.lib
        else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libclio/x64/debug/clio.lib
    } else {
        PKGCONFIG += clio
    }
}
