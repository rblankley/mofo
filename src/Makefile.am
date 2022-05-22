
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
	moc_accountnicknamewidget.cpp \
	moc_accountsdialog.cpp \
	moc_advancedfilterlistwidget.cpp \
	moc_advancedfilterwidget.cpp \
	moc_analysiswidget.cpp \
	moc_collapsiblesplitter.cpp \
	moc_configdialog.cpp \
	moc_filtereditordialog.cpp \
	moc_filtersdialog.cpp \
	moc_filterselectiondialog.cpp \
	moc_fundamentalsviewerwidget.cpp \
	moc_gridtableheadermodel.cpp \
	moc_gridtableheaderview.cpp \
	moc_gridtableview.cpp \
	moc_hoveritemdelegate.cpp \
	moc_mainwindow.cpp \
	moc_networkaccess.cpp \
	moc_optionanalyzer.cpp \
	moc_optionanalyzerthread.cpp \
	moc_optionchainimplvolwidget.cpp \
	moc_optionchainopenintwidget.cpp \
	moc_optionchainprobwidget.cpp \
	moc_optionchainview.cpp \
	moc_optiontradingdetailsdialog.cpp \
	moc_optiontradingview.cpp \
	moc_optionviewertabwidget.cpp \
	moc_optionviewerwidget.cpp \
	moc_symboldetailsdialog.cpp \
	moc_symbolimplvolwidget.cpp \
	moc_symbolpricehistorywidget.cpp \
	moc_tddaemon.cpp \
	moc_watchlistdialog.cpp \
	moc_watchlistselectiondialog.cpp \
	moc_widgetstatesdialog.cpp \
	qrc_mofo.cpp

mofo_SOURCES = \
	$(BUILT_SOURCES) \
	abstractdaemon.cpp \
	accountnicknamewidget.cpp \
	accountsdialog.cpp \
	advancedfilterlistwidget.cpp \
	advancedfilterwidget.cpp \
	analysiswidget.cpp \
	collapsiblesplitter.cpp \
	configdialog.cpp \
	filtereditordialog.cpp \
	filtersdialog.cpp \
	filterselectiondialog.cpp \
	fundamentalsviewerwidget.cpp \
	gridtableheadermodel.cpp \
	gridtableheaderview.cpp \
	gridtableview.cpp \
	hoveritemdelegate.cpp \
	main.cpp \
	mainwindow.cpp \
	networkaccess.cpp \
	optionanalyzer.cpp \
	optionanalyzerthread.cpp \
	optionchainimplvolwidget.cpp \
	optionchainopenintwidget.cpp \
	optionchainprobwidget.cpp \
	optionchainview.cpp \
	optionprofitcalc.cpp \
	optionprofitcalcfilter.cpp \
	optiontradingdetailsdialog.cpp \
	optiontradingview.cpp \
	optionviewertabwidget.cpp \
	optionviewerwidget.cpp \
	symboldetailsdialog.cpp \
	symbolimplvolwidget.cpp \
	symbolpricehistorywidget.cpp \
	tableheaderitem.cpp \
	tddaemon.cpp \
	watchlistdialog.cpp \
	watchlistselectiondialog.cpp \
	widgetstatesdialog.cpp

CLEANFILES = $(BUILT_SOURCES)
