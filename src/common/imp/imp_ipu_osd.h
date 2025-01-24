/*********************************************************
 * File Name   : imp_ipu_osd.h
 * Author      : EMnian
 * Mail        : Felix.jhnian@ingenic.com
 * Created Time: 2023-06-27 09:28
 ********************************************************/

#ifndef _IMP_IPU_OSD_H
#define _IMP_IPU_OSD_H

#define COVER_NUM  0
#define PIC_NUM    1
#define BITMAP_NUM  0

typedef struct _rgn_attr{

  uint8_t mode;  /*0:使用默认属性，1:使用手动填充属性*/
  uint8_t type;
  uint8_t format;
  union{
    void *data;  /* for update data */
    uint32_t color;
  } data;
} rgn_attr_t;

typedef struct _ipu_rgn_param {

  uint8_t en;     // 填充功能使能
  uint8_t group;  //osd group index
  uint8_t handle; //region句柄

  uint16_t x0;    // 左上角x坐标
  uint16_t y0;    // 左上角y坐标
  uint16_t x1;    // 右下角x坐标
  uint16_t y1;    // 右下角y坐标

   rgn_attr_t cfg;
} ipu_rgn_param_t;

typedef struct _ipu_osd_param {

  uint8_t group; //osd group index
  uint16_t handle_num; //一共创建几个region/handle

  ipu_rgn_param_t cover[COVER_NUM];
  ipu_rgn_param_t pic[PIC_NUM];
  ipu_rgn_param_t bitmap[BITMAP_NUM];

} ipu_osd_param_t;

/*
 * @func: sample_ipu_osd_getparam
 * @brief: get ipu osd param pointer
 * @param:
 *   param:   ipu osd param pointer

 * @return:
 *   null
 */
void sample_ipu_osd_getparam(ipu_osd_param_t **param);

/*
 * @func: sample_ipu_osd_setrgnsattr
 * @brief: set ipu osd region attr
 * @param:
 *   param:   ipu osd param (set all region attr at once)

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_setrgnsattr(ipu_osd_param_t *param);

/*
 * @func: sample_ipu_osd_updatergndata_all
 * @brief: update region data (update all region of same type at once)
 * @param:
 *   rgn_num:   region num of target type
 *   rgn_param:   region param of target type

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_updatergndata_all(int rgn_num, ipu_rgn_param_t *rgn_param);

/*
 * @func: sample_ipu_osd_updatergndata_single
 * @brief: update region data (update one region at once)
 * @param:
 *   handle:   region handle
 *   type:     region type
 *   data:     region data

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_updatergndata_single(int handle, int type, void *data);

/*
 * @func: sample_ipu_osd_show_all
 * @brief: show regions (show all region of same type at once)
 * @param:
 *   rgn_num:   region num of target type
 *   rgn_param:   region param of target type

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_show_all(int rgn_num, ipu_rgn_param_t *);

/*
 * @func: sample_ipu_osd_show_single
 * @brief: show region (show one region at once)
 * @param:
 *   handle:   region handle
 *   type:     region type
 *   enable:     1:show;0:close

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_show_single(int handle, int group, int enable);

/*
 * @func: sample_ipu_osd_show_single_timeout
 * @brief: show region (show one region at once)
 * @param:
 *   handle:   region handle
 *   type:     region type
 *   data:     region data
 *   timeout:  show region until timeout

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_show_single_timeout(int handle, int group, uint64_t timeout);

/*
 * @func: sample_ipu_osd_init
 * @brief: init ipu osd (bind and creat regions)
 * @param:
 *   group:     group index
 *   fscell:     framesource cell
 *   eccell:     encoder cell

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_init(int group, IMPCell *fscell, IMPCell *eccell);

/*
 * @func: sample_ipu_osd_exit
 * @brief: exit ipu osd (unbind and destroy regions)
 * @param:
 *   fscell:     framesource cell
 *   eccell:     encoder cell

 * @return:
 *   -1 error, 0 succeed
 */
int sample_ipu_osd_exit(IMPCell *fscell, IMPCell *eccell);

#endif
