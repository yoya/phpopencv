dnl $Id$
dnl config.m4 for extension opencv

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(opencv, for opencv support,
dnl Make sure that the comment is aligned:
dnl [  --with-opencv             Include opencv support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(opencv, whether to enable opencv support,
Make sure that the comment is aligned:
[  --enable-opencv           Enable opencv support])

if test "$PHP_OPENCV" != "no"; then
  dnl Write more examples of tests here...

  # --with-opencv -> check with-path
   SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/opencv/cv.h"  # you most likely want to change this
  if test -r $PHP_OPENCV/$SEARCH_FOR; then # path given as parameter
    OPENCV_DIR=$PHP_OPENCV
  else # search default path list
    AC_MSG_CHECKING([for opencv files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        OPENCV_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$OPENCV_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the opencv distribution])
  fi

  # --with-opencv -> add include path
  PHP_ADD_INCLUDE($OPENCV_DIR/include/opencv)

  # --with-opencv -> check for lib and symbol presence
  LIBNAME=cxcore # you may want to change this
  LIBSYMBOL=cvLoadImage # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH(cxcore, $OPENCV_DIR/lib, OPENCV_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(cv, $OPENCV_DIR/lib, OPENCV_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(highgui, $OPENCV_DIR/lib, OPENCV_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(cvaux, $OPENCV_DIR/lib, OPENCV_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(ml, $OPENCV_DIR/lib, OPENCV_SHARED_LIBADD)
    AC_DEFINE(HAVE_OPENCVLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong opencv lib version or lib not found])
  ],[
    -L$OPENCV_DIR/lib -lcxcore -lcv -lhighgui -lcvaux -lml
  ])
  
  PHP_SUBST(OPENCV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(opencv, php_opencv.c, $ext_shared)
fi
