#pragma once

#include <vector>
#include <cstdint>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

class MySensor {
protected:
    static std::vector<MySensor*> sensorList;

public:
    MySensor() {
        sensorList.push_back(this);
    }
};

class NordBoardInternal : public MySensor {
public:
    double accel_x;
    double accel_y;
    double accel_z;

    double gyro_x;
    double gyro_y;
    double gyro_z;

    // For gravity filtering
    double a = 0.1;
    double gravity[3] = {0, 0, 0}; 

    NordBoardInternal() : MySensor(),
        accel_x(1), accel_y(0), accel_z(0),
        gyro_x(0), gyro_y(0), gyro_z(0) {
            imu = DEVICE_DT_GET_ONE(st_lsm6dsl);
        }

    void setData(double ax, double ay, double az,
                 double gx, double gy, double gz) {
        accel_x = ax;
        accel_y = ay;
        accel_z = az;
        gyro_x = gx;
        gyro_y = gy;
        gyro_z = gz;
    }


    void readData();
    void gravRemover();
private:
    const struct device* imu;
    struct sensor_value accel[3];
    struct sensor_value gyro[3];    
};

class I2CDevice : public MySensor {
protected:
    int address;
    uint8_t reg;
    uint8_t numBytes;
    uint8_t* data;

public:
    I2CDevice(int addr, uint8_t regAddr, uint8_t bytesToRead)
        : MySensor(), address(addr), reg(regAddr), numBytes(bytesToRead) {
        data = new uint8_t[numBytes];
    }

    ~I2CDevice() {
        delete[] data;
    }

    void readRawData();
};

class PPGSensor : public I2CDevice {
public:
    uint32_t sample1;
    uint32_t sample2;

    PPGSensor(int addr, uint8_t regAddr, uint8_t bytesToRead)
        : I2CDevice(addr, regAddr, bytesToRead), sample1(0), sample2(0) {
    }

    // void readData();
};