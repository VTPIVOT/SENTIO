#include <Wire.h>
#include "easy_i2c.h"

#define MAX86161_ADDR       0x62

#define REG_INT_STATUS1     0x00
#define REG_INT_STATUS2     0x01
#define REG_INT_ENABLE1     0x02
#define REG_INT_ENABLE2     0x03
#define REG_FIFO_WR_PTR     0x04
#define REG_FIFO_RD_PTR     0x05
#define REG_OVF_COUNTER     0x06
#define REG_FIFO_DATA_CTR   0x07
#define REG_FIFO_DATA       0x08
#define REG_FIFO_CONFIG1    0x09
#define REG_FIFO_CONFIG2    0x0A
#define REG_SYS_CTRL        0x0D
#define REG_PPG_SYNC_CTRL   0x10
#define REG_PPG_CONFIG1     0x11
#define REG_PPG_CONFIG2     0x12
#define REG_PPG_CONFIG3     0x13
#define REG_PROX_INT_THRESH 0x14
#define REG_PHOTO_DIODE_SEL 0x15
#define REG_PICKET_FENCE    0x16
#define REG_LED_SEQ1        0x20
#define REG_LED_SEQ2        0x21
#define REG_LED_SEQ3        0x22
#define REG_LED1_PA         0x23
#define REG_LED2_PA         0x24
#define REG_LED3_PA         0x25
#define REG_PILOT_PA        0x29
#define REG_PART_ID         0xFF
#define REG_REV_ID          0xFE

#define SLOT_OFF            0x00
#define SLOT_LED1           0x01  // Green
#define SLOT_LED2           0x02  // IR
#define SLOT_LED3           0x03  // Red

#define SLOTS_PER_SAMPLE    4
#define BYTES_PER_SLOT      3
#define BYTES_PER_SAMPLE    (SLOTS_PER_SAMPLE * BYTES_PER_SLOT) // 12
#define MAX_SAMPLES         128
#define FIFO_BUF_SIZE       (MAX_SAMPLES * BYTES_PER_SAMPLE)    // 1536

int32_t led1A[MAX_SAMPLES]; // Slot 1 — Green, PD1
int32_t led1B[MAX_SAMPLES]; // Slot 2 — Green, PD2
int32_t led2A[MAX_SAMPLES]; // Slot 3 — IR,    PD1
int32_t led2B[MAX_SAMPLES]; // Slot 4 — IR,    PD2

// Write a single byte to a register
inline void WriteReg(uint8_t reg, uint8_t value) {
  easy_i2c::Write(Wire, MAX86161_ADDR, reg, value);
}

// Read a single byte from a register
inline uint8_t ReadReg(uint8_t reg) {
  return easy_i2c::Read<uint8_t>(Wire, MAX86161_ADDR, reg);
}

// Burst-read 'length' bytes starting from 'reg' into buf[]
void ReadBurst(uint8_t reg, uint8_t *buf, uint16_t length) {
  easy_i2c::Read(Wire, MAX86161_ADDR, reg, buf, (size_t)length);
}

// Dedicated FIFO reader — register auto-increments on MAX86161
void ReadFifo(uint8_t *buf, uint16_t length) {
  ReadBurst(REG_FIFO_DATA, buf, length);
}

// Returns clamped number of unread FIFO samples
uint8_t GetSampleCount() {
  uint8_t cnt = ReadReg(REG_FIFO_DATA_CTR);
  return (cnt > MAX_SAMPLES) ? (uint8_t)MAX_SAMPLES : cnt;
}

