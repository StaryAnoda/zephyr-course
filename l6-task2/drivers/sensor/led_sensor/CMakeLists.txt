if (CONFIG_LED_SENSOR)
    zephyr_library_sources(led_sensor.c)
    zephyr_include_directories(.)
endif ()
