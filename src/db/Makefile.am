
noinst_LIBRARIES = lib_mofo_db.a

lib_mofo_db_a_CXXFLAGS = -I../ $(CLIO_CFLAGS)

BUILT_SOURCES = \
	moc_appdb.cpp \
	moc_itemmodel.cpp \
	moc_optionchaintablemodel.cpp \
	moc_optiontradingitemmodel.cpp \
	moc_quotetablemodel.cpp \
	moc_sqldb.cpp \
	moc_sqltablemodel.cpp \
	moc_symboldb.cpp

lib_mofo_db_a_SOURCES = \
	$(BUILT_SOURCES) \
	appdb.cpp \
	itemmodel.cpp \
	optionchaintablemodel.cpp \
	optiontradingitemmodel.cpp \
	quotetablemodel.cpp \
	sqldb.cpp \
	sqltablemodel.cpp \
	symboldb.cpp

CLEANFILES = $(BUILT_SOURCES)
