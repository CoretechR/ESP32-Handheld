void drawClockApp(){
  
  static boolean firstRun = true;

  display.fillScreen(WHITE);
  display.setTextSize(12);
  display.setTextColor(BLACK);
  display.setCursor(25, 60);
  display.printf("%02d:%02d", now.tm_hour, now.tm_min);
  display.setTextSize(4);
  display.setCursor(25, 160);
  display.printf("%d.%d.%d", now.tm_mday, now.tm_mon+1, now.tm_year+1900);
  
  drawTaskBar();
  display.fillRect(WIDTH/2-26, 0, 60, 16, BLACK); //draw over taskbar time
  display.refresh();

  firstRun = false;

  if(buttonA.wasPressed()){
    currentPage = 0; // back to menu
    firstRun = true;
  }
  
  buttonA.read();
  buttonB.read();
  buttonUp.read();
  buttonLeft.read();
  buttonRight.read();
}
