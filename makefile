INSTALL_PATH?=/usr/local

build: gblinkdl.cpp typedefs.h
	g++ -o gblinkdx gblinkdl.cpp -I.

install: build
	mv gblinkdx $(INSTALL_PATH)/bin
