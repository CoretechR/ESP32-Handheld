int calcXY[26][2] = {{  140, 80}, //'7'
                       {  180, 80}, //'8'
                       { 220, 80}, //'9'
                       { 260, 80}, //'/' ---
                       { 140, 120}, //'4'
                       { 180, 120}, //'5'
                       { 220, 120}, //'6'
                       { 260, 120}, //'x' ---
                       { 140, 160}, //'1'
                       { 180, 160}, //'2'
                       { 220, 160}, //'3'
                       { 260, 160}, //'-' ---
                       { 140, 200}, //'0'
                       { 180, 200}, //'C'
                       { 220, 200}, //'='
                       { 260, 200}, //'+'
                       };

char calcKeys[27] = {'7', '8', '9', '/', '4', '5', '6', 'x', '1', '2',
                         '3', '-', '0', 'C', '=', '+' };


void drawCalc(){
  
  static boolean firstRun = true;
  static float memA = 0;
  static float memB = 0;
  static char activeOp = 0;

  if(firstRun == true){
    sensX = 0.0028;
    sensY = 0.0028;
    mouseX = WIDTH/2;
    mouseY = HEIGHT/2;
    display.fillScreen(BLACK);
    memA = memB = 0;
  }

  display.fillScreen(BLACK);

  display.fillRect(110, 26, 180, 36, WHITE);
  
  display.setFont();
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(120,38);
  display.println(memA);  


  int mouseOverKey;

  mouseOverKey = -1;
  for(int i = 0; i < 26; i++){
    boolean keyColor = WHITE;    
    if(isAboveCKey(i)){
      mouseOverKey = i;
      keyColor = BLACK;
      display.fillCircle(calcXY[i][0]+5, calcXY[i][1]+7, 18, WHITE);
    }    
    display.drawChar(calcXY[i][0], calcXY[i][1], calcKeys[i], keyColor, BLACK, 2);
  }
  
  display.drawBitmap(mouseX, mouseY, pointer, 17, 25, WHITE);
  
  mpu.getRotation(&gx, &gy, &gz);
  
  drawTaskBar();
  int refreshtime = millis();
  display.refresh();
  refreshtime = millis() - refreshtime;
  
  firstRun = false;

  if((buttonC.wasPressed() || buttonB.wasPressed()) && mouseOverKey >= 0){
    if(calcKeys[mouseOverKey] >= 48 && calcKeys[mouseOverKey] < 58){
      memA = memA*10 + calcKeys[mouseOverKey] - 48;
    }
    else if(calcKeys[mouseOverKey] == '/' || calcKeys[mouseOverKey] == 'x' || 
            calcKeys[mouseOverKey] == '+' || calcKeys[mouseOverKey] == '-'){
      activeOp = calcKeys[mouseOverKey];
      memB = memA;
      memA = 0;
    }
    else if(calcKeys[mouseOverKey] == 'C'){
      memA = memB = 0;
    }
    else if(calcKeys[mouseOverKey] == '='){
      if(activeOp == '/') memA = memB/memA;
      else if(activeOp == 'x') memA = memB*memA;
      else if(activeOp == '+') memA = memB+memA;
      else if(activeOp == '-') memA = memB-memA;
      activeOp = 0;
    }
  }
  
  Serial.print("memA= ");
  Serial.print(memA);
  Serial.print(" memB= ");
  Serial.print(memB);
  Serial.print(" activeOp= ");
  Serial.println(activeOp);
  
  if(buttonLeft.wasPressed()){//backspace
    
  }

  if(buttonA.wasPressed()){
    currentPage = 0; // back to menu
    firstRun = true;
  }

  if(buttonShoulder.isPressed()){
    mouseX += gx*sensX;
    mouseY += gy*sensY;
    mouseX = constrain(mouseX, 0, WIDTH);
    mouseY = constrain(mouseY, 0, HEIGHT);
  }
  
  buttonA.read();
  buttonB.read();
  buttonC.read();
  buttonUp.read();
  buttonLeft.read();
  buttonRight.read();
  buttonDown.read();
  buttonShoulder.read();
}

boolean isAboveCKey(int keyNum){
  return (mouseX > calcXY[keyNum][0]-23 && mouseX < calcXY[keyNum][0]+13 && mouseY > calcXY[keyNum][1]-20 && mouseY < calcXY[keyNum][1]+11);
}
