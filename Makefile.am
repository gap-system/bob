FORLDADD=@FORLDADD@
FORLDFLAGS=@FORLDFLAGS@
bin_PROGRAMS = bob
bob_SOURCES = bob.cc gap.cc
bob_CPPFLAGS = `libs/bin/curl-config --cflags`
bob_LDADD = libs/lib/libcurl.a libs/lib/libarchive.a libs/lib/libz.a libs/lib/libbz2.a libs/lib/libcrypto.a ${FORLDADD}
bob_LDFLAGS = ${FORLDFLAGS}

BUILT_SOURCES = libs/include/zlib.h libs/include/bzlib.h \
		libs/include/archive.h libs/include/curl/curl.h \
		libs/include/openssl/ssl.h

ZLIBVERSION = 1.2.6
BZ2LIBVERSION = 1.0.6
LIBARCHIVEVERSION = 3.0.3
CURLVERSION = 7.24.0
OPENSSLVERSION = 1.0.0g

distclean-local:
	rm -rf libs

CURLARCH = curl-${CURLVERSION}.tar.gz
LIBARCHIVEARCH = libarchive-${LIBARCHIVEVERSION}.tar.gz
ZLIBARCH = zlib-${ZLIBVERSION}.tar.gz
BZ2LIBARCH = bzip2-${BZ2LIBVERSION}.tar.gz
OPENSSLARCH = openssl-${OPENSSLVERSION}.tar.gz

libs/include/zlib.h:
	( if [ ! -e ${ZLIBARCH} ] ; then \
	     curl http://zlib.net/${ZLIBARCH} -o ${ZLIBARCH} ; fi )
	tar xzvf ${ZLIBARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd zlib-${ZLIBVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --static ; \
	  make ; make install )

libs/include/bzlib.h:
	( if [ ! -e ${BZ2LIBARCH} ] ; then \
	     curl http://bzip.org/${BZ2LIBVERSION}/${BZ2LIBARCH} \
	     -o ${BZ2LIBARCH} ; fi )
	tar xzvf ${BZ2LIBARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd bzip2-${BZ2LIBVERSION} ; \
	  make ; make install PREFIX=$$MYPATH/libs )

libs/include/curl/curl.h:
	( if [ ! -e ${CURLARCH} ] ; then \
	     curl http://curl.haxx.se/download/${CURLARCH} \
	     -o ${CURLARCH} ; fi )
	tar xzvf ${CURLARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd curl-${CURLVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared \
	    --disable-imap --disable-imaps --disable-ldap --disable-ldaps \
	    --disable-pop3 --disable-pop3s --disable-rtsp --disable-smtp \
	    --disable-smtps --disable-telnet --disable-gopher --without-ssl \
	    --without-libidn --disable-threaded-resolver --disable-thread \
	    --without-libssh2 \
	    CPPFLAGS="-I$$MYPATH/libs/include" \
	    LDFLAGS="-L$$MYPATH/libs/lib" ; \
	  make ; make install )

libs/include/archive.h:
	( if [ ! -e ${LIBARCHIVEARCH} ] ; then \
	     curl http://libarchive.googlecode.com/files/${LIBARCHIVEARCH} \
	     -o ${LIBARCHIVEARCH} ; fi )
	tar xzvf ${LIBARCHIVEARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd libarchive-${LIBARCHIVEVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared \
	    --without-lzmadec --without-iconv \
	    --without-lzma --without-nettle --without-openssl \
	    --without-xml2 --without-expat --disable-acl \
	    CPPFLAGS="-I$$MYPATH/libs/include" \
	    LDFLAGS="-L$$MYPATH/libs/lib" ; \
	  make ; make install )

libs/include/openssl/ssl.h:
	( if [ ! -e ${OPENSSLARCH} ] ; then \
	    curl http://www.openssl.org/source/${OPENSSLARCH} \
	     -o ${OPENSSLARCH} ; fi )
	tar xzvf ${OPENSSLARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd openssl-${OPENSSLVERSION} ; \
	  ./config --prefix=$$MYPATH/libs --openssldir=$$MYPATH/libs/openssl \
	    no-threads no-shared no-sse2 zlib ; \
	  make ; make test ; make install )

