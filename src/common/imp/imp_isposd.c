/*********************************************************
 * File Name   : imp_isposd.c
 * Author      : EMnian
 * Mail        : Felix.jhnian@ingenic.com
 * Created Time: 2023-06-28 11:41
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <imp/imp_log.h>
#include <imp-common.h>
#include <global_config.h>
#include "imp_isposd.h"
#include <bgramapinfo.h>

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <png.h>

#define MODULE_TAG             "imp_isposd"
#define PNG_DATA_FIEL          "/tmp/ARGB"
#define PNG_TMP_DATA           "/tmp/tmp.png"
#define PNG_BYTES_TO_CHECK     4


#define OSD_WIND_HANDLE        0
#define OSD_TEXT_HANDLE        1
#define OSD_PIC_HANDLE         2

#define png_ratio_width        (200.0)
#define png_ratio_height       (200.0)

#define max_width              3840
#define max_height             2160

//#define DBUG

static volatile unsigned char osd_black_value = 0; /* osd show black value */

#ifdef ISP_OSD_PIC
static char * buff_ARGB; /* store data png->argb */
static osd_param_t isp_osd_info; /* store pic width,height data */
#endif

isposd_param isposd_info = {
#ifdef ISP_OSD_WIND
  .wind[0] = {
    .handle_num = HANDLE_2,
    .en = 1,
    .left = 0,
    .top = 0,
    .width = 50,
    .height = 50,
    .index = 0,
    .cfg.wind_o.color = 0xff0000,
    .cfg.wind_o.line_width = 3,
    .cfg.wind_o.alpha = 4,
  },
#endif

#ifdef ISP_OSD_TEXT
  .text[0] = {
    .handle_num = HANDLE_1,
    .en = 1,
    .left = 200,
    .top = 200,
    .width = 0,
    .height = 0,
    .index = 0,
    .cfg.text_o.type = IMP_ISP_PIC_ARGB_8888,
    .cfg.text_o.argb_type = IMP_ISP_ARGB_TYPE_BGRA,
    .cfg.text_o.data = NULL,
    .cfg.text_o.text_num = 20,
    .cfg.text_o.text_width = 16,
    .cfg.text_o.text_height = 34,
  },
#endif

#ifdef ISP_OSD_PIC
  .pic[0] = {
    .handle_num = HANDLE_0,
    .en = 1,
    .left = 200,
    .top = 200,
    .width = 0,
    .height = 0,
    .index = 0,
    .cfg.pic_o.type = IMP_ISP_PIC_ARGB_8888,
    .cfg.pic_o.argb_type = IMP_ISP_ARGB_TYPE_BGRA,
    .cfg.pic_o.data = NULL,
    .cfg.pic_o.pic_width = 0,
    .cfg.pic_o.pic_height = 0,
    .cfg.pic_o.seat = 1,
  },
#endif
};

/*
function: privacy cover switch
param: on_off
on ： 1  open the privacy cover
off： 0  close the privacy cover
return: 0(success) !0(failed)
note：the outflow cannot be adjusted first "off"
*/
static int black_switch(int on_off)
{
  int ret = 0;
  unsigned char bright_black = 0;
  ret = IMP_ISP_Tuning_GetBrightness(&bright_black);
  if ( ret ) {
    printf("ERROR[%s] ** error osd black(%d) **\n", MODULE_TAG, __LINE__);
    return ret;
  }

  if ( on_off == OSD_SHOW_ON ) {
    osd_black_value = bright_black;
    bright_black = 0;
  } else {
    bright_black = osd_black_value;
  }

  ret = IMP_ISP_Tuning_SetBrightness(bright_black);
  if ( ret ) {
    printf("ERROR[%s] ** error osd black(%d) **\n", MODULE_TAG, __LINE__);
    return ret;
  }
  return 0;
}

