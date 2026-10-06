#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

static const struct i2c_dt_spec sensor = I2C_DT_SPEC_GET(DT_NODELABEL(maxm86161));

int main(void)
{

    const struct device *console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    if (!device_is_ready(console)) {
        return 0;
    }

    uint32_t dtr = 0;
    while (dtr == 0) {
        int ret = uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr);
        if (ret != 0) {
            break;
        }
        k_msleep(100);
    }
    k_msleep(200);

    printk("\nMAXM86161A communication test\n");
    printk("Bus: %s; device address: 0x%02x\n",
           sensor.bus->name, (unsigned int)sensor.addr);

    if (!i2c_is_ready_dt(&sensor)) {
        printk("FAIL: Nordic I2C controller is not ready\n");
        return 0;
    }

    printk("I2C controller ready; reading PART_ID register 0xFF\n");

    while (true) {
        uint8_t part_id = 0;
        int ret = i2c_reg_read_byte_dt(&sensor, 0xFF, &part_id);

        if (ret != 0) {
            printk("FAIL: read at 0x62 failed; error=%d\n", ret);
        } else if (part_id == 0x36) {
            printk("PASS: I2C read succeeded; PART_ID=0x36 (expected)\n");
        } else {
            printk("MISMATCH: I2C read succeeded; PART_ID=0x%02x; expected 0x36\n",
                   (unsigned int)part_id);
        }
        k_msleep(2000);
    }
}
