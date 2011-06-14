/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cv.h>//OpenCVのヘッダ
#include <highgui.h>//OpenCVのヘッダ

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_opencv.h"

typedef IplImage *IplImagePtr; // for ZEND_FETCH_RESOURCE

static void php_free_opencv_image(zend_rsrc_list_entry *rsrc TSRMLS_DC);

ZEND_BEGIN_ARG_INFO_EX(arginfo_cvLoadImage, 0, 0, 2)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cvFlip, 0, 0, 1)
    ZEND_ARG_INFO(0, src)
    ZEND_ARG_INFO(0, dest)
    ZEND_ARG_INFO(0, flip_mode)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(arginfo_cvCvtColor, 0, 0, 3)
    ZEND_ARG_INFO(0, src)
    ZEND_ARG_INFO(0, dest)
    ZEND_ARG_INFO(0, code)
    ZEND_ARG_INFO(0, dstCn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cvReleaseImage, 0, 0, 1)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

/* If you declare any globals in php_opencv.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(opencv)
*/

/* True global resources - no need for thread safety here */
static int le_iplimage;

/* {{{ opencv_functions[]
 *
 * Every user visible function must have an entry in opencv_functions[].
 */
const zend_function_entry opencv_functions[] = {
	PHP_FE(cvLoadImage,	arginfo_cvLoadImage)
    PHP_FE(cvFlip,	arginfo_cvFlip)
    PHP_FE(cvCvtColor,	arginfo_cvCvtColor)
    PHP_FE(cvReleaseImage,	arginfo_cvReleaseImage)
	{NULL, NULL, NULL}	/* Must be the last line in opencv_functions[] */
};
/* }}} */

/* {{{ opencv_module_entry
 */
zend_module_entry opencv_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"opencv",
	opencv_functions,
	PHP_MINIT(opencv),
	PHP_MSHUTDOWN(opencv),
	PHP_RINIT(opencv),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(opencv),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(opencv),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_OPENCV
ZEND_GET_MODULE(opencv)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("opencv.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_opencv_globals, opencv_globals)
    STD_PHP_INI_ENTRY("opencv.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_opencv_globals, opencv_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_opencv_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_opencv_init_globals(zend_opencv_globals *opencv_globals)
{
	opencv_globals->global_value = 0;
	opencv_globals->global_string = NULL;
}
*/
/* }}} */

#define MY_REGISTER_LONG_CONSTANT(X,Y)  REGISTER_LONG_CONSTANT(X, Y, CONST_CS | CONST_PERSISTENT)

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(opencv)
{
    le_iplimage = zend_register_list_destructors_ex(php_free_opencv_image, NULL, "IplImage", module_number);
    
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
    MY_REGISTER_LONG_CONSTANT("CV_LOAD_IMAGE_COLOR", CV_LOAD_IMAGE_COLOR);
    MY_REGISTER_LONG_CONSTANT("CV_BGR2RGB", CV_BGR2RGB);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(opencv)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(opencv)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(opencv)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(opencv)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "opencv support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ cvLoadImage(string filename, int flags)
 */
PHP_FUNCTION(cvLoadImage)
{
    char *filename;
    long filename_len;
    long flags;
    char *buff;
    IplImage *img;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &filename, &filename_len, &flags) == FAILURE) {
        return ;
    }
    buff = malloc(filename_len + 1);
    memcpy(buff, filename, filename_len);
    buff[filename_len] = '\0';
    img = cvLoadImage(buff, flags);
    free(buff);
    if (img == NULL) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "cvLoadImage failed");
        RETURN_FALSE;
    }
    ZEND_REGISTER_RESOURCE(return_value, img, le_iplimage);
}
/* }}} */

/* {{{ cvFlip(IplImage *src, IpImage *dest = null, int flip_mode = 0)
 */
PHP_FUNCTION(cvFlip)
{
    zval *src_z, *dest_z;
    IplImage *src_img = NULL, *dest_img = NULL;
    int flip_mode = 0;
    switch (ZEND_NUM_ARGS()) {
    case 1:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &src_z) == FAILURE)  {
            return;
        }
        ZEND_FETCH_RESOURCE(src_img, IplImagePtr, &src_z, -1, "IplImage", le_iplimage)
        break;
    case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &src_z, &dest_z) == FAILURE)  {
            return;
        }
        ZEND_FETCH_RESOURCE(src_img, IplImagePtr, &src_z, -1, "IplImage", le_iplimage);
        ZEND_FETCH_RESOURCE(dest_img, IplImagePtr, &dest_z, -1, "IplImage", le_iplimage);
        break;
    case 3:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrl", &src_z, &dest_z, &flip_mode) == FAILURE)  {
            return;
        }
        ZEND_FETCH_RESOURCE(src_img, IplImagePtr, &src_z, -1, "IplImage", le_iplimage);
        if (src_z == dest_z) {
            dest_img = src_img;
        } else {
            ZEND_FETCH_RESOURCE(dest_img, IplImagePtr, &dest_z, -1, "IplImage", le_iplimage);
        }
        break;
    default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE;
    }
    cvFlip(src_img, dest_img, flip_mode);
}

/* }}} */

/* {{{ cvCvtColor(string filename, int flags)
 */
PHP_FUNCTION(cvCvtColor)
{
    ;
}

/* }}} */

/* {{{ cvReleaseImage(IplImage img)
 */
PHP_FUNCTION(cvReleaseImage)
{
    ;
}

/* }}} */

/* {{{ php_free_gd_image
 */
static void php_free_opencv_image(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    cvReleaseImage((IplImage**) &rsrc->ptr);
}


/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
