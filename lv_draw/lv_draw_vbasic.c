/**
 * @file lv_vdraw.c
 * 
 */
#include "lv_conf.h"
#if LV_VDB_SIZE != 0

#include <stddef.h>
#include "lvgl/lv_obj/lv_vdb.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Fill an area in the Virtual Display Buffer
 * @param cords_p coordinates of the area to fill
 * @param mask_p fill only o this mask
 * @param color fill color
 * @param opa opacity of the area (0..255)
 */
void lv_vfill(const area_t * cords_p, const area_t * mask_p, 
                          color_t color, opa_t opa)
{
    area_t res_a;
    bool union_ok;
    lv_vdb_t * vdb_p = lv_vdb_get();
    
    /*Get the union of cord and mask*/
    /* The mask is already truncated to the vdb size
     * in 'lv_refr_area_with_vdb' function */
    union_ok = area_union(&res_a, cords_p, mask_p);
    
    /*If there are common part of the three area then draw to the vdb*/
    if(union_ok == true) {
        area_t vdb_rel_a;   /*Stores relative coordinates on vdb*/
        vdb_rel_a.x1 = res_a.x1 - vdb_p->vdb_area.x1;
        vdb_rel_a.y1 = res_a.y1 - vdb_p->vdb_area.y1;
        vdb_rel_a.x2 = res_a.x2 - vdb_p->vdb_area.x1;
        vdb_rel_a.y2 = res_a.y2 - vdb_p->vdb_area.y1;
        
        color_t * vdb_buf_tmp = vdb_p->buf;
        uint32_t vdb_width = area_get_width(&vdb_p->vdb_area);
        /*Move the vdb_tmp to the first row*/
        vdb_buf_tmp += vdb_width * vdb_rel_a.y1;
        
        /*Set all row in vdb to the given color*/
        cord_t row;
        uint32_t col;
        
        /*Run simpler function without opacity*/
        if(opa == OPA_COVER) {
            for(row = vdb_rel_a.y1; row <= vdb_rel_a.y2; row++) {
                for(col = vdb_rel_a.x1; col <= vdb_rel_a.x2; col++) {
                    vdb_buf_tmp[col] = color;
                }

                vdb_buf_tmp += vdb_width;
            }            
        }
        /*Calculate the alpha too*/
        else {
            for(row = vdb_rel_a.y1; row <= vdb_rel_a.y2; row++) {
                for(col = vdb_rel_a.x1; col <= vdb_rel_a.x2; col++) {
                    color_t c = color_mix(color, vdb_buf_tmp[col], opa);
                    vdb_buf_tmp[col] = c;
                }
                vdb_buf_tmp += vdb_width;
            }
        }
    }    
}

/**
 * Draw a letter in the Virtual Display Buffer
 * @param pos_p left-top coordinate of the latter
 * @param mask_p the letter will be drawn only on this area
 * @param font_p pointer to font 
 * @param letter a letter to draw
 * @param color color of letter
 * @param opa opacity of letter (0..255)
 */
