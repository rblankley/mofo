# DOXYGEN SUPPORT
@DX_RULES@

SUBDIRS = src

# allow automake to process markup README
README: README.md
	pandoc -f markdown -t plain --wrap=none $< -o $@

CLEANFILES = README

MOSTLYCLEANFILES = $(DX_CLEANFILES)

