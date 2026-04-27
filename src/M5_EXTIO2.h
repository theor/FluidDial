/*!
 * @brief An IO extended From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 *
 * @Links [EXT.IO2](https://docs.m5stack.com/en/unit/extio2)
 * @version  V1.0.2
 * @date  2022-07-07
 */
#ifndef _M5_EXTIO2_H_
#define _M5_EXTIO2_H_

// v3 supports reading/writing all digital outputs/inputs at once, while v1 and v2 only support reading/writing them one by one. V3 can also be flashed over i2c
#define EXTIO_FIRMWARE 1

#include <Arduino.h>
#include <Wire.h>
#include "pins_arduino.h"

static constexpr uint8_t EXTIO2_DEFAULT_ADDR =         0x45;
static constexpr uint8_t EXTIO2_MODE_REG =             0x00;
static constexpr uint8_t EXTIO2_OUTPUT_CTL_REG =       0x10;
// v3 only
static constexpr uint8_t EXTIO2_OUTPUTS_CTL_REG =      0x18;
static constexpr uint8_t EXTIO2_DIGITAL_INPUT_REG =    0x20;
// v3 only
static constexpr uint8_t EXTIO2_DIGITAL_INPUTS_REG =   0x28;
static constexpr uint8_t EXTIO2_ANALOG_INPUT_8B_REG =  0x30;
static constexpr uint8_t EXTIO2_ANALOG_INPUT_12B_REG = 0x40;
static constexpr uint8_t EXTIO2_SERVO_ANGLE_8B_REG =   0x50;
static constexpr uint8_t EXTIO2_SERVO_PULSE_16B_REG =  0x60;
static constexpr uint8_t EXTIO2_RGB_24B_REG =          0x70;
static constexpr uint8_t EXTIO2_PWM_DUTY_CYCLE_REG =   0x90;
static constexpr uint8_t EXTIO2_PWM_FREQUENCY_REG =    0xA0;

// v3 only
static constexpr uint8_t EXTIO2_FLASH_REG =    0xFD;
static constexpr uint8_t EXTIO2_FW_VERSION_REG = 0xFE;
static constexpr uint8_t EXTIO2_ADDRESS_REG =    0xFF;

typedef enum {
    DIGITAL_INPUT_MODE = 0,
    DIGITAL_OUTPUT_MODE,
    ADC_INPUT_MODE,
    SERVO_CTL_MODE,
    RGB_LED_MODE
} extio_io_mode_t;

typedef enum { _8bit = 0, _12bit } extio_anolog_read_mode_t;

enum class ErrorCode {
    Ok = 0,
    BeginWire = 1,
    Start = 2,
    Stop = 3,
    Timeout = 5,
    Uninit = 255,
};
class M5_EXTIO2 {
   private:
    uint8_t _addr;
    TwoWire *_wire;
    uint8_t _sda;
    uint8_t _scl;
    bool writeBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);
    bool readBytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length);

   public:
    ErrorCode begin(TwoWire *wire = &Wire, uint8_t sda = SDA, uint8_t scl = SCL,
               uint8_t addr = EXTIO2_DEFAULT_ADDR);
    bool setAllPinMode(extio_io_mode_t mode);
    bool setPinMode(uint8_t pin, extio_io_mode_t mode);
    bool setDeviceAddr(uint8_t addr);
    bool setDigitalOutput(uint8_t pin, uint8_t state);
    bool setAllDigitalOutputs(uint8_t pins);
    bool getDigitalInput(uint8_t pin);
    uint8_t getAllDigitalInputs(void);
    uint16_t getAnalogInput(uint8_t pin, extio_anolog_read_mode_t bit = _8bit);
    uint8_t getVersion();
};

#endif
