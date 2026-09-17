#include "mySensor.hpp"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

std::vector<MySensor*> MySensor::sensorList;

void NordBoardInternal::readData() {   
    struct sensor_value odr;
        odr.val1 = 104;
        odr.val2 = 0;

    sensor_attr_set(imu, SENSOR_CHAN_ACCEL_XYZ,
                    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);

    sensor_attr_set(imu, SENSOR_CHAN_GYRO_XYZ,
                    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);
    
    sensor_sample_fetch(imu);
    sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, accel);
    sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, gyro);

    setData(
        sensor_value_to_double(&accel[0]),
        sensor_value_to_double(&accel[1]),
        sensor_value_to_double(&accel[2]),
        sensor_value_to_double(&gyro[0]),
        sensor_value_to_double(&gyro[1]),
        sensor_value_to_double(&gyro[2])
    );

    gravRemover();
}
void NordBoardInternal::gravRemover() {
    gravity[0] = a * accel_x + (1-a) * gravity[0];
    gravity[1] = a * accel_y + (1-a) * gravity[1];
    gravity[2] = a * accel_z + (1-a) * gravity[2];

    double newAx = accel_x - gravity[0];
    double newAy = accel_y - gravity[1];
    double newAz = accel_z - gravity[2];

    setData(newAx, newAy, newAz, gyro_x, gyro_y, gyro_z);
}
void I2CDevice::readRawData() {
    const struct device* bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    i2c_burst_read(bus, address, reg, data, numBytes);
}

/*
void PPGSensor::readData() {
    readRawData();
    // idk yet work harder archit
}
*/