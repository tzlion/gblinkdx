INSTALL_PATH?=/usr/local

build: gblinkdl.cpp stdafx.cpp cbin.h stdafx.h typedefs.h
	g++ -o gblinkdx gblinkdl.cpp -I.

install: build
	mv gblinkdx $(INSTALL_PATH)/bin
