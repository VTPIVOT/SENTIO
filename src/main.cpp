#include <vector>
#include <fstream>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "mySensor.hpp"
// #include "compressNStore.hpp"

using namespace std;

/* ========== USER INPUTS ========== */
int sleepTimeMilis = 50;
int totalIterations = 100;
/* ================================= */

int main(void)
{   
    int currentIteration = 0;
    k_msleep(2000);

    printk("Houston, lookin good!\n\n"); // zp version of print
    k_msleep(500);

    printk("i, ax,ay,az,gx,gy,gz,\n");
    NordBoardInternal boardImu;
    boardImu.readData();
    std::vector<double> data(totalIterations);
    // uint8_t case_num = 0;

    while (1) {
        currentIteration++;
        boardImu.readData();
        data[currentIteration - 1] = boardImu.accel_x;

        k_msleep(sleepTimeMilis);
        // printk("%d: %f,%f,%f,%f,%f,%f\n",
        //     currentIteration,
        //     (double) (boardImu.accel_x),
        //     (double) (boardImu.accel_y), 
        //     (double) (boardImu.accel_z),
        //     (double) (boardImu.gyro_x), 
        //     (double) (boardImu.gyro_y), 
        //     (double) (boardImu.gyro_z));

        /*
        ofstream file("imu_data.csv");
        file << "case" << "ax" << "," << "ay" << "," << "az" << "," << "gx" << "," << "gy" << "," << "gz" << "label" << "\n"; // #delete this line after first run
        file << "case" << case_num  << "," << boardImu.accel_x << "," << boardImu.accel_y << "," << boardImu.accel_z << "," << boardImu.gyro_x << "," << boardImu.gyro_y << "," << boardImu.gyro_z << "n/a" <<  "\n";
        file.flush();
        */

        if (currentIteration == totalIterations) break;
    }
    /*
    printk("Bytes before compression: %d\n", 8 * totalIterations);
    int totalBytes = compressDataCol(compressDataRow(data));
    printk("Bytes after compression: %d\n", totalBytes);
    */
    return 0;
}