void device_data_read(void) {
  uint8_t sampleCnt;
  uint8_t regVal;

  static uint8_t dataBuf[FIFO_BUF_SIZE];

  // Read FIFO sample count from REG_FIFO_DATA_CTR (0x07)
  // easy_i2c template: Read<uint8_t> returns the register value directly
  sampleCnt = easy_i2c::Read<uint8_t>(Wire, MAX86161_ADDR, REG_FIFO_DATA_CTR);
  if (sampleCnt == 0) return;
  if (sampleCnt > MAX_SAMPLES) sampleCnt = MAX_SAMPLES;

  // Burst-read all raw bytes from FIFO using easy_i2c buffer overload:
  // easy_i2c::Read(Wire, addr, reg, buf, len)
  easy_i2c::Read(Wire, MAX86161_ADDR, REG_FIFO_DATA,
                 dataBuf, (size_t)((uint16_t)sampleCnt * BYTES_PER_SAMPLE));

  // Unpack — identical bit-shifts to your pseudocode
  // Each slot = 3 bytes MSB-first, 19-bit ADC value, mask 0x7FFFF
  for (uint8_t i = 0; i < sampleCnt; i++) {
    uint16_t base = (uint16_t)i * BYTES_PER_SAMPLE; // i * 12

    // Slot 1 — LED1 (Green), PD1
    led1A[i] = (((int32_t)dataBuf[base +  0] << 16) |
                ((int32_t)dataBuf[base +  1] <<  8) |
                 (int32_t)dataBuf[base +  2]) & 0x7FFFF;

    // Slot 2 — LED1 (Green), PD2
    led1B[i] = (((int32_t)dataBuf[base +  3] << 16) |
                ((int32_t)dataBuf[base +  4] <<  8) |
                 (int32_t)dataBuf[base +  5]) & 0x7FFFF;

    // Slot 3 — LED2 (IR), PD1
    led2A[i] = (((int32_t)dataBuf[base +  6] << 16) |
                ((int32_t)dataBuf[base +  7] <<  8) |
                 (int32_t)dataBuf[base +  8]) & 0x7FFFF;

    // Slot 4 — LED2 (IR), PD2
    led2B[i] = (((int32_t)dataBuf[base +  9] << 16) |
                ((int32_t)dataBuf[base + 10] <<  8) |
                 (int32_t)dataBuf[base + 11]) & 0x7FFFF;
  }

  // Print results
  Serial.print(F("Samples: "));
  Serial.println(sampleCnt);

  for (uint8_t i = 0; i < sampleCnt; i++) {
    Serial.print(F("["));   Serial.print(i);        Serial.print(F("] "));
    Serial.print(F("G-PD1:")); Serial.print(led1A[i]); Serial.print(F("  "));
    Serial.print(F("G-PD2:")); Serial.print(led1B[i]); Serial.print(F("  "));
    Serial.print(F("IR-PD1:")); Serial.print(led2A[i]); Serial.print(F("  "));
    Serial.print(F("IR-PD2:")); Serial.println(led2B[i]);
  }
}

bool initMAX86161() {
  // Verify part ID using easy_i2c typed read
  uint8_t partID = easy_i2c::Read<uint8_t>(Wire, MAX86161_ADDR, REG_PART_ID);
  Serial.print(F("MAX86161 Part ID: 0x"));
  Serial.println(partID, HEX); // expect 0x36

  if (partID != 0x36) {
    Serial.println(F("ERROR: MAX86161 not found! Check wiring."));
    return false;
  }

  // Soft reset
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_SYS_CTRL, (uint8_t)0x01);
  delay(100);

  // Clear FIFO pointers — easy_i2c single-byte writes
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_FIFO_WR_PTR,  (uint8_t)0x00);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_FIFO_RD_PTR,  (uint8_t)0x00);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_OVF_COUNTER,  (uint8_t)0x00);

  // FIFO config
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_FIFO_CONFIG1, (uint8_t)0x7F);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_FIFO_CONFIG2, (uint8_t)0x12);

  // PPG config
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_PPG_CONFIG1,  (uint8_t)0x08);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_PPG_CONFIG2,  (uint8_t)0x12);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_PPG_CONFIG3,  (uint8_t)0x40);

  // Photodiode selection: odd slots → PD1, even slots → PD2
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_PHOTO_DIODE_SEL, (uint8_t)0x09);

  // LED sequence: slots 1&2 = Green (LED1), slots 3&4 = IR (LED2)
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_LED_SEQ1,
                  (uint8_t)((SLOT_LED1 << 4) | SLOT_LED1)); // 0x11
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_LED_SEQ2,
                  (uint8_t)((SLOT_LED2 << 4) | SLOT_LED2)); // 0x22
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_LED_SEQ3,
                  (uint8_t)((SLOT_OFF  << 4) | SLOT_OFF));  // 0x00

  // LED pulse amplitudes (~10 mA)
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_LED1_PA, (uint8_t)0x32);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_LED2_PA, (uint8_t)0x32);
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_LED3_PA, (uint8_t)0x00);

  // Enable A_FULL and PPG_RDY interrupts
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_INT_ENABLE1, (uint8_t)0x81);

  // Start sensor (clear SHDN)
  easy_i2c::Write(Wire, MAX86161_ADDR, REG_SYS_CTRL, (uint8_t)0x00);

  Serial.println(F("MAX86161 initialised OK."));
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();
  Wire.setClock(400000);

  if (!initMAX86161()) {
    while (1);
  }
}

void loop() {
  // Poll INT_STATUS1 — bit7=A_FULL, bit6=PPG_RDY
  uint8_t intStatus = easy_i2c::Read<uint8_t>(Wire, MAX86161_ADDR, REG_INT_STATUS1);

  if (intStatus & 0xC0) {
    device_data_read();
  }

  delay(10);
}