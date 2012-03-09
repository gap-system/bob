bin_PROGRAMS = bob
bob_SOURCES = bob.cc gap.cc
bob_CPPFLAGS = `libs/bin/curl-config --cflags`
bob_LDADD = libs/lib/libcurl.a libs/lib/libarchive.a libs/lib/libz.a -lrt

CURLVERSION = 7.24.0
LIBARCHIVEVERSION = 3.0.3
ZLIBVERSION = 1.2.6

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
	    --without-libidn --disable-threaded-resolver ; \
	  make ; make install )

libarchive:
	curl http://libarchive.googlecode.com/files/libarchive-${LIBARCHIVEVERSION}.tar.gz \
	     -o libarchive-${LIBARCHIVEVERSION}.tar.gz
	tar xzvf libarchive-${LIBARCHIVEVERSION}.tar.gz
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd libarchive-${LIBARCHIVEVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared \
	    --without-bz2lib --without-lzmadec --without-iconv \
	    --without-lzma --without-nettle --without-openssl \
	    --without-xml2 --without-expat --disable-acl ; \
	  make ; make install )

libz:
	curl http://zlib.net/zlib-${ZLIBVERSION}.tar.gz \
	     -o zlib-${ZLIBVERSION}.tar.gz
	tar xzvf zlib-${ZLIBVERSION}.tar.gz
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd zlib-${ZLIBVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --static ; \
	  make ; make install )

