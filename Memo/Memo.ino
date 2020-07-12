// Tested with ESP32 Core V1.0.4
//#define ANCS //uncomment to enable ANCS messages
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include <Button.h> // https://github.com/JChristensen/Button

#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "bmp.h"

#include "ulp_main.h"
#include "ulptool.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>

#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
MPU6050 mpu;

#ifdef ANCS
  #include "esp32notifications.h" // https://www.github.com/Smartphone-Companions/ESP32-ANCS-Notifications.git
  BLENotifications notifications;
  uint32_t incomingCallNotificationUUID;
  boolean ancsDisconnectFlag = false;
  String latestNotificationTitle;
  String latestNotificationMessage;
  unsigned long notificatonCounter = 0;
#endif
 
char* ssid = "SSIDA";
char* password = "PASSWORDA";

char* ssidB = "SSIDB";
char* passwordB = "PASSWORDB";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#define BLACK 0
#define WHITE 1

#define WIDTH 400
#define HEIGHT 240

#define SHARP_SCK  14
#define SHARP_MOSI 13
#define SHARP_SS   15

#define I2C_SDA 26
#define I2C_SCL 25
#define INTP 34

#define ADC_BAT 39
#define CRG_STAT 36

#define ABUT 33
#define BBUT 32
#define UBUT 19
#define DBUT 23
#define LBUT 22
#define RBUT 18
#define CBUT 21
#define SBUT 35

//#define SKIPBOOTSCREEN

//Pushbuttons connected to GPIO32 & GPIO33 for wakeup
#define BUTTON_PIN_BITMASK 0x300000000
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");
gpio_num_t ulp_toggle_num = GPIO_NUM_2;

Button buttonA(ABUT, false, false, 5); //pin, pullup, invert, debounce
Button buttonB(BBUT, false, false, 5);
Button buttonUp(UBUT, true, true, 5); // turn on pullups for joystick only
Button buttonDown(DBUT, true, true, 5);
Button buttonLeft(LBUT, true, true, 5);
Button buttonRight(RBUT, true, true, 5);
Button buttonC(CBUT, true, true, 5);
Button buttonShoulder(SBUT, false, false, 5);

TaskHandle_t Task1;
TaskHandle_t Task2;

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 400, 240);

int currentPage = 0; // main menu

unsigned long fpsCounter = 0;
float fps = 0;

float battVoltage;
float chargeState;
boolean USBConnected = true;
boolean BLEConnected = false;
boolean Charging = false;
boolean batteryConnected = true;

struct tm now;
RTC_DATA_ATTR int syncCounter = 0; //count wakeups for time sync (RTC memory is not erased during sleep)

float mouseX, mouseY;

int16_t gx, gy, gz;
float sensX = 0.0022;
float sensY = 0.0018;

static void init_ulp_program() 
{
  esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
  ESP_ERROR_CHECK(err);

  rtc_gpio_init(ulp_toggle_num);
  rtc_gpio_pulldown_dis(ulp_toggle_num); // disable VCOM pulldown (saves 80µA)
  rtc_gpio_set_direction(ulp_toggle_num, RTC_GPIO_MODE_OUTPUT_ONLY);

  /* Set ULP wake up period to 500ms */
  ulp_set_wakeup_period(0, 500 * 1000);
}

void enterSleep(){
  //Serial.printf("Entering deep sleep\n\n");
  /* Turn off wireless features */ //needs to be tested but should be OK!
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  /* Put MPU6050 into sleep mode */
  mpu.setSleepEnabled(true);
  /* Start the ULP program */
  ESP_ERROR_CHECK( ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
  ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup() );
  //Configure GPIO32 & GPIO33 as ext1 wake up source for HIGH logic level
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  //wake up at full minute
  esp_sleep_enable_timer_wakeup((60-now.tm_sec) * 1000000);
  //Go to sleep now
  esp_deep_sleep_start();
}

