# This file should be included by the Makefile
# in each language directory

local-all: symlinks
	$(MAKE) html

symlinks:
	ln -fs ../../html html
	ln -fs ../../xsl xsl

# for XML_SOURCES
include ../../Makefile

# XML gettext tools
PO2XML=po2xml

$(XML_SOURCES): %.xml: %.xml.po
	$(PO2XML) ../../$@ $< > $@

