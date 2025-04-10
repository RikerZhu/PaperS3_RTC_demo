/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 *
 *
 * @Hardwares: PaPerS3
 * @Dependent Library:
 * M5Unified@^0.2.3: https://github.com/m5stack/M5Unified
 * tanakamasayuki/I2C BM8563 RTC@^1.0.4:https://github.com/tanakamasayuki/I2C_BM8563
 * epdiy=https://github.com/vroland/epdiy.git#d84d26ebebd780c4c9d4218d76fbe2727ee42b47 
 */

 #include "I2C_BM8563.h"
 #include <WiFi.h>
 #include <M5Unified.h>
 
 // RTC configuration
 #define BM8563_I2C_SDA 41
 #define BM8563_I2C_SCL 42
 I2C_BM8563 rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire);
 const char* ntpServer = "time.cloudflare.com";
 const char* ssid      = "M5Stack";
 const char* password  = "m5office888";
 
 M5Canvas canvas(&M5.Display);
 // Records the number of seconds last displayed
 uint8_t last_second = 255;
 // Record the time of the last global refresh
 uint32_t last_refresh_time = 0;
 
 void setup()
 {
     M5.begin();
     Serial.begin(115200);
     M5.Display.setRotation(1);
     M5.Display.setEpdMode(epd_mode_t::epd_fast);
     canvas.createSprite(M5.Display.width(), M5.Display.height());
     canvas.setTextDatum(top_left);
     canvas.setFont(&fonts::Font7);
     canvas.setTextSize(1);
 
     // WiFi
     WiFi.begin(ssid, password);
     Serial.print("Connecting to Wi-Fi");
     while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial.print(".");
     }
     Serial.println("\nConnected");
 
     // Set ntp time to local (GMT+8)
     configTime(8 * 3600, 0, ntpServer);
 
     // Example Initialize the RTC
     Wire.begin(BM8563_I2C_SDA, BM8563_I2C_SCL);
     rtc.begin();
 
     // Synchronize RTC time
     struct tm timeInfo;
     if (getLocalTime(&timeInfo)) {
         I2C_BM8563_TimeTypeDef timeStruct;
         timeStruct.hours   = timeInfo.tm_hour;
         timeStruct.minutes = timeInfo.tm_min;
         timeStruct.seconds = timeInfo.tm_sec;
         rtc.setTime(&timeStruct);
 
         I2C_BM8563_DateTypeDef dateStruct;
         dateStruct.weekDay = timeInfo.tm_wday;
         dateStruct.month   = timeInfo.tm_mon + 1;
         dateStruct.date    = timeInfo.tm_mday;
         dateStruct.year    = timeInfo.tm_year + 1900;
         rtc.setDate(&dateStruct);
     }
 
     // Record initial time
     last_refresh_time = millis();
     canvas.fillSprite(TFT_WHITE);
   Serial.printf("PIO");
   Serial.printf("Deafult free size: %d\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
   Serial.printf("PSRAM free size: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
   Serial.printf("Flash size: %d bytes\n", ESP.getFlashChipSize());
 }
 void loop()
 {
     I2C_BM8563_DateTypeDef dateStruct;
     I2C_BM8563_TimeTypeDef timeStruct;
 
     // Obtain the RTC time
     rtc.getDate(&dateStruct);
     rtc.getTime(&timeStruct);
 
     uint32_t current_time = millis();
 
     // Update the screen only when the number of seconds changes
     if (timeStruct.seconds != last_second) {
         last_second = timeStruct.seconds;
 
         // Prepare the display content
         char timeStr[20];
         sprintf(timeStr, "%02d:%02d:%02d", timeStruct.hours, timeStruct.minutes, timeStruct.seconds);
 
         char dateStr[20];
         sprintf(dateStr, "%04d-%02d-%02d", dateStruct.year, dateStruct.month, dateStruct.date);
 
         // Clear the canvas and update the display
         canvas.fillSprite(TFT_WHITE);
         canvas.setTextColor(TFT_BLACK);
 
         // Drawing time (center display)
         int textWidth = canvas.textWidth(timeStr);
         canvas.drawString(timeStr, (M5.Display.width() - textWidth) / 2, M5.Display.height() / 2);
 
         // Drawing date (center display)
         textWidth = canvas.textWidth(dateStr);
         canvas.drawString(dateStr, (M5.Display.width() - textWidth) / 2, M5.Display.height() / 3);
 
         // Push the canvas content to the display
         canvas.pushSprite(0, 0);
     }
 
     // Global refresh every 60 seconds
     if (current_time - last_refresh_time > 60000) {
         Serial.println("Performing full refresh...");
         // Force a global refresh
         M5.Display.clear();
         last_refresh_time = current_time;
     }
     delay(50);
 }
 