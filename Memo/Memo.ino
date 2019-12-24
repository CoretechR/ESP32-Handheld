#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <Button.h> // https://github.com/JChristensen/Button
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "esp32/ulp.h"

#include "ulp_main.h"
#include "ulptool.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
 
const char* ssid = "your_ssid";
const char* password = "your_password";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#define BLACK 0
#define WHITE 1

#define SHARP_SCK  14
#define SHARP_MOSI 13
#define SHARP_SS   15

#define ADC_BAT 39
#define CRG_STAT 36

#define ABUT 33
#define BBUT 32
#define UBUT 19
#define DBUT 23
#define LBUT 22
#define RBUT 18
#define CBUT 21

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

TaskHandle_t Task1;
TaskHandle_t Task2;

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 400, 240);


int currentPage = 1; // main menu

unsigned long fpsCounter = 0;
float fps = 0;

float battVoltage;
float chargeState;
boolean USBConnected = true;
boolean Charging = false;
boolean batteryConnected = true;

struct tm now;
RTC_DATA_ATTR int syncCounter = 0; //count wakeups for time sync (RTC memory is not erased during sleep)

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

  pinMode(CRG_STAT, INPUT);
  pinMode(ADC_BAT, INPUT);
  adcAttachPin(CRG_STAT);
  adcAttachPin(ADC_BAT);
  analogReadResolution(12);
  analogSetWidth(12);
  analogSetAttenuation(ADC_11db);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */        
                              
  if(cause == ESP_SLEEP_WAKEUP_UNDEFINED){ //run on reset only
    display.fillScreen(BLACK);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println("Memory Display OS Version 0.03");
    display.refresh();
    display.println("Copyright(C) 2019-2020 CoreTech");
    display.refresh();
    display.printf("CPU: Xtensa LX6 2 CPU %dMHz\n\n", getCpuFrequencyMhz());
    display.refresh();
    display.println("Fetching time from NTP server...");
    if(syncRTC()) display.println("Time synced");
    else display.println("Time sync failed");
    display.refresh();
    delay(500);
  }
  
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
    getLocalTime(&now,0);
    delay(500); //core 0 needs free time for background tasks
  }
}

void loop(void){
  fps = 1000/float(millis()-fpsCounter);
  fpsCounter = millis();

  
  Serial.print(fps);
  Serial.println("fps");

  if(currentPage == 2) drawLunar();
  else if(currentPage == 3) drawNews();
  else if(currentPage == 4) drawClockApp();
  else if(currentPage == 6) drawHackaday();
  else drawMainMenu();
}

///

void drawTaskBar(){
  display.fillRect(0, 0, display.width(), 16, BLACK);
  //time
  display.setTextSize(2);
  display.setTextColor(WHITE);
  //clock
  display.setCursor(display.width()/2-26,0);
  display.printf("%02d:%02d", now.tm_hour, now.tm_min);
  //battery
  //display.printf("   %d", analogRead(ADC_BAT));
  display.setCursor(10,0);  
  display.printf(" %.2fV", battVoltage);
  display.drawRect(display.width()-30, 3, 19, 10, WHITE);
  display.fillRect(display.width()-11, 5, 2, 6, WHITE);
  if(batteryConnected){
    if(battVoltage > 3.7) display.fillRect(display.width()-28, 5, 3, 6, WHITE);
    if(battVoltage > 3.8) display.fillRect(display.width()-24, 5, 3, 6, WHITE);
    if(battVoltage > 3.9) display.fillRect(display.width()-20, 5, 3, 6, WHITE);
    if(battVoltage > 4.1) display.fillRect(display.width()-16, 5, 3, 6, WHITE);
  }
  //USB
  if(USBConnected){
    display.fillRect(display.width()-45, 5, 9, 5, WHITE);
    display.fillRect(display.width()-44, 2, 2, 4, WHITE);
    display.fillRect(display.width()-39, 2, 2, 4, WHITE);
    display.fillRect(display.width()-42, 10, 3, 4, WHITE);
  }
  //fps
  display.setCursor(10,0);
  //display.print(fps);
  //display.print("fps");
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
  if(WifiConnect(8)){
    timeClient.begin();
    timeClient.setTimeOffset(3600); //+1hr
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