/*
   osd 图与pc 图像宽高等比例计算（图像切换分辨率，osd 图也等比例显示）
srcwidth:设定的等比例宽（例如：2560）
srcheight:设定的等比例高（例如：1440）
picwidth:pc 输出的宽
picheight:pc 输出的高
need_Width:osd 输出的宽
need_Height:osd 输出的高
*/
#ifdef ISP_OSD_PIC
static void Stretch_pic(int srcWidth, int srcHeight, int picWidth, int picHeight, int *need_Width, int *need_Height)
{
  *need_Width  = (int)( (float)picWidth / (float)srcWidth * png_ratio_width );//这个是确定在显示器中的显示大小
  *need_Height = (int)( (float)picHeight / (float)srcHeight * png_ratio_height );

  if ( *need_Width % 2 ) {
    *need_Width = *need_Width + 1;
  }

  if ( *need_Height % 2 ) {
    *need_Height = *need_Height + 1;
  }

  if ( *need_Width >= *need_Height ) {
    *need_Height = *need_Width;
  } else {
    *need_Width = *need_Height;
  }
}

/*
   将png 图像转ARGB 图像
   返回值：0（成功） !0(失败)
   */
static int load_png_image(const char *filepath)
{
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep* row_pointers;
  char buf[PNG_BYTES_TO_CHECK];
  int w, h, x = 0, y, temp, color_type;
  int  fd = 0;
  unsigned char * buf_f = (unsigned char *)malloc(4);

  fp = fopen(filepath, "rb");
  if ( fp == NULL ) {
    printf("ERROR[%s] ** open filepath error ** \n", MODULE_TAG);
    return -1;
  }
  /*
     读取png 文件大小
     */
  fd = open(PNG_DATA_FIEL, O_RDWR|O_CREAT|O_TRUNC, 0777);

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  info_ptr = png_create_info_struct(png_ptr);

  setjmp( png_jmpbuf( png_ptr ) );

  temp = fread(buf, 1, PNG_BYTES_TO_CHECK, fp);

  if ( temp < PNG_BYTES_TO_CHECK ) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return -1;
  }

  temp = png_sig_cmp( (png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK );

  if ( temp != 0 ) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return -1;
  }

  rewind(fp);
  png_init_io(png_ptr, fp);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
  color_type = png_get_color_type(png_ptr, info_ptr);
  w = png_get_image_width(png_ptr, info_ptr);
  h = png_get_image_height(png_ptr, info_ptr);
  row_pointers = png_get_rows(png_ptr, info_ptr);
  switch ( color_type ) {
  case PNG_COLOR_TYPE_RGB_ALPHA:
    for ( y = 0; y < h; ++y ) {
      for (x = 0; x < w * 4; x++) {
        buf_f[2] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // red
        buf_f[1] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // green
        buf_f[0] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // blue
        buf_f[3] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x]; // alpha
        /*
           将转换的argb 数据写到
           ARGB 文件里
           */
        if ( write(fd, buf_f, 4) != 4 )
          printf("INFO[%s] ## png->arbg write error %d##\n", MODULE_TAG, __LINE__);
      }
    }
    close(fd);
    break;
  case PNG_COLOR_TYPE_RGB:
    for ( y = 0; y < h; ++y ) {
      for ( x = 0; x < w * 3; ) {
      }
    }
    break;
  default:
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return 0;
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  fclose(fp);
  return 0;
}

/*
   将图像缩放成对应比例
   str_dir：需要处理的png 图像路径
strout : 处理后生成的图像 （/tmp/tmp.png)
nDestWidth:调整的宽
nDestHeight :调整的高
*/
static void resize_png(char const *str_dir,int nDestWidth, int nDestHeight)
{
  char const *str = NULL;
  char const *strout = PNG_TMP_DATA;

  int iw = 0, ih = 0, n = 0;
  int ow = 0, oh = 0;
  unsigned char *odata = NULL;
  unsigned char *idata = NULL;

  str = (char const *)str_dir;

  /*
     加载png 图像,
     获取图像的信息：
     图像的宽：iw
     图像的高：ih
     图像的颜色通道：n
     */
  idata = stbi_load(str, &iw, &ih, &n, 0);
  /*
     输出的图像宽高
     */
  ow = nDestWidth;
  oh = nDestHeight;
  odata = malloc(ow * oh * n);
  /*
     处理png 图像（进行缩放）
     */
  stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0,
         STBIR_TYPE_UINT8, n, 0, 0,
         STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
         STBIR_FILTER_BOX, STBIR_FILTER_BOX,
         STBIR_COLORSPACE_SRGB, NULL
        );
  /*
     将缩放后的 png 图像放入 strout中
     */
  stbi_write_png(strout, ow, oh, n, odata, 0);

  stbi_image_free(idata);
  stbi_image_free(odata);
}

