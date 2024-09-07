INSTALL_PATH?=/usr/local

build: gblinkdl.cpp typedefs.h ppgb.h ppgb.c
	g++ -o gblinkdx gblinkdl.cpp ppgb.c  -I.

install: build
	mv gblinkdx $(INSTALL_PATH)/bin
