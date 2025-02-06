#include "self_check.h"
#include "cam_self_check.h"
#include "gyro_self_check.h"
/*camera自检属性*/
static SelfCheckProperty CamcheckProperty;

/*自检完成标志位*/
static int self_check_flag = 0;
/*陀螺仪自检属性*/
static SelfCheckProperty GyrocheckProperty;

void self_check_start(void)
{
    cam_self_check_start(&CamcheckProperty);

    // gyro_self_check_start(&GyrocheckProperty);

    self_check_flag = 1;
}


// 获取相机自检属性
SelfCheckProperty* self_check_get_cam_check_property(void)
{
    return &CamcheckProperty;
}

// 获取陀螺仪自检属性
SelfCheckProperty* self_check_get_gyro_check_property(void)
{
    return &GyrocheckProperty;
}

// 获取自检状态
int self_check_get_status(void)
{
    return self_check_flag;
}