/*
   ARGB数据储存到osd_buff
file:png 图像路径
pc_width:pc输出的宽
pc_height:pc输出的高
返回值：argb 数据的指针
*/
static char *set_argb_buff(char const *file, int pc_width,int pc_height)
{
  FILE *fp = NULL;
  char *osd_buff = NULL;
  int size = 0;

  /*
     调整 png 图像的大小
     */
  Stretch_pic(max_width, max_height, pc_width, pc_height,
        &isp_osd_info.png_width,
        &isp_osd_info.png_height
       );

  resize_png(file, isp_osd_info.png_width,
       isp_osd_info.png_height);

  /*
     png->argb
     */
  if ( load_png_image(PNG_TMP_DATA) < 0 ) {
    printf("ERROR[%s] ** error load_png_image fail **\n", MODULE_TAG);
    return NULL;
  }

  fp = fopen(PNG_DATA_FIEL, "rb");
  if ( NULL == fp ) {
    printf("ERROR[%s] ** error open %s fail **\n", MODULE_TAG, PNG_DATA_FIEL);
    return NULL;
  }
  fseek(fp , 0, SEEK_END);
  size = ftell(fp);

  if ( buff_ARGB != NULL ) {
    free(buff_ARGB);
    buff_ARGB = NULL;
    printf("INFO[%s] --> free argb buff success <--\n", MODULE_TAG);
  }

  osd_buff = malloc(size);
  rewind(fp);

  fread(osd_buff, 1, size, fp);
  if ( ferror(fp) ) {
    printf("INFO[%s] ## read argb data fail ##\n", MODULE_TAG);
  }
  fclose(fp);

  return osd_buff;

}