void lv_vletter(const point_t * pos_p, const area_t * mask_p, 
                     const font_t * font_p, uint8_t letter,
                     color_t color, opa_t opa)
{      
    if(font_p == NULL) return;

    uint8_t letter_w = font_get_width(font_p, letter);
    uint8_t letter_h = font_get_height(font_p);

    const uint8_t * map_p = font_get_bitmap(font_p, letter);

    if(map_p == NULL) return;

    /*If the letter is completely out of mask don't draw it */
    if(pos_p->x + letter_w < mask_p->x1 || pos_p->x > mask_p->x2 ||
       pos_p->y + letter_h < mask_p->y1 || pos_p->y > mask_p->y2) return;

    lv_vdb_t * vdb_p = lv_vdb_get();
    cord_t vdb_width = area_get_width(&vdb_p->vdb_area);
    color_t * vdb_buf_tmp = vdb_p->buf;
    cord_t col, row;
    uint8_t col_bit;
    uint8_t col_byte_cnt;

    cord_t col_start = pos_p->x > mask_p->x1 ? 0 : mask_p->x1 - pos_p->x;
    cord_t col_end  = pos_p->x + letter_w < mask_p->x2 ? letter_w : mask_p->x2 - pos_p->x + 1;
    cord_t row_start = pos_p->y > mask_p->y1 ? 0 : mask_p->y1 - pos_p->y;
    cord_t row_end  =  pos_p->y + letter_h < mask_p->y2 ? letter_h : mask_p->y2 - pos_p->y + 1;

    /*Set a pointer on VDB to the first pixel of the letter*/
    vdb_buf_tmp += ((pos_p->y - vdb_p->vdb_area.y1) * vdb_width)
                    + pos_p->x - vdb_p->vdb_area.x1;

    /*If the letter is partially out of mask the move there on VDB*/
    vdb_buf_tmp += (row_start * vdb_width) + col_start;

    /*Move on the map too*/
    map_p += (row_start * font_p->width_byte) + (col_start>>3);

    for(row = row_start; row < row_end; row ++) {
        col_byte_cnt = 0;
        col_bit = 7 - (col_start % 8);
        for(col = col_start; col < col_end; col ++) {

            if((*map_p & (1 << col_bit)) != 0) {
                if(opa == OPA_COVER) *vdb_buf_tmp = color;
                else *vdb_buf_tmp = color_mix(color, *vdb_buf_tmp, opa);
            }
            vdb_buf_tmp++;

            /*Use a col. more times depending on LV_UPSCALE_FONT*/
           if(col_bit != 0) col_bit --;
           else {
               col_bit = 7;
               col_byte_cnt ++;
               map_p ++;
            }
        }

        map_p += font_p->width_byte - col_byte_cnt;
        vdb_buf_tmp += vdb_width  - (col_end - col_start); /*Next row in VDB*/
    }
}

/**
 * Draw a color map to the display
 * @param cords_p coordinates the color map
 * @param mask_p the map will drawn only on this area
 * @param map_p pointer to a color_t array
 * @param opa opacity of the map (ignored, only for compatibility with lv_vmap)
 * @param transp true: enable transparency of LV_IMG_COLOR_TRANSP color pixels
 * @param upscale true: upscale to double size
 * @param recolor mix the pixels with this color
 * @param recolor_opa the intense of recoloring
 */