void setup(void){

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause != ESP_SLEEP_WAKEUP_ULP) {
    init_ulp_program();
  }
  if(cause == ESP_SLEEP_WAKEUP_TIMER){
    if(syncCounter++ >= 360){ //sync with ntp server every 6h
      syncRTC();
      syncCounter = 0;
    }
    display.begin();
    display.clearDisplay();
    display.setRotation(0);
    getLocalTime(&now,0);
    drawClock();
    enterSleep();
  }
  pinMode(0, OUTPUT); // don't change, -10µA???

  display.begin();
  display.clearDisplay();
  display.setRotation(0); //Tetris?

  Serial.begin(115200);
  Serial.print("Wakeup cause: ");
  if(cause == ESP_SLEEP_WAKEUP_UNDEFINED) Serial.println("ESP_SLEEP_WAKEUP_UNDEFINED"); //normal reset
  else if(cause == ESP_SLEEP_WAKEUP_ALL) Serial.println("ESP_SLEEP_WAKEUP_ALL");
  else if(cause == ESP_SLEEP_WAKEUP_EXT0) Serial.println("ESP_SLEEP_WAKEUP_EXT0");
  else if(cause == ESP_SLEEP_WAKEUP_EXT1) Serial.println("ESP_SLEEP_WAKEUP_EXT1"); //button A&B
  else if(cause == ESP_SLEEP_WAKEUP_TIMER) Serial.println("ESP_SLEEP_WAKEUP_TIMER"); //scheduled timer wakeup
  else if(cause == ESP_SLEEP_WAKEUP_TOUCHPAD) Serial.println("ESP_SLEEP_WAKEUP_TOUCHPAD");
  else if(cause == ESP_SLEEP_WAKEUP_ULP) Serial.println("ESP_SLEEP_WAKEUP_ULP");
  else if(cause == ESP_SLEEP_WAKEUP_GPIO) Serial.println("ESP_SLEEP_WAKEUP_GPIO");
  else if(cause == ESP_SLEEP_WAKEUP_UART) Serial.println("ESP_SLEEP_WAKEUP_UART");


  pinMode(ABUT, INPUT);
  pinMode(BBUT, INPUT);
  pinMode(UBUT, INPUT_PULLUP);
  pinMode(DBUT, INPUT_PULLUP);
  pinMode(LBUT, INPUT_PULLUP);
  pinMode(RBUT, INPUT_PULLUP);
  pinMode(CBUT, INPUT_PULLUP);
  pinMode(SBUT, INPUT_PULLUP);

  pinMode(CRG_STAT, INPUT);
  pinMode(ADC_BAT, INPUT);
  pinMode(INTP, INPUT_PULLUP);
  adcAttachPin(CRG_STAT);
  adcAttachPin(ADC_BAT);
  analogReadResolution(12);
  analogSetWidth(12);
  analogSetAttenuation(ADC_11db);

  
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  mpu.initialize();
  mpu.setXAccelOffset(-1839);
  mpu.setYAccelOffset(893);
  mpu.setZAccelOffset(4861);
  mpu.setXGyroOffset(87);
  mpu.setYGyroOffset(22);
  mpu.setZGyroOffset(-130);
  
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */     

  #ifdef ANCS
    notifications.begin("ESP32 Memo"); // Start ANCS 
    notifications.setConnectionStateChangedCallback(onBLEStateChanged);
    notifications.setNotificationCallback(onNotificationArrived);
    notifications.setRemovedCallback(onNotificationRemoved);
  #endif
                           
  if(cause == ESP_SLEEP_WAKEUP_UNDEFINED){ //run on reset only
    #if !defined SKIPBOOTSCREEN
      display.fillScreen(BLACK);
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.println("Memory Display OS Version 0.07");
      display.refresh();
      display.println("Copyright(C) 2019-2020 CoreTech");
      display.refresh();
      display.printf("CPU: Xtensa LX6 2 CPU %dMHz\n\n", getCpuFrequencyMhz());
      display.refresh();

      if (WifiConnect(5) == false){ // switch to backup wifi
        ssid = ssidB;
        password = passwordB;
      }
      
      display.println("Fetching time from NTP server...");
      if(syncRTC()) display.println("Time synced");
      else display.println("Time sync failed");
      display.refresh();
      delay(500);
    #endif
  }

  setMenuPos();
  
}

void Task1code( void * pvParameters ){
  for(;;){
    adcAttachPin(CRG_STAT);
    adcAttachPin(ADC_BAT);
    analogReadResolution(12);
    analogSetWidth(12);
    analogSetAttenuation(ADC_11db);
  
    battVoltage = analogRead(ADC_BAT)*6.6/4096.0*1.1;
    if(battVoltage < 3.6 && battVoltage > 2){ //force standby when battery is critical
      display.fillScreen(WHITE);
      display.setTextSize(3);
      display.setTextColor(BLACK);
      display.setCursor(0, 26);
      display.println("  Battery Critical");
      display.println("  Please turn off");
      display.println("  and recharge");
      display.refresh();
      /* Put MPU6050 into sleep mode */
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      btStop();
      mpu.setSleepEnabled(true);
      esp_deep_sleep_start();
    }
    //TODO: measure V_USB in next hardware revision!
    if(battVoltage > 4.21){
      Charging = true;
      USBConnected = true;
      batteryConnected = true;
    }
    else if(battVoltage < 1){
      Charging = false;
      USBConnected = true;
      batteryConnected = false;
    }
    else{
      Charging = false;
      USBConnected = false;
      batteryConnected = true;
    }
    #ifdef ANCS
    // Restart BLE Advertising to prevent reconnect issue
    if(ancsDisconnectFlag) { 
      delay(100);
      notifications.startAdvertising();
      ancsDisconnectFlag = false;
    }
    #endif
    getLocalTime(&now,0);
    delay(500); //core 0 needs free time for background tasks
  }
}

void loop(void){
  fps = 1000/float(millis()-fpsCounter);
  fpsCounter = millis();

  
  //Serial.print(fps);
  //Serial.println("fps");

  if(currentPage == 1) drawPaint();
  //else if(currentPage == 2) drawSettings();
  else if(currentPage == 3) drawCalc();
  else if(currentPage == 4) drawNews();
  else if(currentPage == 5) drawClockApp();
  else if(currentPage == 6) drawLunar();
  else if(currentPage == 7) drawKeyboardDemo();
  //else if(currentPage == 8) drawOpac();
  else if(currentPage == 9) drawSpiritLevel();
  else if(currentPage == 10) drawHackaday();
  else drawMainMenu();
}

