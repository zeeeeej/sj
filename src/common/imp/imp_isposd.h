/*********************************************************
 * File Name   : imp_isposd.h
 * Author      : EMnian
 * Mail        : Felix.jhnian@ingenic.com
 * Created Time: 2023-06-27 09:17
 ********************************************************/

#ifndef HEADER_ISPOSD_H
#define HEADER_ISPOSD_H

// #define ISP_OSD_WIND
// #define ISP_OSD_TEXT
#define ISP_OSD_PIC

// #define WIND_SUM                 1
// #define TEXT_SUM                 1
#define PIC_SUM                  1
#define HANDLE_SUM               8

#define HANDLE_0                 0
#define HANDLE_1                 1
#define HANDLE_2                 2
#define HANDLE_3                 3
#define HANDLE_4                 4
#define HANDLE_5                 5
#define HANDLE_6                 6
#define HANDLE_7                 7
#define HANDLE_T40_9             9
#define HANDLE_DIS              -1

#define BLACK_ON                 1
#define BLACK_OFF                0
#define TIME_INTERVAL_OFF       -1
#define SHOW_TEXT_NUM           "0"
#define SHOW_TEXT_DATA          "1"


#define OSD_SHOW_ON              1
#define OSD_SHOW_OFF             0

typedef struct _png_param {
  int png_width;           // png 显示的最终 宽
  int png_height;          // png 显示的最终 高
} osd_param_t;

typedef struct _wind_param {
  uint32_t color;          // 画窗颜色 y = color >> 16 & 0xff; u = color >> 8 &0xff; v = color & 0xff
  uint8_t  line_width;     // 窗口边框宽度
  uint8_t  alpha;          // 宽口边框alpha（3bit）
} wind_param;

typedef struct _text_param {
  int type;                // 填充图片格式 IMP_ISP_PIC_ARGB_8888
  int argb_type;           // 填充格式 IMP_ISP_ARGB_TYPE_BGRA
  char *data;              // 填充图片首地址
  uint16_t text_num;       // 字数
  uint16_t text_width;     // 字宽
  uint16_t text_height;    // 字高
} text_param;

typedef struct _pic_param {
  int type;                // 填充图片类型 IMP_ISP_PIC_ARGB_8888
  int argb_type;           // 填充格式 IMP_ISP_ARGB_TYPE_BGRA
  char *data;              // 填充图片首地址
  uint32_t pic_width;      // pic 选择的宽
  uint32_t pic_height;     // pic 选择的高
  uint8_t seat;            // 显示点按宽或高的第几段显示
} pic_param;


typedef struct _iosd_param{
  uint8_t  handle_num;     // osd 区号(范围: 0~7) wind( 0~19 )
  uint8_t  en;             // 填充功能使能
  uint16_t left;           // 填充横向起始点
  uint16_t top;            // 填充纵向起始点
  uint16_t width;          // 显示的宽度
  uint16_t height;         // 显示的高度
  uint8_t index;           // 显示索引
  union{
    wind_param wind_o;
    text_param text_o;
    pic_param pic_o;
  } cfg;
} iosd_param;

typedef struct _osd_param{

#ifdef ISP_OSD_WIND
  iosd_param wind[WIND_SUM];
#endif

#ifdef ISP_OSD_TEXT
  iosd_param text[TEXT_SUM];
#endif

#ifdef ISP_OSD_PIC
  iosd_param pic[PIC_SUM];
#endif
  int handle_0[HANDLE_SUM];
} isposd_param;

/*************  显示  ***************
  handle_num >= 0： 表示osd 区显示编号
  time > 0：osd 显示时间；
  time = -1：osd 一直显示，直到sample_isposd_show( handle_num, time, "-1"),停止显示
  pngnum 对于 wind：pngnum != “NULL” 显示，pngnum == “-1”，停止显示
  pngnum 对于 text：pngnum != “NULL” 显示data；pngnum == "NULL"，显示数字；pngnum == “-1”，停止显示
  pngnum 对于 png：pngnum != “NULL” 显示png 图；pngnum == “NULL”，显示data；pngnum == “-1”，停止显示
  handle_num = -1 时，打开隐私盖功能，使用如下：
  打开隐私盖：sample_isposd_show( -1, time, "1");
  关闭隐私盖：sample_isposd_show( -1, time, "0");
 *************************************/
int sample_isposd_show(int handle_num, int time, char *pngnum);

/* 设置osd 区域属性 */
void sample_isposd_setattr(isposd_param *info);

/* 获取osd 区属性 */
void sample_isposd_getattr(isposd_param *info);

/* 初始化 */
int sample_isposd_init(void);

/* 反初始化 */
int sample_isposd_deinit(void);

#endif /* HEADER_NAME_H */
