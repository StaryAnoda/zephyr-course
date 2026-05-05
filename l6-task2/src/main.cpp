#include <led_sensor.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void) {
    const struct device *led_sensor = DEVICE_DT_GET(DT_NODELABEL(led_sensor0));
    struct sensor_value val = {0};
    int ret = 0;

    if (!device_is_ready(led_sensor)) {
        LOG_ERR_DEVICE_NOT_READY(led_sensor);
        return -ENODEV;
    }

    while (1) {
        ret = sensor_sample_fetch(led_sensor);
        if (ret) {
            LOG_ERR("Failed to fetch sensor sample [%d]", ret);
            k_oops();
        }
        k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);

        ret = sensor_channel_get(led_sensor, SENSOR_CHAN_ALL, &val);
        if (ret) {
            LOG_ERR("Failed to get sensor sample [%d]", ret);
            k_oops();
        }
        k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);

        ret = led_sensor_toggle(led_sensor);
        if (ret) {
            LOG_ERR("Failed to toggle sensor sample [%d]", ret);
            k_oops();
        }
        k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);
    }
    return 0;
}
