// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// System interface routines for Windows with raw LovyanGFX (non-M5, rectangular screen)

// stdio.h must precede LovyanGFX headers for image file functions to work correctly
#include "stdio.h"

#include "System.h"
#include "FluidNCModel.h"
#include "Drawing.h"
#include "NVS.h"

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "LGFX_SDL.hppx"  // defines lgfx::LGFX wrapping Panel_sdl

#include <windows.h>
#include <commctrl.h>
#include <direct.h>

// Concrete LGFX instance sized to the configured display dimensions
static lgfx::LGFX xdisplay(DISPLAY_WIDTH, DISPLAY_HEIGHT);
LGFX_Device&      display = xdisplay;

LGFX_Sprite canvas(&xdisplay);

static m5::Touch_Class xtouch;
m5::Touch_Class&       touch = xtouch;

bool  round_display = false;
Point sprite_offset { 0, 0 };

// ---------------------------------------------------------------------------
// Serial (Win32 API)
// ---------------------------------------------------------------------------

#define TIOCM_LE  0x001
#define TIOCM_DTR 0x002
#define TIOCM_RTS 0x004
#define TIOCM_ST  0x008
#define TIOCM_SR  0x010
#define TIOCM_CTS 0x020
#define TIOCM_CAR 0x040
#define TIOCM_RNG 0x080
#define TIOCM_DSR 0x100
#define TIOCM_CD  TIOCM_CAR
#define TIOCM_RI  TIOCM_RNG

static bool getcomm(HANDLE comfid, LPDCB dcb) {
    if (!GetCommState((HANDLE)comfid, dcb)) {
        printf("Can't get COM mode, error %d\n", GetLastError());
        return true;
    }
    return false;
}

static bool setcomm(HANDLE comfid, LPDCB dcb) {
    if (!SetCommState((HANDLE)comfid, dcb)) {
        printf("Can't set COM mode, error %d\n", GetLastError());
        return true;
    }
    return false;
}

bool serial_set_baud(HANDLE comfid, DWORD baudrate) {
    DCB dcb;
    if (getcomm(comfid, &dcb)) {
        return true;
    }
    dcb.BaudRate = baudrate;
    return setcomm(comfid, &dcb);
}

int serial_timed_read_com(HANDLE handle, LPVOID buffer, DWORD len, DWORD ms) {
    HANDLE       hComm = (HANDLE)handle;
    COMMTIMEOUTS timeouts;
    DWORD        actual;

    timeouts.ReadIntervalTimeout         = 1;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.ReadTotalTimeoutConstant    = ms;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant   = 10;

    if (!SetCommTimeouts(hComm, &timeouts)) {
        printf("Can't set COM timeout\n");
        CloseHandle((HANDLE)hComm);
        return -1;
    }

    ReadFile(hComm, (LPVOID)buffer, (DWORD)len, &actual, NULL);
    return actual;
}

int serial_write(HANDLE handle, LPCVOID buffer, DWORD len) {
    DWORD actual;
    (void)WriteFile((HANDLE)handle, (LPCVOID)buffer, (DWORD)len, (LPDWORD)&actual, NULL);
    return actual;
}

HANDLE serial_open_com(char* portname) {
    wchar_t      wcomname[10];
    DCB          dcb;
    HANDLE       hComm;
    COMMTIMEOUTS timeouts;

    _snwprintf(wcomname, 10, L"\\\\.\\%S", portname);
    hComm = CreateFileW(wcomname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hComm == INVALID_HANDLE_VALUE) {
        return hComm;
    }

    FillMemory(&dcb, sizeof(dcb), 0);
    dcb.DCBlength = sizeof(DCB);

    if (!BuildCommDCB("115200,n,8,1", &dcb)) {
        printf("Can't build DCB\n");
        CloseHandle((HANDLE)hComm);
        return INVALID_HANDLE_VALUE;
    }

    if (!SetCommState(hComm, &dcb)) {
        printf("Can't set COM mode, error %d\n", GetLastError());
        CloseHandle((HANDLE)hComm);
        return INVALID_HANDLE_VALUE;
    }

    timeouts.ReadIntervalTimeout         = 2;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.ReadTotalTimeoutConstant    = 100;
    timeouts.WriteTotalTimeoutMultiplier = 2;
    timeouts.WriteTotalTimeoutConstant   = 100;

    if (!SetCommTimeouts(hComm, &timeouts)) {
        printf("Can't set COM timeout\n");
        CloseHandle((HANDLE)hComm);
        return INVALID_HANDLE_VALUE;
    }

    return hComm;
}

extern char* comname;

#ifndef STUB_SERIAL
static HANDLE hFNC;
#endif

// ---------------------------------------------------------------------------
// init_system / update_events
// ---------------------------------------------------------------------------

