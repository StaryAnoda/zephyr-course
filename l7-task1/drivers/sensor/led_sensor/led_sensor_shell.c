#include "led_sensor.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/shell/shell.h>

const struct device *led_sensor_dev = DEVICE_DT_GET(DT_NODELABEL(led_sensor0));

static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv) {
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    int rc = 0;

    if (!device_is_ready(led_sensor_dev)) {
        shell_error(sh, "LED sensor device not ready");
        return -ENODEV;
    }

    rc = sensor_sample_fetch(led_sensor_dev);
    if (rc) {
        shell_error(sh, "sensor_sample_fetch failed: %d", rc);
        return rc;
    }

    shell_print(sh, "sample_fetch OK – LED is OFF");
    return 0;
}

static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv) {
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    int rc = 0;
    struct sensor_value val;

    if (!device_is_ready(led_sensor_dev)) {
        shell_error(sh, "LED sensor device not ready");
        return -ENODEV;
    }

    rc = sensor_channel_get(led_sensor_dev, SENSOR_CHAN_ALL, &val);
    if (rc) {
        shell_error(sh, "sensor_channel_get failed: %d", rc);
        return rc;
    }

    shell_print(sh, "channel_get OK – LED is ON");
    shell_print(sh, "value: %d.%06d", val.val1, val.val2);
    return 0;
}

static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv) {
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "Device name  : %s", led_sensor_dev->name);
    shell_print(sh, "Device ready : %s",
                device_is_ready(led_sensor_dev) ? "yes" : "NO");
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    led_sensor_subcmd,
    SHELL_CMD_ARG(fetch, NULL, "Turn LED ON  (calls sensor_sample_fetch)",
                  cmd_sensor_fetch, 1, 0),
    SHELL_CMD_ARG(
        read, NULL,
        "Turn LED OFF and print last value (calls sensor_channel_get)",
        cmd_sensor_read, 1, 0),
    SHELL_CMD_ARG(info, NULL, "Print device name and ready state",
                  cmd_sensor_info, 1, 0),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_ARG_REGISTER(sensor, &led_sensor_subcmd,
                       "LED sensor commands (fetch | read | info )", NULL, 1,
                       0);
