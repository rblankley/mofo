
noinst_LIBRARIES = lib_mofo_apibase.a

lib_mofo_apibase_a_CXXFLAGS = -I../ $(CLIO_CFLAGS)

BUILT_SOURCES = \
	moc_abstractapi.cpp \
	moc_serializedapi.cpp \
	moc_serializedjsonapi.cpp \
	moc_serializedxmlapi.cpp

lib_mofo_apibase_a_SOURCES = \
	$(BUILT_SOURCES) \
	abstractapi.cpp \
	serializedapi.cpp \
	serializedjsonapi.cpp \
	serializedxmlapi.cpp

CLEANFILES = $(BUILT_SOURCES)