void init_system() {
    lgfx::Panel_sdl::setup();
    xdisplay.init();
    touch.begin(&xdisplay);
    canvas.setColorDepth(16);
    canvas.createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#ifndef STUB_SERIAL
    hFNC = serial_open_com(comname);
    if (hFNC == INVALID_HANDLE_VALUE) {
        printf("Can't open %s\n", comname);
        exit(1);
    }
    serial_set_baud(hFNC, 115200);
#endif
    display.clear();
}

void update_events() {
    lgfx::Panel_sdl::loop();
    touch.update(lgfx::millis());
}

// ---------------------------------------------------------------------------
// Serial I/O (extern "C" for C linkage expected by GrblParser / FluidNCModel)
// ---------------------------------------------------------------------------

#ifdef STUB_SERIAL

extern "C" void fnc_putchar(uint8_t c) {}

extern "C" int fnc_getchar() {
    return -1;
}

#else

extern "C" void fnc_putchar(uint8_t c) {
    serial_write(hFNC, &c, 1);
}

extern "C" int fnc_getchar() {
    char c;
    int  cnt = serial_timed_read_com(hFNC, &c, 1, 1);
    if (cnt > 0) {
        update_rx_time();
        return c;
    }
    return -1;
}

#endif  // STUB_SERIAL

extern "C" void poll_extra() {}

// ---------------------------------------------------------------------------
// Debug output
// ---------------------------------------------------------------------------

void dbg_write(uint8_t c) {
    putchar(c);
}

void dbg_print(const char* s) {
    char c;
    while ((c = *s++) != '\0') {
        putchar(c);
    }
}

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------

extern "C" int milliseconds() {
    return lgfx::millis();
}

void delay_ms(uint32_t ms) {
    SDL_Delay(ms);
}

// ---------------------------------------------------------------------------
// PNG drawing
// ---------------------------------------------------------------------------

void drawPngFile(const char* filename, int x, int y) {
    drawPngFile(&canvas, filename, x, y);
}

void drawPngFile(LGFX_Sprite* sprite, const char* filename, int x, int y) {
    std::string fn("data/");
    fn += filename;
    sprite->drawPngFile(fn.c_str(), x, -y, 0, 0, 0, 0, 1.0f, 1.0f, datum_t::middle_center);
}

// ---------------------------------------------------------------------------
// NVS (file-based, stored under prefs/)
// ---------------------------------------------------------------------------

static FILE* prefFile(const char* handle, const char* pname, const char* mode) {
    static char fname[60];
    snprintf(fname, 60, "%s/%s", handle, pname);
    return fopen(fname, mode);
}

void nvs_get_str(nvs_handle_t handle, const char* name, char* value, size_t* len) {
    FILE* fd = prefFile(handle, name, "rb");
    if (fd) {
        *len = fread(value, 1, *len - 1, fd);
        fclose(fd);
    } else {
        *len = 0;
    }
    value[*len] = '\0';
}

void nvs_set_str(nvs_handle_t handle, const char* name, const char* value) {
    FILE* fd = prefFile(handle, name, "wb");
    if (fd) {
        fwrite(value, 1, strlen(value), fd);
        fclose(fd);
    }
}

void nvs_get_i32(nvs_handle_t handle, const char* name, int* value) {
    char   strval[20];
    size_t len = 20;
    nvs_get_str(handle, name, strval, &len);
    if (*strval) {
        *value = atoi(strval);
    }
}

void nvs_set_i32(nvs_handle_t handle, const char* name, int value) {
    char valstr[20];
    snprintf(valstr, 20, "%d", value);
    nvs_set_str(handle, name, valstr);
}

nvs_handle_t nvs_init(const char* name) {
    char dname[50];
    _mkdir("prefs");
    snprintf(dname, 50, "prefs/%s", name);
    _mkdir(dname);
    return strdup(dname);
}

// ---------------------------------------------------------------------------
// Display / layout
// ---------------------------------------------------------------------------

void base_display() {
    display.clear();
}

void show_logo() {}

void system_background() {}

void set_layout(int n) {}

void next_layout(int delta) {}

// ---------------------------------------------------------------------------
// Input stubs (rectangular display has no inherent click zones)
// ---------------------------------------------------------------------------

bool screen_encoder(int x, int y, int& delta) {
    return false;
}

bool screen_button_touched(bool pressed, int x, int y, int& button) {
    return false;
}

bool switch_button_touched(bool& pressed, int& button) {
    return false;
}

// ---------------------------------------------------------------------------
// Encoder stubs (Encoder.cpp excluded; Encoder.h declares these)
// ---------------------------------------------------------------------------

int16_t get_encoder() {
    return 0;
}

void init_encoder(int a_pin, int b_pin) {}

// ---------------------------------------------------------------------------
// Miscellaneous stubs
// ---------------------------------------------------------------------------

void ackBeep() {}

void deep_sleep(int us) {}

void resetFlowControl() {}

bool ui_locked() {
    return false;
}