static int isp_osd_png_on(int index, char *pngnum)
{
  int ret = 0;
  IMPIspOsdAttrAsm stISPOSDAsm;

  if ( strcmp("NULL", pngnum) != 0 ) {
    char png_frame[30];
    /* png 图像路径 */
    sprintf(png_frame, "/system/etc/png/%s", pngnum);

    if ( access( png_frame, F_OK ) < 0 ) {
      printf("ERROR[%s] ** error png frame path %s **\n", MODULE_TAG, png_frame);
      return -1;
    }

    buff_ARGB = set_argb_buff(png_frame,
            isposd_info.pic[index].cfg.pic_o.pic_width,
            isposd_info.pic[index].cfg.pic_o.pic_height);
    if ( buff_ARGB == NULL ) {
      printf("ERROR[%s] ** buff argb NULL **\n", MODULE_TAG);
      return -1;
    }
  }

  stISPOSDAsm.type = ISP_OSD_REG_PIC;
  stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_type = isposd_info.pic[index].cfg.pic_o.type;
  stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_argb_type = isposd_info.pic[index].cfg.pic_o.argb_type;
  stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_DISABLE;
  stISPOSDAsm.stsinglepicAttr.pic.pinum =  isposd_info.pic[index].handle_num;
  stISPOSDAsm.stsinglepicAttr.pic.osd_enable = isposd_info.pic[index].en;

  if ( strcmp("NULL", pngnum) != 0 ) {
    stISPOSDAsm.stsinglepicAttr.pic.osd_left = ( isposd_info.pic[index].cfg.pic_o.pic_width - isposd_info.pic[index].left - isp_osd_info.png_width) / isposd_info.pic[index].cfg.pic_o.seat;
    stISPOSDAsm.stsinglepicAttr.pic.osd_top = ( isposd_info.pic[index].cfg.pic_o.pic_height - isposd_info.pic[index].top ) / isposd_info.pic[index].cfg.pic_o.seat;
    stISPOSDAsm.stsinglepicAttr.pic.osd_width = isp_osd_info.png_width;
    stISPOSDAsm.stsinglepicAttr.pic.osd_height = isp_osd_info.png_height;
    stISPOSDAsm.stsinglepicAttr.pic.osd_image = buff_ARGB;
    stISPOSDAsm.stsinglepicAttr.pic.osd_stride = isp_osd_info.png_width * 4;
  } else {
    stISPOSDAsm.stsinglepicAttr.pic.osd_left = ( isposd_info.pic[index].cfg.pic_o.pic_width - isposd_info.pic[index].left ) / isposd_info.pic[index].cfg.pic_o.seat;
    stISPOSDAsm.stsinglepicAttr.pic.osd_top = ( isposd_info.pic[index].cfg.pic_o.pic_height - isposd_info.pic[index].top ) / isposd_info.pic[index].cfg.pic_o.seat;
    stISPOSDAsm.stsinglepicAttr.pic.osd_width = isposd_info.pic[index].width;
    stISPOSDAsm.stsinglepicAttr.pic.osd_height = isposd_info.pic[index].height;
    stISPOSDAsm.stsinglepicAttr.pic.osd_image = isposd_info.pic[index].cfg.pic_o.data;
    stISPOSDAsm.stsinglepicAttr.pic.osd_stride = isposd_info.pic[index].width * 4;
  }

  ret = IMP_ISP_Tuning_SetOsdRgnAttr(CH0_INDEX, isposd_info.pic[index].handle_num, &stISPOSDAsm);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** png set osd rgn attr failed **\n", MODULE_TAG);
    return ret;
  }

  ret = IMP_ISP_Tuning_ShowOsdRgn(CH0_INDEX, isposd_info.pic[index].handle_num, OSD_SHOW_ON);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** png show on failed **\n", MODULE_TAG);
    return ret;
  }

  return ret;
}

static int isp_osd_png_off(int handle_num)
{
  int ret = 0;

  ret = IMP_ISP_Tuning_ShowOsdRgn(CH0_INDEX, handle_num, OSD_SHOW_OFF);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** png show off failed **\n", MODULE_TAG);
    return -1;
  }

  return ret;
}
#endif

