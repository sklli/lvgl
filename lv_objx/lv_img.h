/**
 * @file lv_img.h
 * 
 */

#ifndef LV_IMG_H
#define LV_IMG_H

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
#include "misc_conf.h"
#if USE_LV_IMG != 0 && USE_FSINT != 0 && USE_UFS != 0

#if USE_FSINT == 0
#error "lv_img: fsint is required. Enable it in misc_conf.h (USE_FSINT  1) "
#endif

#if USE_UFS == 0
#error "lv_img: ufs is required. Enable it in misc_conf.h (USE_UFS  1) "
#endif

#include "../lv_obj/lv_obj.h"
#include "misc/fs/fsint.h"

#if LV_IMG_ENABLE_SYMBOLS
#include "lv_label.h"
#include "../lv_misc/fonts/symbol_def.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of image*/
typedef struct
{
    /*No inherited ext. because inherited from the base object*/ /*Ext. of ancestor*/
    /*New data for this type */
    char* fn;   /*Image file name. E.g. "U:/my_image"*/
    cord_t w;   /*Width of the image (doubled when upscaled)*/
    cord_t h;   /*Height of the image (doubled when upscaled)*/
    uint8_t auto_size   :1;     /*1: automatically set the object size to the image size*/
    uint8_t upscale     :1;     /*1: upscale to double size*/
    uint8_t transp      :1;     /*Transp. bit in the image header (library handles this)*/
}lv_img_ext_t;

/*Style of image*/
typedef struct
{
	lv_objs_t objs;	/*Style of ancestor*/
	/*New style element for this type */
	opa_t recolor_opa;                /*Intensity of recoloring (OPA_TRANSP, OPA_10 ... OPA_COVER)*/
#if LV_IMG_ENABLE_SYMBOLS != 0
    font_types_t sym_font;            /*Symbol font*/
#endif
}lv_imgs_t;

/*Built-in styles of image*/
typedef enum
{
	LV_IMGS_DEF,
	LV_IMGS_LIGHT,
	LV_IMGS_DARK,
}lv_imgs_builtin_t;


/* Image header it is compatible with
 * the result image converter utility*/
typedef struct
{
    uint16_t w;         /*Width of the image map*/
    uint16_t h;         /*Height of the image map*/
    uint16_t cd;        /*Color depth (8/16 or 24)*/
    uint16_t transp :1; /*1: Do not draw LV_IMG_TRANSP_COLOR pixels*/
}lv_img_raw_header_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**
 * Create an image objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a rectangle object, if not NULL then the new object will be copied from it
 * @return pointer to the created image
 */
lv_obj_t * lv_img_create(lv_obj_t * par, lv_obj_t * copy);

/**
 * Signal function of the image
 * @param img pointer to animage object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 */
bool lv_img_signal(lv_obj_t * img, lv_signal_t sign, void * param);

/**
 * Return with a pointer to built-in style and/or copy it to a variable
 * @param style a style name from lv_imgs_builtin_t enum
 * @param copy copy the style to this variable. (NULL if unused)
 * @return pointer to an lv_imgs_t style
 */
lv_imgs_t * lv_imgs_get(lv_imgs_builtin_t style, lv_imgs_t * copy);

/**
 * Create a file to the RAMFS from a picture data
 * @param fn file name of the new file (e.g. "pic1", will be available at "U:/pic1")
 * @param data pointer to a color map with lv_img_raw_header_t header
 * @return result of the file operation. FS_RES_OK or any error from fs_res_t
 */
fs_res_t lv_img_create_file(const char * fn, const color_int_t * data);

/**
 * Set a file to the image
 * @param img pointer to an image object
 * @param fn file name in the RAMFS to set as picture (e.g. "U:/pic1").
 */
void lv_img_set_file(lv_obj_t * img, const char * fn);

/**
 * Enable the auto size feature.
 * If enabled the object size will be same as the picture size.
 * @param img pointer to an image
 * @param en true: auto size enable, false: auto size disable
 */
void lv_img_set_auto_size(lv_obj_t * img, bool en);

/**
 * Enable the upscaling with LV_DOWNSCALE.
 * @param img pointer to an image
 * @param en true: upscale enable, false: upscale disable
 */
void lv_img_set_upscale(lv_obj_t * img, bool en);

/**
 * Get the auto size enable attribute
 * @param img pointer to an image
 * @return true: auto size is enabled, false: auto size is disabled
 */
bool lv_img_get_auto_size(lv_obj_t * img);


/**
 * Get the upscale enable attribute
 * @param img pointer to an image
 * @return true: upscale is enabled, false: upscale is disabled
 */
bool lv_img_get_upscale(lv_obj_t * img);


lv_imgs_t * lv_imgs_get(lv_imgs_builtin_t style, lv_imgs_t * copy);


/**********************
 *      MACROS
 **********************/

/*Use this macro to declare an image in a c file*/
#define LV_IMG_DECLARE(var_name) extern const color_int_t var_name[];

#endif

#endif
