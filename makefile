INSTALL_PATH?=/usr/local

build: gblinkdl.cpp typedefs.h ppgb.h ppgb.c
	gcc -c -o ppgb.o ppgb.c
	g++ -c -o gblinkdl.o gblinkdl.cpp
	g++ -o gblinkdx ppgb.o gblinkdl.o

install: build
	mv gblinkdx $(INSTALL_PATH)/bin
