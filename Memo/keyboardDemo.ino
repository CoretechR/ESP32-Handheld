int keybXY[26][2] = {{  20, 140}, //'Q'
                       {  60, 140}, //'W'
                       { 100, 140}, //'E'
                       { 140, 140}, //'R'
                       { 180, 140}, //'T'
                       { 220, 140}, //'Z'
                       { 260, 140}, //'U'
                       { 300, 140}, //'I'
                       { 340, 140}, //'O'
                       { 380, 140}, //'P' ---
                       {  40, 170}, //'A'
                       {  80, 170}, //'S'
                       { 120, 170}, //'D'
                       { 160, 170}, //'F'
                       { 200, 170}, //'G'
                       { 240, 170}, //'H'
                       { 280, 170}, //'J'
                       { 320, 170}, //'K'
                       { 360, 170}, //'L' ---
                       {  60, 200}, //'Y'
                       { 100, 200}, //'X'
                       { 140, 200}, //'C'
                       { 180, 200}, //'V'
                       { 220, 200}, //'B'
                       { 260, 200}, //'N'
                       { 300, 200}, //'M'
                       };

char keybChar[27] = {'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P',
                         'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
                           'Y', 'X', 'C', 'V', 'B', 'N', 'M', ' ' };

void drawKeyboardDemo(){
  
  static boolean firstRun = true;
  static String textString = "";

  if(firstRun == true){
    sensX = 0.0028;
    sensY = 0.0028;
    mouseX = WIDTH/2;
    mouseY = HEIGHT/2;
    display.fillScreen(BLACK);
    textString = "";
  }

  display.fillScreen(BLACK);
  display.setFont();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,40);
  //show cursor blinking at 2Hz
  if(millis()/500%2){
    textString += '_';
    display.println(textString);
    textString.remove(textString.length()-1);
  }
  else display.println(textString);
  

  int mouseOverKey;

  mouseOverKey = -1;
  for(int i = 0; i < 26; i++){
    boolean keyColor = WHITE;    
    if(isAboveKey(i)){
      mouseOverKey = i;
      keyColor = BLACK;
      display.fillCircle(keybXY[i][0]+5, keybXY[i][1]+7, 18, WHITE);
    }
    
    display.drawChar(keybXY[i][0], keybXY[i][1], keybChar[i], keyColor, BLACK, 2);
  }
  if(mouseY > 216){
    mouseOverKey = 27; //space
    display.fillRect(WIDTH/2-100, HEIGHT-16, 200, 20, WHITE);
  }

  
  display.drawBitmap(mouseX, mouseY, pointer, 17, 25, WHITE);
  
  mpu.getRotation(&gx, &gy, &gz);
  
  drawTaskBar();
  int refreshtime = millis();
  display.refresh();
  refreshtime = millis() - refreshtime;
  Serial.print("refresh: ");
  Serial.print(refreshtime);
  Serial.print("  ");
  
  firstRun = false;

  if((buttonC.wasPressed() || buttonB.wasPressed()) && mouseOverKey >= 0){//add character
    textString += keybChar[mouseOverKey];
  }

  if(buttonLeft.wasPressed()){//backspace
    textString.remove(textString.length()-1);
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

boolean isAboveKey(int keyNum){
  return (mouseX > keybXY[keyNum][0]-23 && mouseX < keybXY[keyNum][0]+13 && mouseY > keybXY[keyNum][1]-20 && mouseY < keybXY[keyNum][1]+11);
}