#ifdef ISP_OSD_TEXT
static int isp_osd_time_on(int index , char *type)
{
  int ret = 0;

  // type "NULL":数字  type 不为NULL:文字

  /* generate time */
  char DateStr[40];
  time_t currTime;
  struct tm *currDate;
  unsigned i = 0, j = 0;
  void *dateData = NULL;
  uint32_t *data_t = malloc(isposd_info.text[index].cfg.text_o.text_num * isposd_info.text[index].cfg.text_o.text_width * isposd_info.text[index].cfg.text_o.text_height * sizeof(uint32_t));


  int penpos_t = 0;
  int fontadv = 0;

  time(&currTime);
  currDate = localtime(&currTime);
  memset (DateStr, 0, 40);
  strftime(DateStr, 40, "%Y-%m-%d %I:%M:%S", currDate);
  for ( i = 0; i < isposd_info.text[index].cfg.text_o.text_num; i++ ) {
    switch ( DateStr[i] ) {
    case '0' ... '9':
      dateData = (void *)gBgramap[DateStr[i] - '0'].pdata;
      fontadv = gBgramap[DateStr[i] - '0'].width;
      penpos_t += gBgramap[DateStr[i] - '0'].width;
      break;
    case '-':
      dateData = (void *)gBgramap[10].pdata;
      fontadv = gBgramap[10].width;
      penpos_t += gBgramap[10].width;
      break;
    case ' ':
      dateData = (void *)gBgramap[11].pdata;
      fontadv = gBgramap[11].width;
      penpos_t += gBgramap[11].width;
      break;
    case ':':
      dateData = (void *)gBgramap[12].pdata;
      fontadv = gBgramap[12].width;
      penpos_t += gBgramap[12].width;
      break;
    default:
      break;
    }
    for ( j = 0; j < OSD_REGION_HEIGHT; j++ ) {
      memcpy((void *)((uint32_t *)data_t + j * isposd_info.text[index].cfg.text_o.text_num * isposd_info.text[index].cfg.text_o.text_width + penpos_t),
             (void *)((uint32_t *)dateData + j*fontadv), fontadv*sizeof(uint32_t));

    }
  }

  IMPIspOsdAttrAsm stISPOSDAsm;
  stISPOSDAsm.type = ISP_OSD_REG_PIC;
  stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_type = isposd_info.text[index].cfg.text_o.type;
  stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_argb_type = isposd_info.text[index].cfg.text_o.argb_type;
  stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_DISABLE;
  stISPOSDAsm.stsinglepicAttr.pic.pinum =  isposd_info.text[index].handle_num;
  stISPOSDAsm.stsinglepicAttr.pic.osd_enable = isposd_info.text[index].en;
  stISPOSDAsm.stsinglepicAttr.pic.osd_left = isposd_info.text[index].left;
  stISPOSDAsm.stsinglepicAttr.pic.osd_top = isposd_info.text[index].top;
  stISPOSDAsm.stsinglepicAttr.pic.osd_width = isposd_info.text[index].cfg.text_o.text_num * isposd_info.text[index].cfg.text_o.text_width;
  stISPOSDAsm.stsinglepicAttr.pic.osd_height = isposd_info.text[index].cfg.text_o.text_height;

  if ( strcmp("NULL", type) == 0 ) {
    stISPOSDAsm.stsinglepicAttr.pic.osd_image = (char*)data_t;
  } else {
    stISPOSDAsm.stsinglepicAttr.pic.osd_image = isposd_info.text[index].cfg.text_o.data;
  }

  stISPOSDAsm.stsinglepicAttr.pic.osd_stride = isposd_info.text[index].cfg.text_o.text_num * isposd_info.text[index].cfg.text_o.text_width * 4;

  ret = IMP_ISP_Tuning_SetOsdRgnAttr(CH0_INDEX, isposd_info.text[index].handle_num, &stISPOSDAsm);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** time set osd rgn attr failed **\n", MODULE_TAG);
    return ret;
  }

  ret = IMP_ISP_Tuning_ShowOsdRgn(CH0_INDEX, isposd_info.text[index].handle_num, OSD_SHOW_ON);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** time show on failed **\n", MODULE_TAG);
    return ret;
  }

  return ret;
}

static int isp_osd_time_off(int handle_num)
{
  int ret = 0;

  ret = IMP_ISP_Tuning_ShowOsdRgn(CH0_INDEX, handle_num, OSD_SHOW_OFF);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** time show off failed **\n", MODULE_TAG);
    return -1;
  }

  return ret;
}
#endif

#ifdef ISP_OSD_WIND
static int isp_osd_wind_on(int index)
{
  int ret = 0;

  IMPOSDRgnAttr rIspOsdAttr;
  rIspOsdAttr.type = OSD_REG_ISP_LINE_RECT;
  rIspOsdAttr.osdispdraw.stDrawAttr.pinum = isposd_info.wind[index].handle_num;
  rIspOsdAttr.osdispdraw.stDrawAttr.type = IMP_ISP_DRAW_WIND;
  rIspOsdAttr.osdispdraw.stDrawAttr.color_type = IMPISP_MASK_TYPE_YUV;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.enable = isposd_info.wind[index].en;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.left = isposd_info.wind[index].left;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.top = isposd_info.wind[index].top;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.width = isposd_info.wind[index].width;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.height = isposd_info.wind[index].height;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.y_value = ( isposd_info.wind[index].cfg.wind_o.color >> 16 ) & 0xff;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.u_value = ( isposd_info.wind[index].cfg.wind_o.color >> 8 ) & 0xff;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.v_value = isposd_info.wind[index].cfg.wind_o.color & 0xff;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.line_width = isposd_info.wind[index].cfg.wind_o.line_width;
  rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.alpha = isposd_info.wind[index].cfg.wind_o.alpha; /*范围为[0,4]*/

  ret = IMP_OSD_SetRgnAttr_ISP(&rIspOsdAttr, 0);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** wind show on failed **\n", MODULE_TAG);
    return ret;
  }

  return ret;
}
#endif

