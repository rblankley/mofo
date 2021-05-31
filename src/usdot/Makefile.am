
noinst_LIBRARIES = lib_mofo_usdot.a

lib_mofo_usdot_a_CXXFLAGS = -I../ $(CLIO_CFLAGS)

BUILT_SOURCES = \
	moc_dbadapterusdot.cpp \
	moc_usdotapi.cpp

lib_mofo_usdot_a_SOURCES = \
	$(BUILT_SOURCES) \
	dbadapterusdot.cpp \
	usdotapi.cpp

CLEANFILES = $(BUILT_SOURCES)
