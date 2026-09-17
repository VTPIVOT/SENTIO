#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "compressNStore.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>

#define FLASH_NODE DT_NODELABEL(p25q16h)

/* ===== USER INPUTS ===== */
const int NUM_DECIMALS = 3;
const int ZERO_TOL = 0.01;
const int BUFFER_SIZE = 512;
/* =======================*/

std::vector<int16_t> compressDataRow(std::vector<double> data) {
    std::vector<int16_t> newData(data.size());
    for (int i = 0; i < data.size(); i++) {
        if (data[i] <= ZERO_TOL) {newData[i] = 0;}
        newData[i] = (int16_t) (data[i] * pow(10, NUM_DECIMALS));
    }
    return newData;
}

// int16_t[dynamic] zeroRunLengthEncoding(std::vectpr<int> data) {
    
// }

uint16_t zigzag(int16_t num) {
    if (num < 0) return (uint16_t)(-2 * num - 1);
    else return (uint16_t)(2 * num);
}

std::vector<uint16_t> differenceEncoding(std::vector<int16_t> data) {
    // Difference encoding - worth doing?
    std::vector<uint16_t> newData(data.size());
    newData[0] = zigzag(data[0]);
    for (int i = 1; i < BUFFER_SIZE; i++) {
        newData[i] = zigzag(data[i] - data[i-1]);
        // newData[i] = zigzag(data[i]);
    }
    return newData;
}

int compressDataCol(std::vector<int16_t> data) {
    std::vector<uint16_t> encData = differenceEncoding(data);

    int pos = 0;
    uint8_t buffer[BUFFER_SIZE];
    bool cont = false;
    uint16_t d;
    int storedBlocks = 0;

    for (int i = 0; i < encData.size(); i++) {
        d = data[i];
        if ((d >> 7) > 0) cont = true;
        if ((d >= 32768 && pos == BUFFER_SIZE - 2) || (d >= 128 && pos == BUFFER_SIZE - 1)) {
            cont = false;
            pos = BUFFER_SIZE;
        }
        while (cont) {
            buffer[pos] = (d & ((1 << 7) - 1)) + pow(2,7);
            pos++;
            d = d >> 7;
            if (d <= 0) break;
        }
        if (pos == BUFFER_SIZE) {
            // Store data
            storedBlocks++;
            pos = 0;
            memset(buffer, 0, sizeof(buffer));
        }
    }   

    int totalBytes = (storedBlocks * BUFFER_SIZE) + pos;
    return totalBytes;
}

static const struct device *flash_dev = DEVICE_DT_GET(FLASH_NODE);
static uint32_t flash_addr = 0;

void save_data(uint8_t buffer[BUFFER_SIZE])
{
    flash_write(flash_dev, flash_addr, buffer, BUFFER_SIZE);
    flash_addr += BUFFER_SIZE;
}