static int isp_osd_deinit(int handle_num)
{
  int ret = 0;

  ret = IMP_ISP_Tuning_ShowOsdRgn(CH0_INDEX, handle_num, OSD_SHOW_OFF);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** isposd deinit show failed **\n", MODULE_TAG);
    return ret;
  }
  ret = IMP_ISP_Tuning_DestroyOsdRgn(CH0_INDEX, handle_num);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** isposd deinit dest failed **\n", MODULE_TAG);
    return ret;
  }

  return ret;

}

static int select_show(int handle_num, char *pngnum)
{
  int ret = 0;
  int i = 0;
  int type = 0;
  int index = 0;

  if ( handle_num < 0 ) {
    if ( strcmp("1" , pngnum) == 0 ) {
      ret = black_switch(BLACK_ON);
      if ( ret < 0 ) {
        printf("ERROR[%s] ** show black on failed **\n", MODULE_TAG);
        return ret;
      }
    } else if ( strcmp("0", pngnum) == 0 ) {
      ret = black_switch(BLACK_OFF);
      if ( ret < 0 ) {
        printf("ERROR[%s] ** show black off failed **\n", MODULE_TAG);
        return ret;
      }
    } else {
      printf("ERROR[%s] ** show black failed **\n", MODULE_TAG);
      return ret;
    }
    return ret;
  }

#ifdef ISP_OSD_WIND
  for ( i = 0; i < WIND_SUM; i++ ) {
    if ( handle_num == isposd_info.wind[i].handle_num ) {
      type = OSD_WIND_HANDLE;
      //osd 类型为WIND，第几个WIND
      index = isposd_info.wind[i].index;
      break;
    }
  }
#endif

#ifdef ISP_OSD_TEXT
  for ( i = 0; i < TEXT_SUM; i++ ) {
    if ( handle_num == isposd_info.text[i].handle_num ) {
      type = OSD_TEXT_HANDLE;
      index = isposd_info.text[i].index;
      break;
    }
  }
#endif

#ifdef ISP_OSD_PIC
  for ( i = 0; i < PIC_SUM; i++ ) {
    if ( handle_num == isposd_info.pic[i].handle_num ) {
      type = OSD_PIC_HANDLE;
      index = isposd_info.pic[i].index;
      break;
    }
  }
#endif

  //printf("INFO[%s] --> handle_num %d type %d index %d <--\n", MODULE_TAG, handle_num, type, index);

  switch ( type ) {
#ifdef ISP_OSD_WIND
  case OSD_WIND_HANDLE:
    if ( !strcmp("-1", pngnum) ) {
      // wind 不支持动态开关
      printf("INFO[%s] --> isp osd wind off <--\n", MODULE_TAG);
    } else {
      ret = isp_osd_wind_on(index);
#ifdef DBUG
      printf("INFO[%s] --> wind ret %d <--\n", MODULE_TAG, ret);
#endif
    }
    break;
#endif

#ifdef ISP_OSD_TEXT
  case OSD_TEXT_HANDLE:
    if ( !strcmp("-1", pngnum) ) {
      ret = isp_osd_time_off(handle_num);
    } else {
      ret = isp_osd_time_on(index , pngnum);
#ifdef DBUG
      printf("INFO[%s] --> text ret %d <--\n", MODULE_TAG, ret);
#endif
    }
    break;
#endif

#ifdef ISP_OSD_PIC
  case OSD_PIC_HANDLE:
    if ( !strcmp("-1", pngnum) ) {
      ret = isp_osd_png_off(handle_num);
    } else {
      ret = isp_osd_png_on(index, pngnum);
#ifdef DBUG
      printf("INFO[%s] --> png ret %d <--\n", MODULE_TAG, ret);
#endif
    }
    break;
#endif
  default :
    printf("ERROR[%s] ** error show on handle_num(%d) %d **\n", MODULE_TAG, handle_num, __LINE__);
    ret = -1;
    break;
  }
  return ret;
}

