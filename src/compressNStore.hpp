#pragma once

std::vector<int16_t> compressDataRow(std::vector<double> data);
uint16_t zigzag(int16_t num);
std::vector<uint16_t> differenceEncoding(std::vector<int16_t> data);
int compressDataCol(std::vector<int16_t> data);
void storeData();

