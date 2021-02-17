### Example c_cpp_properties.json:
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "C:\\Program Files (x86)\\Arduino\\tools\\**",
                "C:\\Program Files (x86)\\Arduino\\libraries\\**",
                "C:\\Program Files (x86)\\Arduino\\libraries\\Servo\\src\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\tools\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\variants\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\variants\\standard",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\cores\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\libraries\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\bootloaders\\**",
                "C:\\Program Files (x86)\\Arduino\\hardware\\tools\\avr\\avr\\include",
                "C:\\Users\\sietz\\Documents\\Arduino\\libraries\\**"
            ],
            "forcedInclude": [
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\cores\\arduino\\Arduino.h"
            ],
            "defines": [
                "USBCON"
            ],
            "intelliSenseMode": "gcc-x64",
            "compilerPath": "\"C:/Program Files (x86)/Arduino/hardware/tools/avr/bin/avr-g++.exe\"-c -g -Os -w -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10813 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR",
            "cStandard": "c18",
            "cppStandard": "c++17"
        }
    ],
    "version": 4
}

### Example arduino.json:

{
    "board": "arduino:avr:uno",
    "sketch": "program\\program.ino",
    "output": "program\\ArduinoOutput",
    "port": "COM5",
    "debugger": "jlink"
}