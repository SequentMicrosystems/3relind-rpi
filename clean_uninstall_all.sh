sudo make uninstall && sudo make distclean && sudo make -f Makefile_relay uninstall && sudo make -f Makefile_relay clean
rm -f ./Makefile.in
rm -f ./configure
rm -f ./config.h.in~
rm -f ./config.h.in
rm -f ./aclocal.m4
rm -f ./src/Makefile.in
rm -rf ./src/.deps
rm -rf ./build-aux
rm -rf ./autom4te.cache
rm -f ./m4/libtool.m4
rm -f ./m4/lt~obsolete.m4
rm -f ./m4/ltoptions.m4
rm -f ./m4/ltsugar.m4
rm -f ./m4/ltversion.m4
