
SUBDIRS = apibase calc db tda usdot util

# ------------
# Distribution
# ------------

pkgsysconfdir = $(sysconfdir)/$(PACKAGE)

dist_pkgsysconf_DATA = \
	endpoints.config \
	logging.config

# -------
# Program
# -------

bin_PROGRAMS = mofo

mofo_CXXFLAGS=$(CLIO_CFLAGS)

mofo_LDADD = \
	tda/lib_mofo_tda.a \
	usdot/lib_mofo_usdot.a \
	apibase/lib_mofo_apibase.a \
	calc/lib_mofo_calc.a \
	db/lib_mofo_db.a \
	util/lib_mofo_util.a \
	$(CLIO_LIBS)

BUILT_SOURCES = \
	moc_abstractdaemon.cpp \
	moc_configdialog.cpp \
	moc_gridtableheadermodel.cpp \
	moc_gridtableheaderview.cpp \
	moc_gridtableview.cpp \
	moc_hoveritemdelegate.cpp \
	moc_mainwindow.cpp \
	moc_networkaccess.cpp \
	moc_optionchainview.cpp \
	moc_optiontradingview.cpp \
	moc_optionviewertabwidget.cpp \
	moc_optionviewerwidget.cpp \
	moc_tddaemon.cpp \
	moc_watchlistdialog.cpp \
	qrc_mofo.cpp

mofo_SOURCES = \
	$(BUILT_SOURCES) \
	abstractdaemon.cpp \
	configdialog.cpp \
	gridtableheadermodel.cpp \
	gridtableheaderview.cpp \
	gridtableview.cpp \
	hoveritemdelegate.cpp \
	main.cpp \
	mainwindow.cpp \
	networkaccess.cpp \
	optionchainview.cpp \
	optionprofitcalc.cpp \
	optiontradingview.cpp \
	optionviewertabwidget.cpp \
	optionviewerwidget.cpp \
	tableheaderitem.cpp \
	tddaemon.cpp \
	watchlistdialog.cpp

CLEANFILES = $(BUILT_SOURCES)
