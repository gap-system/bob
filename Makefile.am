bin_PROGRAMS = bob
bob_SOURCES = bob.cc *.cc
bob_CPPFLAGS = `bin/curl-config --cflags`
bob_LDADD = lib/libcurl.a -lrt

curl:
	tar xzvf curl-7.24.0.tar.gz
	cd curl-7.24.0
	./configure --prefix=../libs --disable-shared \
	  --disable-imap --disable-imaps --disable-ldap --disable-ldaps \
	  --disable-pop3 --disable-pop3s --disable-rtsp --disable-smtp \
	  --disable-smtps --disable-telnet --disable-gopher --without-ssl \
	  --without-zlib --without-libidn --disable-threaded-resolver
	make
	make install

