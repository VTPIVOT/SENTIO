#include <vector>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/fs/fs.h>

#include <stdio.h>
#include <string.h>

#include "mySensor.hpp"
// #include "compressNStore.hpp"

using namespace std;

/* ========== USER INPUTS ========== */

int sleepTimeMilis = 50;
int totalIterations = 100;

/* ================================= */

int main(void)
{
    k_msleep(2000);

    printk("Houston, lookin good!\n\n");

    k_msleep(500);

    printk("i,ax,ay,az,gx,gy,gz\n");


    /* =========================================
       IMU SETUP
       ========================================= */

    NordBoardInternal boardImu;

    boardImu.readData();

    std::vector<double> data(totalIterations);
<<<<<<< HEAD


    /* =========================================
       FILE SETUP
       ========================================= */

    struct fs_file_t file;

    fs_file_t_init(&file);


    int ret = fs_open(
        &file,
        "/lfs/imu_data.csv",
        FS_O_CREATE | FS_O_WRITE
    );


    if (ret < 0)
    {
        printk("ERROR opening file: %d\n", ret);
        return 0;
    }


    printk("imu_data.csv opened successfully\n");


    /* =========================================
       WRITE CSV HEADER
       ========================================= */

    const char header[] =
        "case,ax,ay,az,gx,gy,gz,label\n";


    ret = fs_write(
        &file,
        header,
        strlen(header)
    );


    if (ret < 0)
    {
        printk("ERROR writing header: %d\n", ret);

        fs_close(&file);

        return 0;
    }


    /* =========================================
       SENSOR LOOP
       ========================================= */

    char line[200];

    int currentIteration = 0;


    while (currentIteration < totalIterations)
    {
        /* Get new IMU measurement */

        boardImu.readData();


        /* Store accel_x for your compression code */

        data[currentIteration] = boardImu.accel_x;


        /* Print to terminal like before */

        printk(
            "%d: %f,%f,%f,%f,%f,%f\n",
            currentIteration,
            (double)boardImu.accel_x,
            (double)boardImu.accel_y,
            (double)boardImu.accel_z,
            (double)boardImu.gyro_x,
            (double)boardImu.gyro_y,
            (double)boardImu.gyro_z
        );


        /* =====================================
           CREATE ONE CSV ROW IN RAM
           ===================================== */

        int len = snprintf(
            line,
            sizeof(line),
            "%d,%f,%f,%f,%f,%f,%f,Walking\n",
            currentIteration,
            (double)boardImu.accel_x,
            (double)boardImu.accel_y,
            (double)boardImu.accel_z,
            (double)boardImu.gyro_x,
            (double)boardImu.gyro_y,
            (double)boardImu.gyro_z
        );


        if (len < 0)
        {
            printk("ERROR creating CSV line\n");
        }
        else
        {
            /* =================================
               WRITE ROW INTO FLASH
               ================================= */

            ret = fs_write(
                &file,
                line,
                len
            );


            if (ret < 0)
            {
                printk(
                    "ERROR writing CSV: %d\n",
                    ret
                );
            }
        }


        /* Sync every 20 samples */

        if ((currentIteration + 1) % 20 == 0)
        {
            ret = fs_sync(&file);

            if (ret < 0)
            {
                printk(
                    "ERROR syncing file: %d\n",
                    ret
                );
            }
        }


        currentIteration++;

        k_msleep(sleepTimeMilis);
    }


    /* =========================================
       FINISH FILE
       ========================================= */

    fs_sync(&file);

    fs_close(&file);

    printk("\nCSV file saved!\n");
    printk("Location: /lfs/imu_data.csv\n\n");
    /* =========================================
   READ CSV BACK AND PRINT IT
   ========================================= */

fs_file_t_init(&file);

ret = fs_open(
    &file,
    "/lfs/imu_data.csv",
    FS_O_READ
);

if (ret < 0)
{
    printk("ERROR opening CSV for reading: %d\n", ret);
    return 0;
}

printk("\n========== CSV CONTENTS ==========\n");

char readBuffer[128];

while (1)
{
    int bytesRead = fs_read(
        &file,
        readBuffer,
        sizeof(readBuffer) - 1
    );

    if (bytesRead < 0)
    {
        printk("ERROR reading CSV: %d\n", bytesRead);
        break;
    }

    if (bytesRead == 0)
    {
        break;
    }

    readBuffer[bytesRead] = '\0';

    printk("%s", readBuffer);
}

printk("\n========== END CSV ==========\n");

fs_close(&file);

    /* =========================================
       YOUR EXISTING COMPRESSION CODE
       ========================================= */

    printk(
        "Bytes before compression: %d\n",
        8 * totalIterations
    );

    int totalBytes =
        compressDataCol(
            compressDataRow(data)
        );

    printk(
        "Bytes after compression: %d\n",
        totalBytes
    );


=======
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
>>>>>>> 198331827c5f7e35e8680fbe476a251eb24c8ab4
    return 0;
}