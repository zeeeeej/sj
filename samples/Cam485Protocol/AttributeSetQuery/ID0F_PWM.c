#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elog.h"
#include "AttributeTable.h"
#include "cm_config.h"
#include "pwm_ctrl.h"

#define TAG_NAME "[ID0F_PWM]"
int attribute_heating_wire_set(const uint8_t* value, uint32_t value_len)
{
    if (value == NULL || value_len != 1) {
        log_e("Invalid parameter for wire duty set");
        return -1;
    }
    uint8_t result = 0;
    log_i("Set wire duty value : %d",value[0]);
    set_pwm_duty_cycle_encapsulated(value[0]);
    return SendSetAttributeResp(0x0F,result);
}
int attribute_heating_wire_get()
{

    uint8_t result = 0;
    log_i("Get pwm duty");
    uint8_t duty_cycle;
    if (Get_PWM_Duty_Cycle(&duty_cycle) != 0) {
        result = 1;
    }
    uint8_t GetValue[1];
    GetValue[0] = duty_cycle;
    uint32_t valueLen = sizeof(GetValue);
    return SendGetAttributeResp(0x0F,result,GetValue,valueLen);
}