void lv_vmap(const area_t * cords_p, const area_t * mask_p, 
             const color_t * map_p, opa_t opa, bool transp, bool upscale,
			 color_t recolor, opa_t recolor_opa)
{
    area_t masked_a;
    bool union_ok;
    lv_vdb_t * vdb_p = lv_vdb_get();

    /*Get the union of map size and mask*/
    /* The mask is already truncated to the vdb size
    * in 'lv_refr_area_with_vdb' function */
    union_ok = area_union(&masked_a, cords_p, mask_p);

    /*If there are common part of the three area then draw to the vdb*/
    if(union_ok == false)  return;

    uint8_t ds_shift = 0;
    if(upscale != false) ds_shift = 1;

    /*If the map starts OUT of the masked area then calc. the first pixel*/
    cord_t map_width = area_get_width(cords_p) >> ds_shift;
    if(cords_p->y1 < masked_a.y1) {
        map_p += (uint32_t) map_width * ((masked_a.y1 - cords_p->y1) >> ds_shift);
    }
    if(cords_p->x1 < masked_a.x1) {
        map_p += (masked_a.x1 - cords_p->x1) >> ds_shift;
    }

    /*Stores coordinates relative to the act vdb*/
    masked_a.x1 = masked_a.x1 - vdb_p->vdb_area.x1;
    masked_a.y1 = masked_a.y1 - vdb_p->vdb_area.y1;
    masked_a.x2 = masked_a.x2 - vdb_p->vdb_area.x1;
    masked_a.y2 = masked_a.y2 - vdb_p->vdb_area.y1;

    cord_t vdb_width = area_get_width(&vdb_p->vdb_area);
    color_t * vdb_buf_tmp = vdb_p->buf;
    vdb_buf_tmp += (uint32_t) vdb_width * masked_a.y1; /*Move to the first row*/

    map_p -= (masked_a.x1 >> ds_shift);

    if(upscale != false) {
        cord_t row;
        cord_t col;
        color_t transp_color = LV_COLOR_TRANSP;
        color_t color_tmp;
        color_t prev_color = COLOR_BLACK;
        cord_t map_col;

        color_tmp = color_mix(recolor, prev_color, recolor_opa);
        for(row = masked_a.y1; row <= masked_a.y2; row++) {
            for(col = masked_a.x1; col <= masked_a.x2; col ++) {
                map_col = col >> 1;

                if(map_p[map_col].full != prev_color.full) {
                    prev_color.full = map_p[map_col].full;
                    color_tmp = color_mix(recolor, prev_color, recolor_opa);
                }
                if(transp == false || map_p[map_col].full != transp_color.full) {
                    vdb_buf_tmp[col] = color_mix( color_tmp, vdb_buf_tmp[col], opa);
                }
            }
            if((row & 0x1) != 0) map_p += map_width; /*Next row on the map*/
            vdb_buf_tmp += vdb_width ;        /*Next row on the VDB*/
        }
    }
    else {
        if(transp == false) { /*Simply copy the pixels to the VDB*/
            cord_t row;

            if(opa == OPA_COVER)  { /*no opa */
                for(row = masked_a.y1; row <= masked_a.y2; row++) {
                    memcpy(&vdb_buf_tmp[masked_a.x1],
                           &map_p[masked_a.x1],
                           area_get_width(&masked_a) * sizeof(color_t));
                    map_p += map_width;               /*Next row on the map*/
                    vdb_buf_tmp += vdb_width;         /*Next row on the VDB*/
                }
            } else {
                cord_t col;
                for(row = masked_a.y1; row <= masked_a.y2; row++) {
                    for(col = masked_a.x1; col <= masked_a.x2; col ++) {
                        vdb_buf_tmp[col] = color_mix( map_p[col], vdb_buf_tmp[col], opa);
                    }
                    map_p += map_width;              /*Next row on the map*/
                    vdb_buf_tmp += vdb_width;        /*Next row on the VDB*/
                }
            }

          /*To recolor draw simply a rectangle above the image*/
#if LV_VDB_SIZE != 0
          lv_vfill(cords_p, mask_p, recolor, recolor_opa);
#endif
        } else { /*transp == true: Check all pixels */
            cord_t row;
            cord_t col;
            color_t transp_color = LV_COLOR_TRANSP;

            if(recolor_opa == OPA_TRANSP) {/*No recolor*/
                if(opa == OPA_COVER)  { /*no opa */
                    for(row = masked_a.y1; row <= masked_a.y2; row++) {
                        for(col = masked_a.x1; col <= masked_a.x2; col ++) {
                            if(map_p[col].full != transp_color.full) {
                                vdb_buf_tmp[col] = map_p[col];
                            }
                        }

                        map_p += map_width; /*Next row on the map*/
                        vdb_buf_tmp += vdb_width;         /*Next row on the VDB*/
                    }
                } else {
                    for(row = masked_a.y1; row <= masked_a.y2; row++) {
                        for(col = masked_a.x1; col <= masked_a.x2; col ++) {
                            if(map_p[col].full != transp_color.full) {
                                vdb_buf_tmp[col] = color_mix( map_p[col], vdb_buf_tmp[col], opa);
                            }
                        }

                        map_p += map_width; /*Next row on the map*/
                        vdb_buf_tmp += vdb_width;         /*Next row on the VDB*/
                    }
                }
            } else { /*Recolor needed*/
                color_t color_tmp;
                if(opa == OPA_COVER)  { /*no opa */
                    for(row = masked_a.y1; row <= masked_a.y2; row++) {
                        for(col = masked_a.x1; col <= masked_a.x2; col ++) {
                            if(map_p[col].full != transp_color.full) {
                                color_tmp = color_mix(recolor, map_p[col], recolor_opa);
                                vdb_buf_tmp[col] = color_tmp;
                            }
                        }

                        map_p += map_width; /*Next row on the map*/
                        vdb_buf_tmp += vdb_width;         /*Next row on the VDB*/
                    }
                } else {
                    for(row = masked_a.y1; row <= masked_a.y2; row++) {
                        for(col = masked_a.x1; col <= masked_a.x2; col ++) {
                            if(map_p[col].full != transp_color.full) {
                                color_tmp = color_mix(recolor, map_p[col], recolor_opa);
                                vdb_buf_tmp[col] = color_mix(color_tmp, vdb_buf_tmp[col], opa);
                            }
                        }

                        map_p += map_width; /*Next row on the map*/
                        vdb_buf_tmp += vdb_width;         /*Next row on the VDB*/
                    }
                }
            }
        }
    }
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