void sample_isposd_setattr(isposd_param *p)
{
  memcpy(&isposd_info, p, sizeof(isposd_info));

}

void sample_isposd_getattr(isposd_param *p)
{
  memcpy(p, &isposd_info, sizeof(isposd_info));
}

int sample_isposd_show(int handle_num, int time, char *pngnum)
{
  int ret = 0;

  ret = select_show(handle_num, pngnum);
  if ( ret < 0 ) {
    printf("ERROR[%s] ** error show on handle_num(%d) %d **\n", MODULE_TAG, handle_num, __LINE__);
    return ret;
  }

  if ( time > 0 ) {
    usleep(time * 1000);
    ret = select_show(handle_num, "-1");
    if ( ret < 0 ) {
      printf("ERROR[%s] ** error show off handle_num(%d) %d **\n", MODULE_TAG, handle_num, __LINE__);
      return ret;
    }
  }

  return ret;
}

int sample_isposd_init( void )
{
  int ret = 0;
  int i = 0;

  for ( i = 0; i < HANDLE_SUM; i++ ) 
  {
    isposd_info.handle_0[i] = IMP_ISP_Tuning_CreateOsdRgn(CH0_INDEX, NULL);
    #ifdef DBUG
        printf("INFO[%s] --> ispinit handle num %d <--\n", MODULE_TAG, isposd_info.handle_0[i]);
    #endif
    ret = isposd_info.handle_0[i];
    if ( ret < 0 ) {
      printf("ERROR[%s] ** isposd created handle %d failed **\n", MODULE_TAG, isposd_info.handle_0[i]);
      return ret;
    }
  }
  return ret;
}

int sample_isposd_deinit( void )
{
  int ret = 0;
  int num = 0;
  int i = 0;

#ifdef ISP_OSD_PIC
  for ( i = 0; i < PIC_SUM; i++ ) {
    free(buff_ARGB);
    free(isposd_info.pic[i].cfg.pic_o.data);
    isposd_info.pic[i].cfg.pic_o.data = NULL;
    buff_ARGB = NULL;
    ret = isp_osd_deinit(isposd_info.pic[i].handle_num);
    if ( ret < 0 ) {
      printf("ERROR[%s] ** isposd deinit pic handle %d failed **\n", MODULE_TAG, isposd_info.pic[i].handle_num);
      return ret;
    }
    num++;
  }
#endif

#ifdef ISP_OSD_TEXT
  for ( i = 0; i < TEXT_SUM; i++ ) {
    free(isposd_info.text[i].cfg.text_o.data);
    isposd_info.text[i].cfg.text_o.data = NULL;
    ret = isp_osd_deinit(isposd_info.text[i].handle_num);
    if ( ret < 0 ) {
      printf("ERROR[%s] ** isposd deinit text handle %d failed **\n", MODULE_TAG, isposd_info.text[i].handle_num);
      return ret;
    }
    num++;
  }
#endif

#ifdef ISP_OSD_WIND
  for ( i = 0; i < WIND_SUM; i++ ) {
    ret = isp_osd_deinit(isposd_info.wind[i].handle_num);
    if ( ret < 0 ) {
      printf("ERROR[%s] ** isposd deinit wind handle %d failed **\n", MODULE_TAG, isposd_info.wind[i].handle_num);
      return ret;
    }
    num++;
  }
#endif

  if ( num != HANDLE_SUM ) {
    for( i = HANDLE_SUM - 1; i >= num; i-- ) {
      ret = isp_osd_deinit(i);
#ifdef DBUG
      printf("INFO[%s] --> isposd deinit handle num %d <--\n", MODULE_TAG, i);
#endif
    }
  }

  return ret;
}
