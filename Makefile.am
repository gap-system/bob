bin_PROGRAMS = bob
bob_SOURCES = bob.cc gap.cc
bob_CPPFLAGS = `libs/bin/curl-config --cflags`
bob_LDADD = libs/lib/libcurl.a libs/lib/libarchive.a libs/lib/libz.a -lrt
bob_LDFLAGS = -static

bob-bob.o:    bob.cc bob.h zlib bz2lib libarchive libcurl

libs/lib/libcurl.a: libcurl

libs/lib/libz.a: zlib

libs/lib/libbz2.a: bz2lib

libs/lib/libarchive.a: libarchive

.PHONY:	libcurl zlib bz2lib libarchive

CURLVERSION = 7.24.0
LIBARCHIVEVERSION = 3.0.3
ZLIBVERSION = 1.2.6
BZ2LIBVERSION = 1.0.6

libcurl:
	( if [ ! -e curl-${CURLVERSION}.tar.gz ] ; then \
	     curl http://curl.haxx.se/download/curl-${CURLVERSION}.tar.gz \
	     -o curl-${CURLVERSION}.tar.gz ; fi )
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
	( if [ ! -e libarchive-${LIBARCHIVEVERSION}.tar.gz ] ; then \
	     curl http://libarchive.googlecode.com/files/libarchive-${LIBARCHIVEVERSION}.tar.gz \
	     -o libarchive-${LIBARCHIVEVERSION}.tar.gz ; fi )
	tar xzvf libarchive-${LIBARCHIVEVERSION}.tar.gz
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd libarchive-${LIBARCHIVEVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared \
	    --without-lzmadec --without-iconv \
	    --without-lzma --without-nettle --without-openssl \
	    --without-xml2 --without-expat --disable-acl ; \
	  make ; make install )

zlib:
	( if [ ! -e zlib-${ZLIBVERSION}.tar.gz ] ; then \
	     curl http://zlib.net/zlib-${ZLIBVERSION}.tar.gz \
	     -o zlib-${ZLIBVERSION}.tar.gz ; fi )
	tar xzvf zlib-${ZLIBVERSION}.tar.gz
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd zlib-${ZLIBVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --static ; \
	  make ; make install )

bz2lib:
	( if [ ! -e bzip2-${BZ2LIBVERSION}.tar.gz ] ; then \
	     curl http://bzip.org/${BZ2LIBVERSION}/bzip2-${BZ2LIBVERSION}.tar.gz \
	     -o bzip2-${BZ2LIBVERSION}.tar.gz ; fi )
	tar xzvf bzip2-${BZ2LIBVERSION}.tar.gz
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd bzip2-${BZ2LIBVERSION} ; \
	  make ; make install PREFIX=$$MYPATH/libs )

