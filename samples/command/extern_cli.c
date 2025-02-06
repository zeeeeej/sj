#include "MsgDispatherPort.h"
/*485循环缓冲区命令*/
static void print_485_cir_buf_data(int argc, char **argv) {
    print_485_cir_buf_contents();
}

/*打印循环缓冲区中的数据*/
void register_extern_module_commands(void) {
    register_command("cir_buf_data", print_485_cir_buf_data);  // 注册模块命令
}