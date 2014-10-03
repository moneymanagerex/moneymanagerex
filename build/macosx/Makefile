# Makefile to build a Mac OS X distribution package (.dmg)
#
#

TARGET=mmex
RELDIR=../../

MAJORVERSION := $(shell grep wxString\ MMEX_VERSION ../../src/constants.cpp | awk -F\" '{print $$2;}' | awk -F. '{print $$1}')
MINORVERSION := $(shell grep wxString\ MMEX_VERSION ../../src/constants.cpp | awk -F\" '{print $$2;}' | awk -F. '{print $$2}')
REVISION_NUM := $(shell grep wxString\ MMEX_VERSION ../../src/constants.cpp | awk -F\" '{print $$2;}' | awk -F. '{print $$3}')
RELEASE_SPECIAL := $(shell grep define\ MMEX_BUILD_TYPE_RELEASE_CANDIDATE ../../src/constants.cpp | tail -1 | awk '{print $$3}')
#RELEASE_SPECIAL := $(shell grep int\ MMEX_VERSION_DEVELOPMENT_COUNT ../../src/constants.cpp | awk '{print $$5}' | tr -d ";")
RELEASENUM := $(MAJORVERSION).$(MINORVERSION).$(REVISION_NUM)
ifdef RELEASE_SPECIAL
RELEASENAME := $(RELEASENUM)-RC$(RELEASE_SPECIAL)
else
RELEASENAME := $(RELEASENUM)
endif

CPBIN=/bin/cp
RMBIN=/bin/rm
MDBIN=/bin/mkdir -p


SRCDIR=./dmg/
DMGNAME = ../../$(TARGET)-$(RELEASENAME).dmg
APPBNDL=$(TARGET).app
DOCS := ../../README.TXT ../../doc/license.txt ../../doc/version.txt 


all: dist


#dmg-background.png: dmg-background.svg
#	inkscape -z --file=dmg-background.svg --export-png=dmg-background.png --export-dpi=72
#	inkscape -z --file=dmg-background.svg --export-png=dmg-background.png --export-height=400 --export-width=500


dist: 
	$(MDBIN) $(SRCDIR)
	$(RMBIN) -f $(SRCDIR).DS_Store
	$(CPBIN) $(DOCS) $(SRCDIR)
	$(CPBIN) -R $(RELDIR)$(APPBNDL) $(SRCDIR)
	../../../yoursway-create-dmg/create-dmg \
			--window-size 500 400 \
			--background ./dmg-background.png \
			--text-size 14 \
			--icon-size 40 \
			--volname "MoneyManagerEX" \
			--volicon ../../graphics/mmex.icns \
			--icon "$(APPBNDL)" 125 200 \
			--app-drop-link 370 200 \
			--icon "README.TXT" 125 325 \
			--icon "version.txt" 250 325 \
			--icon "license.txt" 370 325 \
		$(DMGNAME) \
		$(SRCDIR)

clean:
	$(RMBIN) -f $(DMGNAME)


grep-test:
	echo $(MAJORVERSION)
	echo $(MINORVERSION)
	echo $(REVISION_NUM)
	echo $(RELEASE_SPECIAL)
	echo $(RELEASENUM)
	echo $(RELEASENAME)


