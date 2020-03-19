

void drawPaint(){
  
  static boolean firstRun = true;
  static boolean drawColor = BLACK;
  static int brushSize = 7; //must be even number
  boolean pointerArea[brushSize][brushSize];

  if(firstRun == true){
    display.fillScreen(WHITE);
    //display.drawBitmap(0, 15, wallpaper, 400, 225, BLACK);
    sensX = 0.0022;
    sensY = 0.0022;
    mouseX = WIDTH/2;
    mouseY = HEIGHT/2;
    delay(200); //don't draw accidentally
    buttonB.read();
    buttonC.read();
  }

  //save pointer background area
  for(int k = 0; k < brushSize; k++){ //column y
    for(int i = 0; i < brushSize; i++){ //row x   
      int pAx = mouseX - brushSize/2 + i;
      int pAy = mouseY  - brushSize/2 + k;   
      if(pAx >= 0 && pAy >= 0)
        pointerArea[i][k] = display.getPixel(pAx, pAy);
    }
  }

  

  display.fillCircle(mouseX, mouseY, float(brushSize)/2 - 0.5, !drawColor); // draw pointer
  display.fillCircle(mouseX, mouseY, float(brushSize)/2 - 1.5, drawColor); // draw pointer

  
  drawTaskBar();

  mpu.getRotation(&gx, &gy, &gz);
  display.refresh();

  //restore pointer background
  for(int k = 0; k < brushSize; k++){ //column y
      for(int i = 0; i < brushSize; i++){ //row x
        int pAx = mouseX - brushSize/2 + i;
        int pAy = mouseY  - brushSize/2 + k;
        if(pAx >= 0 && pAy >= 0)          
          display.drawPixel(pAx, pAy, pointerArea[i][k]);
    }
  }
    
  if(buttonB.isPressed() || buttonC.isPressed()){//draw over screen
    display.fillCircle(mouseX, mouseY, brushSize/2, drawColor); // draw pointer
  }
  
  firstRun = false;

  if(buttonUp.isPressed()){
    if(brushSize < 35) brushSize += 1;
  }
  
  if(buttonDown.isPressed()){
    if(brushSize > 5) brushSize -= 1;
  }
  
  if(buttonLeft.wasPressed() || buttonRight.wasPressed()){
    drawColor = !drawColor;
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