///

void drawTaskBar(){
  display.fillRect(0, 0, WIDTH, 16, BLACK);
  //time
  display.setFont();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  //clock
  display.setCursor(WIDTH/2-26,0);
  display.printf("%02d:%02d", now.tm_hour, now.tm_min);
  //battery
  //display.printf("   %d", analogRead(ADC_BAT));
  display.setCursor(10,0);  
  //display.printf(" %.2fV", battVoltage);
  display.drawRect(WIDTH-30, 3, 19, 10, WHITE);
  display.fillRect(WIDTH-11, 5, 2, 6, WHITE);
  if(batteryConnected){
    if(battVoltage > 3.7) display.fillRect(WIDTH-28, 5, 3, 6, WHITE);
    if(battVoltage > 3.8) display.fillRect(WIDTH-24, 5, 3, 6, WHITE);
    if(battVoltage > 3.9) display.fillRect(WIDTH-20, 5, 3, 6, WHITE);
    if(battVoltage > 4.1) display.fillRect(WIDTH-16, 5, 3, 6, WHITE);
  }
  //USB
  if(USBConnected){
    display.fillRect(WIDTH-45, 5, 9, 5, WHITE);
    display.fillRect(WIDTH-44, 2, 2, 4, WHITE);
    display.fillRect(WIDTH-39, 2, 2, 4, WHITE);
    display.fillRect(WIDTH-42, 10, 3, 4, WHITE);
  }
  if(BLEConnected){
    display.drawBitmap(WIDTH-57, 2, bluetooth_bmp, 6, 12, WHITE);
  }
  #ifdef ANCS
    if(notificatonCounter > millis()){
      display.fillRect(0, 16, WIDTH, 50, BLACK);
      display.fillRoundRect(5, 16, WIDTH-(5*2), 50-5, 6, WHITE);
      display.setCursor(8, 28);
      display.setTextSize(1);
      display.setTextColor(BLACK);
  
      display.setTextWrap(false);
      display.setFont(&FreeSansBold9pt7b);
      display.print(latestNotificationTitle.substring(0, 42));
      
      display.setCursor(8, 54);
      display.setFont(&FreeSans9pt7b);
      display.print(latestNotificationMessage.substring(0, 42));
      if(latestNotificationMessage.length() >= 38) display.print("...");
      display.setTextWrap(true);
    
      display.setFont();
    }
  #endif
    
}

void grayRect(int x, int y, int w, int h) {
  boolean toggle = true;
  for (int i = x; i < x+w; i++) {
    for (int k = y; k < y+h; k++) {
      if (toggle) display.drawPixel(i, k+i%2, BLACK);
      toggle = !toggle;
    }
  }
}

boolean syncRTC(){
  if(WifiConnect(5)){
    timeClient.begin();
    timeClient.setTimeOffset(7200); //+2hr for DST
    timeClient.update();
    timeval epoch = {timeClient.getEpochTime(), 0};
    const timeval *tv = &epoch;
    timezone utc = {0,0};
    const timezone *tz = &utc;
    settimeofday(tv, tz);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    return true;
  }
  else return false;
}

boolean WifiConnect(unsigned int timeout){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      timeout--;
      if(timeout == 0) break;
  }
  if(WiFi.status() == WL_CONNECTED) return true;
  else return false;
}

void drawClock(){
  display.fillScreen(BLACK);
  display.setTextSize(12);
  display.setTextColor(WHITE);
  display.setCursor(25, 60);
  display.printf("%02d:%02d", now.tm_hour, now.tm_min);
  display.setTextSize(4);
  display.setCursor(25, 160);
  display.printf("%d.%d.%d", now.tm_mday, now.tm_mon+1, now.tm_year+1900);
  display.refresh();
}

#ifdef ANCS
  void onNotificationRemoved(const ArduinoNotification * notification, const Notification * rawNotificationData) {
       display.printf("Removed notification: %s\n%s\n%s\n", notification->title, notification->message, notification->type);
  }
  
  void onNotificationArrived(const ArduinoNotification * notification, const Notification * rawNotificationData) {
      latestNotificationTitle = notification->title;
      latestNotificationMessage = notification->message;    
      display.printf("Got notification: %s\n%s\n%s\n", notification->title, notification->message, notification->type);
      Serial.println(notifications.getNotificationCategoryDescription(notification->category));  // ie "social media"
      Serial.println(notification->categoryCount); // How may other notifications are there from this app (ie badge number)
      notificatonCounter = millis() + 3000;
  }
  
  void onBLEStateChanged(BLENotifications::State state) {
    switch(state) {
        case BLENotifications::StateConnected:
            Serial.println("StateConnected - connected to a phone or tablet");
            BLEConnected = true;
            break;
  
        case BLENotifications::StateDisconnected:
            Serial.println("StateDisconnected - disconnected from a phone or tablet"); 
            ancsDisconnectFlag = true;
            BLEConnected = false;
            break; 
    }
  }
#endif
