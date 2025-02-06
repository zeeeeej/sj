CONFIG_FILE="/system/etc/cm_config.ini"

get_config_value() {
    section=$1
    key=$2

    awk -F '=' -v section="[$section]" -v key="$key" '
    $0 == section { in_section=1; next }
    in_section && $1 ~ key { print $2; exit }
    $0 ~ /^\[.*\]/ { in_section=0 }
    ' "$CONFIG_FILE"
}

# 获取配置值
GPIO_PIN=$(get_config_value "PWM_Config" "GPIO_PIN")
DUTY_CYCLE=$(get_config_value "PWM_Config" "DUTY_CYCLE")
FREQUENCY=$(get_config_value "PWM_Config" "FREQUENCY")

echo "GPIO_PIN: $GPIO_PIN"
echo "DUTY_CYCLE: $DUTY_CYCLE"
echo "FREQUENCY: $FREQUENCY"


GPIO_PIN=$(echo "$GPIO_PIN" | tr -d '[:space:]')
DUTY_CYCLE=$(echo "$DUTY_CYCLE" | tr -d '[:space:]')
FREQUENCY=$(echo "$FREQUENCY" | tr -d '[:space:]')

# 周期计算（微秒）
PERIOD_US=$((1000000 / FREQUENCY))
HIGH_TIME=$((PERIOD_US * DUTY_CYCLE / 100))
LOW_TIME=$((PERIOD_US - HIGH_TIME))

# 导出 GPIO 并配置为输出
if [ ! -d "/sys/class/gpio/gpio$GPIO_PIN" ]; then
    echo "$GPIO_PIN" > /sys/class/gpio/export
    echo "out" > /sys/class/gpio/gpio$GPIO_PIN/direction
fi

# 模拟 PWM 信号
echo "启动 PWM，GPIO_PIN=$GPIO_PIN，DUTY_CYCLE=$DUTY_CYCLE%，FREQUENCY=$FREQUENCY Hz"
while true; do
    echo 1 > /sys/class/gpio/gpio$GPIO_PIN/value
    usleep $HIGH_TIME
    echo 0 > /sys/class/gpio/gpio$GPIO_PIN/value
    usleep $LOW_TIME
done