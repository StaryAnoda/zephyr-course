#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static int our_board_init(void)
{
	printk("Board Initialized\n");
	return 0;
}

SYS_INIT(our_board_init, APPLICATION, 0);
