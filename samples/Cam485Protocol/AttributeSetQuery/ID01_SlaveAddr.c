#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include "MsgDispatcher.h"
#include "AttributeTable.h"
#include "Cam485ProtocolCommon.h"
int attribute_slave_address_set(const uint8_t* value, uint32_t value_len)
{
    if (value == NULL || value_len != 1) {
        log_e("Invalid parameter for slave address set");
        return -1;
    }
    uint8_t result = 0;
    log_i("Set slave address");
    if (Set_g_slave_address(value[0]) != 0) {
        result = 1;
    }
    else
    {
        Set_Protocol_Slave_Address(value[0]);
    }
    return SendSetAttributeResp(0x01,result);
}
int attribute_slave_address_get()
{
    uint8_t result = 0;
    log_i("Get slave address");
    uint8_t slave_address = 0;
    if (Get_Protocol_Slave_Address(&slave_address) != 0) {
        result = 1;
    }
    uint8_t GetValue[1];
    GetValue[0] = slave_address;
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x01,result,GetValue,valueLen);
}
