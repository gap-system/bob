FORLDADD=@FORLDADD@
FORLDFLAGS=@FORLDFLAGS@
bin_PROGRAMS = bob
bob_SOURCES = bob.cc gap.cc hl_sha1.cc
bob_CPPFLAGS = `libs/bin/curl-config --cflags` -Wall -Wextra
bob_LDADD = libs/lib/libcurl.a libs/lib/libarchive.a libs/lib/libz.a libs/lib/libbz2.a libs/lib/liblzma.a ${FORLDADD}
bob_LDFLAGS = ${FORLDFLAGS}

BUILT_SOURCES = libs/include/zlib.h libs/include/bzlib.h \
		libs/include/lzma.h libs/include/archive.h \
		libs/include/curl/curl.h

ZLIBVERSION = 1.2.7
BZ2LIBVERSION = 1.0.6
LIBARCHIVEVERSION = 3.0.3
CURLVERSION = 7.24.0
OPENSSLVERSION = 1.0.0g
XZUTILSVERSION = 5.0.3

CURLARCH = curl-${CURLVERSION}.tar.gz
LIBARCHIVEARCH = libarchive-${LIBARCHIVEVERSION}.tar.gz
ZLIBARCH = zlib-${ZLIBVERSION}.tar.gz
BZ2LIBARCH = bzip2-${BZ2LIBVERSION}.tar.gz
XZUTILSARCH = xz-${XZUTILSVERSION}.tar.gz
OPENSSLARCH = openssl-${OPENSSLVERSION}.tar.gz

distclean-local:
	rm -rf libs zlib-${ZLIBVERSION} bzip2-${BZ2LIBVERSION} \
	       curl-${CURLVERSION} libarchive-${LIBARCHIVEVERSION} \
	       xzutils-${XZUTILSVERSION}

libs/include/zlib.h:
	( if [ ! -e ${ZLIBARCH} ] ; then \
	     curl http://zlib.net/${ZLIBARCH} -o ${ZLIBARCH} ; fi )
	rm -rf zlib-${ZLIBVERSION}
	tar xzvf ${ZLIBARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd zlib-${ZLIBVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --static ; \
	  make ; make install )

bz2lib_m32:
	( if [ ! -e ${BZ2LIBARCH} ] ; then \
	     curl http://bzip.org/${BZ2LIBVERSION}/${BZ2LIBARCH} \
	     -o ${BZ2LIBARCH} ; fi )
	rm -rf bzip2-${BZ2LIBVERSION}
	tar xzvf ${BZ2LIBARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd bzip2-${BZ2LIBVERSION} ; \
	  make CFLAGS="-m32 -Wall -Winline -O2 -g -D_FILE_OFFSET_BITS=64" ; \
	  make install PREFIX=$$MYPATH/libs )

libs/include/bzlib.h:
	( if [ ! -e ${BZ2LIBARCH} ] ; then \
	     curl http://bzip.org/${BZ2LIBVERSION}/${BZ2LIBARCH} \
	     -o ${BZ2LIBARCH} ; fi )
	rm -rf bzip2-${BZ2LIBVERSION}
	tar xzvf ${BZ2LIBARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd bzip2-${BZ2LIBVERSION} ; \
	  make ; make install PREFIX=$$MYPATH/libs )

libs/include/curl/curl.h:
	( if [ ! -e ${CURLARCH} ] ; then \
	     curl http://curl.haxx.se/download/${CURLARCH} \
	     -o ${CURLARCH} ; fi )
	rm -rf curl-${CURLVERSION}
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

libs/include/archive.h: libs/include/zlib.h libs/include/bzlib.h libs/include/lzma.h
	( if [ ! -e ${LIBARCHIVEARCH} ] ; then \
	     curl http://libarchive.googlecode.com/files/${LIBARCHIVEARCH} \
	     -o ${LIBARCHIVEARCH} ; fi )
	rm -rf libarchive-${LIBARCHIVEVERSION}
	tar xzvf ${LIBARCHIVEARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd libarchive-${LIBARCHIVEVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared \
	    --without-lzmadec --without-iconv \
	    --with-lzma --without-nettle --without-openssl \
	    --without-xml2 --without-expat --disable-acl \
	    CPPFLAGS="-I$$MYPATH/libs/include" \
	    LDFLAGS="-L$$MYPATH/libs/lib" ; \
	  make ; make install )

libs/include/openssl/ssl.h:
	( if [ ! -e ${OPENSSLARCH} ] ; then \
	    curl http://www.openssl.org/source/${OPENSSLARCH} \
	     -o ${OPENSSLARCH} ; fi )
	rm -rf openssl-${OPENSSLVERSION}
	tar xzvf ${OPENSSLARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd openssl-${OPENSSLVERSION} ; \
	  if test `uname -s` == "Darwin" ; then \
	  ./Configure --prefix=$$MYPATH/libs \
	              --openssldir=$$MYPATH/libs/openssl \
	              no-threads no-shared no-sse2 zlib darwin64-x86_64-cc ; \
	  else \
	  ./config --prefix=$$MYPATH/libs --openssldir=$$MYPATH/libs/openssl \
	  no-threads no-shared no-sse2 zlib ; \
	  fi ; \
	  make ; make install )

libs/include/lzma.h:
	( if [ ! -e ${XZUTILSARCH} ] ; then \
	    curl http://tukaani.org/xz/${XZUTILSARCH} \
	     -o ${XZUTILSARCH} ; fi )
	rm -rf xz-${XZUTILSVERSION}
	tar xzvf ${XZUTILSARCH}
	if [ ! -e libs ] ; then mkdir libs ; fi
	( export MYPATH=`pwd` ; cd xz-${XZUTILSVERSION} ; \
	  ./configure --prefix=$$MYPATH/libs --disable-shared ; make ; \
	  make install )

tolinux64: bob
	strip bob
	cp -a bob /scratch/neunhoef/mywebpage.pub/for/BOB/bob-linux-64bit

tolinux32: bob
	strip bob
	cp -a bob /scratch/neunhoef/mywebpage.pub/for/BOB/bob-linux-32bit

toosx64: bob
	strip bob
	scp bob neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/for/BOB/bob-osx-64bit

toosx32: bob
	strip bob
	scp bob neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/for/BOB/bob-osx-32bit

