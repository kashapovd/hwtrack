/*
  hwtrack - system monitor with arduino
  Author: Yaroslav Kashapov
  Created 28 jan 2020
 */

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include <ST7558.h>
#include <Wire.h>
#include <SPI.h>

#define RST_PIN A3
#define NUM_ARGS 9
    
//functions
void
        parseArgs(String, String *),
        draw(String *),
        processArgs(String *),
        plotCpuGraph(uint8_t);

//vars
const unsigned char PROGMEM celsius_icon[] =  { 0xce,0xd1,0x10,0x10,0x10,0x11,0x0e };
uint8_t cpuGraph[94] = {0};
struct structArgs {

        struct {
                unsigned int temp : 7;
                unsigned int util : 7;
        } cpu;

        struct {

                uint16_t total;
                uint16_t available;
                uint8_t util : 7;
        } ram;

        struct {
                String total;
                uint8_t usage : 7; 
                uint8_t load : 7;
                        
        } ssd;

        struct {
                String total;
                uint8_t usage;
                uint8_t load : 7;
        } hdd;

        String date;
        
} args;


ST7558 lcd = ST7558(RST_PIN);

void setup() {
        Serial.begin(9600);    
        // lcd init
        Wire.begin();
        lcd.init();
        lcd.setRotation(0);
        lcd.setContrast(70);
        lcd.clearDisplay();

        if(!Serial.available()) {
                lcd.print(F("Plug in and \nrun the program"));
                lcd.display();
                lcd.clearDisplay();
        }
}

//main loop
void loop() {
        String stringArgs[NUM_ARGS];

        if (Serial.available()) {
                parseArgs(Serial.readString(), (String *)&stringArgs);
                draw((String *)&stringArgs);
                lcd.display();
                lcd.clearDisplay();
        } else {
                while(!Serial.available());
        }
}

void draw (String *arr_args) {
        processArgs(arr_args);
        plotCpuGraph(args.cpu.util);

        lcd.setCursor(32, 0);
        lcd.print(args.date);
        lcd.drawLine(0, 8, lcd.width(), 8, BLACK);

        lcd.setCursor(0,10);
        lcd.print(F("CPU:")); lcd.print(args.cpu.temp); lcd.drawBitmap(36, 10, celsius_icon, 8, 7, BLACK);  
        lcd.drawRoundRect(50, 10, 46, 7, 1, BLACK);
        lcd.fillRect(52, 12, map(args.cpu.util, 0, 100, 0, 42), 3, BLACK);

        lcd.setCursor(0,40);
        lcd.print(F("RAM:")); lcd.print(args.ram.util); lcd.println(F("%"));
        lcd.drawRoundRect(50, 40, 46, 7, 1, BLACK);
        lcd.fillRect(52, 42, map(args.ram.util, 0, 100, 0, 42), 3, BLACK);

        lcd.print(F("SSD:")); lcd.print(args.ssd.load); lcd.print(F("%"));
        lcd.setCursor(52, 48);
        lcd.print(args.ssd.usage); lcd.print(F("/")); lcd.println(args.ssd.total);
    
        lcd.print(F("HDD:")); lcd.print(args.hdd.load); lcd.print(F("%"));
        lcd.setCursor(52, 56);
        lcd.print(args.hdd.usage); lcd.print(F("/")); lcd.println(args.hdd.total + "G");
        
}

void processArgs(String *arr_args) {
        args.cpu.temp = arr_args[0].toInt();
        args.cpu.util = arr_args[1].toInt();
        args.ram.total = arr_args[2].toInt();
        args.ram.available = arr_args[3].toInt();
        args.ram.util = 100*float(1 - (float(args.ram.available) / float(args.ram.total)));
        args.date = arr_args[4];
        args.ssd.total = arr_args[5];
        args.ssd.usage = arr_args[6].substring(0, arr_args[6].indexOf('G')).toInt();
        args.ssd.load = float(float(args.ssd.usage)*100.0 / float(args.ssd.total.substring(0, args.ssd.total.indexOf('G')).toInt()));
        args.hdd.total = arr_args[7];
        args.hdd.usage = arr_args[8].toInt();
        args.hdd.load  = float(float(args.hdd.usage )*100.0 / float(args.hdd.total.substring(0, args.hdd.total.indexOf('G')).toInt()));
}

void plotCpuGraph (uint8_t cpu_util) {
        lcd.drawRect(0, 18, lcd.width(), 21, BLACK);
        *(cpuGraph+0) = cpu_util;
        for (int i = 94; i > 0; i--) {
                if (*(cpuGraph+94-i) != 0) {
                        lcd.drawRect(i, 37, 1, -map(cpuGraph[94-i], 0, 100, 0, 17), BLACK);        
                }
                *(cpuGraph+i) = *(cpuGraph+i-1);
        }
}

void parseArgs(String str_args, String *arr_args) {
        int8_t idelim;
        int8_t prev_idelim;
        prev_idelim = idelim = 0;
        for (int i = 0; i < NUM_ARGS; i++) {
                idelim = str_args.indexOf(';');
                if (idelim != -1) {
                        str_args.setCharAt(idelim, ' ');
                        *(arr_args+i) = str_args.substring(prev_idelim, idelim);
                        prev_idelim = idelim + 1;
                }
        }
}
