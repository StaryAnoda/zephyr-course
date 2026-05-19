#include "led_sensor.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static void blink_work_handler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(blink_work, blink_work_handler);

const struct device *led_sensor_dev = DEVICE_DT_GET(DT_NODELABEL(led_sensor0));

static struct {
    int remaining;
    int duration_ms;
} blink_ctx;

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

static void blink_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (blink_ctx.remaining <= 0) {
        return; /* sequence finished                                    */
    }

    led_sensor_toggle(led_sensor_dev);
    blink_ctx.remaining--;

    if (blink_ctx.remaining > 0) {
        k_work_schedule(&blink_work, K_MSEC(blink_ctx.duration_ms));
    }
}

static int cmd_sensor_blink(const struct shell *sh, size_t argc, char **argv) {
    char *endptr;

    if (!device_is_ready(led_sensor_dev)) {
        shell_error(sh, "LED sensor device not ready");
        return -ENODEV;
    }

    long count = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || count < 1 || count > 1000) {
        shell_error(sh, "Invalid count '%s'. Expected integer in [1..1000].",
                    argv[1]);
        return -EINVAL;
    }

    long duration_ms = 250;
    if (argc >= 3) {
        duration_ms = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0' || duration_ms < 10 || duration_ms > 5000) {
            shell_error(
                sh, "Invalid duration_ms '%s'. Expected integer in [10..5000].",
                argv[2]);
            return -EINVAL;
        }
    }

    k_work_cancel_delayable(&blink_work);
    blink_ctx.remaining = (int)count * 2;
    blink_ctx.duration_ms = (int)duration_ms;
    k_work_schedule(&blink_work, K_NO_WAIT);

    shell_print(sh, "Blinking: count=%ld, half-period=%ld ms (non-blocking)",
                count, duration_ms);
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
    SHELL_CMD_ARG(
        blink, NULL,
        "Blink the LED.  Usage: sensor blink <count> [duration_ms]\n"
        "  count       : number of blinks      [1..1000]\n"
        "  duration_ms : ON/OFF time per blink  [10..5000], default 250",
        cmd_sensor_blink, 2, 1),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_ARG_REGISTER(sensor, &led_sensor_subcmd,
                       "LED sensor commands (fetch | read | info )", NULL, 1,
                       0);
