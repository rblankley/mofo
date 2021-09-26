
noinst_LIBRARIES = lib_mofo_calc.a

lib_mofo_calc_a_CXXFLAGS = -I../ $(CLIO_CFLAGS)

BUILT_SOURCES = \
	moc_binomialcalc.cpp \
	moc_blackscholescalc.cpp \
	moc_expectedvaluecalc.cpp

lib_mofo_calc_a_SOURCES = \
	$(BUILT_SOURCES) \
	binomialcalc.cpp \
	blackscholescalc.cpp \
	expectedvaluecalc.cpp

CLEANFILES = $(BUILT_SOURCES)
