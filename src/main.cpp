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
        parse_args(String, String *),
        draw(String *, uint8_t *),
        plot_cpu_graph(uint8_t, uint8_t *);

//vars
const unsigned char PROGMEM celsius_icon[] =  { 0xce,0xd1,0x10,0x10,0x10,0x11,0x0e };
uint8_t cpu_graph[94] = {0};

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

        String args[NUM_ARGS];
        uint8_t *cpu_graph_ = cpu_graph;

        if (Serial.available()) {  
                parse_args(Serial.readString(), (String *)&args);
                draw((String *)&args, cpu_graph_);
                lcd.display();
                lcd.clearDisplay();
        } else {
                while(!Serial.available());
        }
}

void draw (String *arr_args, uint8_t *cpu_graph_) {

        uint8_t cpu_temp = arr_args[0].toInt();
        uint8_t cpu_util = arr_args[1].toInt();

        uint16_t ram_total = arr_args[2].toInt();
        uint16_t ram_available = arr_args[3].toInt();
        uint8_t ram_util = 100*float(1 - (float(ram_available) / float(ram_total)));

        String date = arr_args[4];

        String ssd_total = arr_args[5];
        uint8_t ssd_usage_ = arr_args[6].substring(0, arr_args[6].indexOf('%')).toInt();
        uint8_t ssd_usage = float(ssd_total.substring(0, ssd_total.indexOf('G')).toFloat() * (float(ssd_usage_)/100.0));

        String hdd_total = arr_args[7];
        uint8_t hdd_usage = arr_args[8].toInt();
        uint8_t hdd_usage_ = float(float(hdd_usage)*100.0 / float(hdd_total.substring(0, hdd_total.indexOf('G')).toInt()));

        lcd.setCursor(32, 0);
        lcd.print(date);
        lcd.drawLine(0, 8, lcd.width(), 8, BLACK);

        lcd.setCursor(0,10);
        lcd.print(F("CPU:")); lcd.print(cpu_temp); lcd.drawBitmap(36, 10, celsius_icon, 8, 7, BLACK);  
        lcd.drawRoundRect(50, 10, 46, 7, 1, BLACK);
        lcd.fillRect(52, 12, map(cpu_util, 0, 100, 0, 42), 3, BLACK);

        lcd.setCursor(0,40);
        lcd.print(F("RAM:")); lcd.print(ram_util); lcd.println(F("%"));
        lcd.drawRoundRect(50, 40, 46, 7, 1, BLACK);
        lcd.fillRect(52, 42, map(ram_util, 0, 100, 0, 42), 3, BLACK);

        lcd.print(F("SSD:")); lcd.print(ssd_usage_); lcd.print(F("%"));
        lcd.setCursor(52, 48);
        lcd.print(ssd_usage); lcd.print(F("/")); lcd.println(ssd_total);
    
        lcd.print(F("HDD:")); lcd.print(hdd_usage_); lcd.print(F("%"));
        lcd.setCursor(52, 56);
        lcd.print(hdd_usage); lcd.print(F("/")); lcd.println(hdd_total + "G");
        plot_cpu_graph(cpu_util, cpu_graph_);
}

void plot_cpu_graph (uint8_t cpu_util, uint8_t *cpu_graph_) {

        lcd.drawRect(0, 18, lcd.width(), 21, BLACK);
        *(cpu_graph+0) = cpu_util;
        for (int i = 94; i > 0; i--) {

                if (*(cpu_graph+94-i) != 0) {

                        lcd.drawRect(i, 37, 1, -map(cpu_graph[94-i], 0, 100, 0, 17), BLACK);        
                }
                *(cpu_graph+i) = *(cpu_graph+i-1);
        }
}

void parse_args(String str_args, String *arr_args) {

        int8_t index_delim = 0;
        int8_t prev_index_delim = 0;

        for (int i = 0; i < NUM_ARGS; i++) {

                index_delim = str_args.indexOf(';');
                if (index_delim != -1) {

                        str_args.setCharAt(index_delim, ' ');
                        *(arr_args+i) = str_args.substring(prev_index_delim, index_delim);
                        prev_index_delim = index_delim + 1;
                }
        }
}
