#include "led_sensor.h"
#include <stdbool.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#define DT_DRV_COMPAT custom_led_sensor

LOG_MODULE_REGISTER(led_sensor, LOG_LEVEL_INF);

struct led_sensor_config {
    struct gpio_dt_spec led_gpio;
};

static const struct led_sensor_config led_sensor_cfg_0 = {
    .led_gpio = GPIO_DT_SPEC_INST_GET(0, led_gpios),
};

struct led_sensor_data {
    bool led_state;
};

static struct led_sensor_data led_sensor_data_0 = {
    .led_state = true,
};

int led_sensor_toggle(const struct device *dev) {
    const struct led_sensor_config *config = dev->config;
    struct led_sensor_data *data = dev->data;

    data->led_state = !data->led_state;
    int ret = gpio_pin_set_dt(&config->led_gpio, data->led_state);
    if (ret) {
        LOG_ERR("Fail to write to GPIO %d", config->led_gpio.pin);
        return ret;
    }

    LOG_INF("led sensor toggle: LED turned %s", data->led_state ? "ON" : "OFF");
    return 0;
}

static int channel_get_my_impl(const struct device *dev,
                               enum sensor_channel chan,
                               struct sensor_value *val) {
    const struct led_sensor_config *config = dev->config;
    struct led_sensor_data *data = dev->data;
    int ret = 0;
    val->val1 = 0;
    val->val2 = 0;

    data->led_state = true;
    ret = gpio_pin_set_dt(&config->led_gpio, data->led_state);
    if (ret) {
        LOG_ERR("Fail to write to GPIO %d", config->led_gpio.pin);
        return ret;
    }

    LOG_INF("channel_get: LED turned OFF");

    return 0;
}

static int sample_fetch_my_impl(const struct device *dev,
                                enum sensor_channel chan) {
    const struct led_sensor_config *config = dev->config;
    struct led_sensor_data *data = dev->data;
    int ret = 0;

    data->led_state = false;
    ret = gpio_pin_set_dt(&config->led_gpio, data->led_state);
    if (ret) {
        LOG_ERR("Fail to write to GPIO %d", config->led_gpio.pin);
        return ret;
    }

    LOG_INF("sample_fetch: LED turned ON");

    return 0;
}

static int init(const struct device *dev) {
    const struct led_sensor_config *config = dev->config;
    struct led_sensor_data *data = dev->data;
    int ret = 0;

    if (!gpio_is_ready_dt(&config->led_gpio)) {
        LOG_ERR("GPIO device %s is not ready", config->led_gpio.port->name);
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&config->led_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure LED GPIO: %d", ret);
        return ret;
    }

    ret = gpio_pin_set_dt(&config->led_gpio, data->led_state);
    if (ret) {
        LOG_ERR("Fail to write to GPIO %d", config->led_gpio.pin);
        return ret;
    }

    LOG_INF("LED sensor initialised (pin %d)", config->led_gpio.pin);

    return 0;
}

static DEVICE_API(sensor, led_sensor_api) = {
    .channel_get = channel_get_my_impl,
    .sample_fetch = sample_fetch_my_impl,
};

DEVICE_DT_INST_DEFINE(0, init, NULL, &led_sensor_data_0, &led_sensor_cfg_0,
                      POST_KERNEL, 90, &led_sensor_api);
