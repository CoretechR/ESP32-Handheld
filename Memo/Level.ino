void drawSpiritLevel(){
  
  static boolean firstRun = true;
  static float angle = 0;

  if(firstRun == true){
  }

  angle = asin(float(mpu.getAccelerationY())/16000.0)*180.0/3.1415926;   // 3*cos(millis()/3000.0)*70.0;

  display.fillScreen(WHITE);
  float heightAdd = (WIDTH/2)*tan(degToRad(angle));
  float widthAdd = (HEIGHT/2)/tan(degToRad(angle));

  //display.drawLine(0, HEIGHT/2+angleAdd, WIDTH, HEIGHT/2-angleAdd, BLACK);
  if (angle < 31 && angle > -31) { //normal
    display.fillTriangle(0, HEIGHT, 0, HEIGHT/2-heightAdd, WIDTH, HEIGHT, BLACK);
    display.fillTriangle(0, HEIGHT/2-heightAdd, WIDTH, HEIGHT/2+heightAdd, WIDTH, HEIGHT, BLACK);
  } else if (angle > 31 && angle < 149 ) { //left tilt
    display.fillTriangle(0, 0, WIDTH/2-widthAdd, 0, 0, HEIGHT, BLACK);
    display.fillTriangle(WIDTH/2-widthAdd, 0, WIDTH/2+widthAdd, HEIGHT, 0, HEIGHT, BLACK);
  } else if (angle < -31 && angle > -149) { //right tilt
    display.fillTriangle(WIDTH, 0, WIDTH/2-widthAdd, 0, WIDTH, HEIGHT, BLACK);
    display.fillTriangle(WIDTH/2-widthAdd, 0, WIDTH/2+widthAdd, HEIGHT, WIDTH, HEIGHT, BLACK);
  } else { //upside town
    display.fillTriangle(0, 0, 0, HEIGHT/2-heightAdd, WIDTH, 0, BLACK);
    display.fillTriangle(0, HEIGHT/2-heightAdd, WIDTH, HEIGHT/2+heightAdd, WIDTH, 0, BLACK);
  }

  //display.fillCircle(WIDTH/2, HEIGHT/2, 30, WHITE);

  display.setFont();
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(WIDTH/2-20, HEIGHT/2-8);
  display.printf("%02.0f%c", angle, 247);

  
  drawTaskBar();  
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

float degToRad(float num){
  return 0.01745329 * num;
}
