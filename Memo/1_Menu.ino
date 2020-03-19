int menuVal = 0;
int menuPos[18][2];
                    
String menuTitles[18] = {"  Zeichnen   ", //0
                         "Einstellungen", //1
                         "   Rechner   ", //2
                         " Nachrichten ", //3
                         "    Uhr      ", //4
                         "Lunar Lander ", //5
                         "  Tastatur   ", //6
                         " Bibliothek  ", //7
                         " Wasserwaage ", //8
                         "  Hackaday   ", //9
                         "             ", //10
                         "             ", //11
                         "             ", //12
                         "             ", //13
                         "             ", //14
                         "             ", //15
                         "             ", //16
                         "             "};//17

void setMenuPos(){
  for(int k = 0; k < 6; k++){
    for(int i = 0; i < 3; i++){
      menuPos[k*3+i][0] = 23 + k*60;
      menuPos[k*3+i][1] = 31 + i*60;
    }
  }  
}

void drawMainMenu(){

  static boolean firstRun = true;

  if(firstRun == true){
    display.fillScreen(WHITE);
  }
  
  if(buttonB.wasPressed()){
    currentPage = menuVal+1;
  }

  if(buttonA.wasPressed()){
    drawClock();
    delay(200); //debounce
    enterSleep();
  }

  if(buttonLeft.wasPressed()) if(menuVal >= 2) menuVal-=3;
  if(buttonRight.wasPressed()) if(menuVal < 15) menuVal+=3;
  if(buttonUp.wasPressed()) menuVal--;
  if(buttonDown.wasPressed()) menuVal++;
  if(menuVal > 17) menuVal = 17;
  if(menuVal < 0) menuVal = 0;
  
  display.fillScreen(WHITE);
  //grayRect(0, 16, WIDTH, HEIGHT-16);
  drawTaskBar();
  drawAppSelect();
  drawIcons();
  
  display.refresh();
  buttonA.read();
  buttonB.read();
  buttonUp.read();
  buttonDown.read();
  buttonLeft.read();
  buttonRight.read();
  buttonC.read();
  firstRun = false;
}

void drawAppSelect(){
  //app highlight
  display.drawBitmap(menuPos[menuVal][0]-5, menuPos[menuVal][1]-5, select_bmp, 58, 58, BLACK);
  //app description
  display.fillRect(100, HEIGHT-24, WIDTH-200, 2, BLACK);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(WIDTH/2-74, HEIGHT-18);
  display.println(menuTitles[menuVal]);
}

void drawIcons(){
 
  display.drawBitmap(menuPos[0][0], menuPos[0][1], paint_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[1][0], menuPos[1][1], settings_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[2][0], menuPos[2][1], calc_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[3][0], menuPos[3][1], news_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[4][0], menuPos[4][1], clock_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[5][0], menuPos[5][1], lander_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[6][0], menuPos[6][1], keyboard_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[7][0], menuPos[7][1], opac_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[8][0], menuPos[8][1], level_bmp, 48, 48, BLACK);
  display.drawBitmap(menuPos[9][0], menuPos[9][1], hackaday_bmp, 48, 48, BLACK);
  
}
