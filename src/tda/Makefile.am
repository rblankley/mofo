
noinst_LIBRARIES = lib_mofo_tda.a

lib_mofo_tda_a_CXXFLAGS = -I../ $(CLIO_CFLAGS)

BUILT_SOURCES = \
	moc_dbadaptertd.cpp \
	moc_tdapi.cpp \
	moc_tdcredentialsdialog.cpp \
	moc_tdoauthapi.cpp

lib_mofo_tda_a_SOURCES = \
	$(BUILT_SOURCES) \
	dbadaptertd.cpp \
	tdapi.cpp \
	tdcredentialsdialog.cpp \
	tdoauthapi.cpp

CLEANFILES = $(BUILT_SOURCES)
