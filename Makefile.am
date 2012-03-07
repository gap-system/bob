bin_PROGRAMS = bob
bob_SOURCES = bob.cc gap.cc io.cc
bob_CPPFLAGS = `libs/bin/curl-config --cflags`
bob_LDADD = libs/lib/libcurl.a -lrt

CURLVERSION = 7.24.0

libcurl:
	curl http://curl.haxx.se/download/curl-${CURLVERSION}.tar.gz \
	     -o curl-${CURLVERSION}.tar.gz
	tar xzvf curl-${CURLVERSION}.tar.gz
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd curl-${CURLVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared \
	    --disable-imap --disable-imaps --disable-ldap --disable-ldaps \
	    --disable-pop3 --disable-pop3s --disable-rtsp --disable-smtp \
	    --disable-smtps --disable-telnet --disable-gopher --without-ssl \
	    --without-zlib --without-libidn --disable-threaded-resolver ; \
	  make ; make install )

