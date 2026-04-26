// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// System interface routines for the Arduino framework

#include "System.h"
#include "M5GFX.h"
#include "Drawing.h"
#include "HardwareM5Dial.hpp"
#ifdef USE_WIFI
#    include "WiFiConnection.h"
#endif
#ifdef USE_EXTIO
#    include "M5_EXTIO2.h"
M5_EXTIO2 extio;
volatile uint8_t extio_inputs;


void extioTask(void* parameter) {
    while (true) {
        extio_inputs = extio.getAllDigitalInputs();
        // USBSerial.printf("extio_inputs: %02x\n", extio_inputs);
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

#endif

LGFX_Device&       display = M5Dial.Display;
LGFX_Sprite        canvas(&M5Dial.Display);
m5::Speaker_Class& speaker   = M5Dial.Speaker;
m5::Touch_Class&   touch     = M5Dial.Touch;
Stream&            debugPort = USBSerial;

m5::Button_Class& dialButton = M5Dial.BtnA;
m5::Button_Class  greenButton;
m5::Button_Class  redButton;
#ifdef USE_EXTIO
m5::Button_Class  abortButton;
m5::Button_Class  macroButton;
#endif

bool round_display = true;

void init_hardware() {
    auto cfg = M5.config();

    // Don't enable the encoder because M5's encoder driver is flaky
    M5Dial.begin(cfg, false, false);

    // Turn on the power hold pin
    lgfx::gpio::command(lgfx::gpio::command_mode_output, GPIO_NUM_46);
    lgfx::gpio::command(lgfx::gpio::command_write_high, GPIO_NUM_46);

    #ifdef DEBUG_TO_USB
    // This must be done after M5Dial.begin which sets the PortA pins
    // to I2C mode.  We need to override that to use them for serial.
    // The baud rate is irrelevant because USBSerial emulates a UART
    // API but the data never travels over an actual physical UART
    // link with a defined baud rate.  The data instead travels over
    // a USB link at the USB data rate.  You can set the baud rate
    // at the other end to anything you want and it will still work.
    USBSerial.begin();
    #endif

#ifdef USE_WIFI
    if (wifi_use_uart_mode()) {
        init_fnc_uart(FNC_UART_NUM, PND_TX_FNC_RX_PIN, PND_RX_FNC_TX_PIN);
    }
#else
    init_fnc_uart(FNC_UART_NUM, PND_TX_FNC_RX_PIN, PND_RX_FNC_TX_PIN);
#endif

#ifdef USE_EXTIO
    // Port A (GPIO1/2) stays in I2C mode from M5Dial.begin(); Wire is already initialized.
    //  while(!USBSerial) {
    //         delay(100);
    //     }
    // USBSerial.println("extio init");
        display.setCursor(20, display.height() / 2);
        display.print("Initializing ExtIO...");
    ErrorCode e = ErrorCode::Uninit;
    while ((e = extio.begin(&Wire)) != ErrorCode::Ok) {
        display.printf("ExtIO init error: %d\n", e);

        if(e == ErrorCode::Timeout){
            display.println();
            display.println("ExtIO init timeout, resetting...");
            Wire.endTransmission();
            Wire.end();
            delay(2000);
        }
       
        delay(50);
        // USBSerial.printf("extio Connect Error: %d\n", e);
    }
    display.clear();
    extio.setAllPinMode(DIGITAL_INPUT_MODE);

    // display.setCursor(20, display.height() / 2);
    // int i = 0;
    // while (true) {
    // display.clear();
    //      display.drawString("FW VERSION: " + String(extio.getVersion()) , 10, 40);
    //      display.drawString("All: " + String(extio.getAllDigitalInputs(), 16), 10, 60);
    //     for (uint8_t i = 0; i < 8; i++) {
    //         if (extio.getDigitalInput(i)) {
    //             display.fillRect(i * 20 + 20, 145, 18, 20, ORANGE);
    //         } else {
    //             display.drawRect(i * 20 + 20, 145, 18, 20, ORANGE);
    //         }
    //     }
    // vTaskDelay(100);
    // }

    xTaskCreatePinnedToCore(extioTask, "extioTask"  // A name just for humans
                            ,
                            2048  // This stack size can be checked & adjusted
                                  // by reading the Stack Highwater
                            ,
                            NULL,
                            1  // Priority, with 3 (configMAX_PRIORITIES - 1)
                               // being the highest, and 0 being the lowest.
                            ,
                            NULL, 1);
       
    // GPIO13 (WAKEUP_GPIO) is a dedicated deep-sleep wakeup pin — connect a button to GPIO13+GND.
    // lgfx::gpio::command(lgfx::gpio::command_mode_input_pullup, WAKEUP_GPIO);
    abortButton.setDebounceThresh(5);
    macroButton.setDebounceThresh(5);
#else
    // Setup external GPIOs as buttons
    lgfx::gpio::command(lgfx::gpio::command_mode_input_pullup, RED_BUTTON_PIN);
    lgfx::gpio::command(lgfx::gpio::command_mode_input_pullup, GREEN_BUTTON_PIN);
#endif
    greenButton.setDebounceThresh(5);
    redButton.setDebounceThresh(5);

    init_encoder(ENC_A, ENC_B);

    speaker.setVolume(255);

    touch.setFlickThresh(30);
}

Point sprite_offset { 0, 0 };

void show_logo() {
    display.drawPngFile(LittleFS, "/fluid_dial.png", 0, 0, display.width(), display.height(), 0, 0, 0.0f, 0.0f, datum_t::middle_center);
}

void base_display() {
    display.clear();
}

void next_layout(int delta) {}

void system_background() {
    canvas.fillSprite(TFT_BLACK);
}

bool switch_button_touched(bool& pressed, int& button) {
    if (redButton.wasPressed()) {
        button  = 0;
        pressed = true;
        return true;
    }
    if (redButton.wasReleased()) {
        button  = 0;
        pressed = false;
        return true;
    }
    if (dialButton.wasPressed()) {
        button  = 1;
        pressed = true;
        return true;
    }
    if (dialButton.wasReleased()) {
        button  = 1;
        pressed = false;
        return true;
    }
    if (greenButton.wasPressed()) {
        button  = 2;
        pressed = true;
        return true;
    }
    if (greenButton.wasReleased()) {
        button  = 2;
        pressed = false;
        return true;
    }
    #ifdef USE_EXTIO
    if (abortButton.wasPressed()) {
        button  = 3;
        pressed = true;
        return true;
    }
    if (abortButton.wasReleased()) {
        button  = 3;
        pressed = false;
        return true;
    }
    if (macroButton.wasPressed()) {
        button  = 4;
        pressed = true;
        return true;
    }
    if (macroButton.wasReleased()) {
        button  = 4;
        pressed = false;
        return true;
    }
    #endif
    return false;
}

bool screen_encoder(int x, int y, int& delta) {
    return false;
}
bool screen_button_touched(bool pressed, int x, int y, int& button) {
    return false;
}

void update_events() {
    M5Dial.update();

    auto ms = m5gfx::millis();

#ifdef USE_EXTIO
    // Single I2C read for all 8 pins; buttons are active-low (0 = pressed)
    redButton.setRawState(ms,   !(extio_inputs & (1 << EXTIO_RED_PIN)));
    greenButton.setRawState(ms, !(extio_inputs & (1 << EXTIO_GREEN_PIN)));
    abortButton.setRawState(ms, !(extio_inputs & (1 << EXTIO_ABORT_PIN)));
    macroButton.setRawState(ms, !(extio_inputs & (1 << EXTIO_MACRO_PIN)));
#else
    // The red and green buttons are active low
    redButton.setRawState(ms, !m5gfx::gpio_in(RED_BUTTON_PIN));
    greenButton.setRawState(ms, !m5gfx::gpio_in(GREEN_BUTTON_PIN));
#endif
}

void ackBeep() {
    speaker.tone(1800, 50);
}

bool ui_locked(bool redrawButtonsFlag) {
    return false;
}

int num_layouts = 1;
int32_t layout_num = 0;
void redrawButtons() {}

#include <driver/rtc_io.h>
// The M5 Library is broken with respect to deep sleep on M5 Dial
// so we have to do it ourselves.  The problem is that the WAKE
// button is supposed to be the dial button that connects to GPIO42,
// but that can't work because GPIO42 is not an RTC GPIO and thus
// cannot be used as an ext0 wakeup source.
void deep_sleep(int us) {
#ifndef USE_EXTIO
    display.sleep();
    rtc_gpio_pullup_en((gpio_num_t)WAKEUP_GPIO);

    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_GPIO, false);
    while (digitalRead(WAKEUP_GPIO) == false) {
        delay_ms(10);
    }
    if (us > 0) {
        esp_sleep_enable_timer_wakeup(us);
    } else {
        // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    }
    esp_deep_sleep_start();
#endif
}
