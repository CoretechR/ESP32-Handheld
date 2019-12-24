// code derived from processing.org user antiplastik https://www.processing.org/discourse/beta/num_1259265932.html
void drawLunar(){
  
  static boolean firstRun = true;
  
  static float GRAVITY = 0.08;
  static float AIR = 0.99;
  static float TURN_VALUE = PI/40;
  static float PROP = 0.5;
  
  static float posx, posy;
  static float rot;
   
  static float vx, vy;
  static float vrot;

  if(firstRun == true){
    vx = vy = vrot = 0;
    rot = -PI/2;
    posx = display.width()/2;
    posy = display.height()/2;
    display.fillScreen(WHITE);
  }

  if(buttonUp.isPressed() || buttonB.isPressed()){
    vx += PROP*cos(rot);
    vy += PROP*sin(rot);
  }
  if(buttonLeft.isPressed()){
    vrot -= TURN_VALUE/10;
  }
  if(buttonRight.isPressed()){
    vrot += TURN_VALUE/10;
  }
  
  posx += vx;
  posy += vy;
  vy += GRAVITY;
  vrot *= AIR;
  vy *= AIR;
  rot += vrot;

  float rPos[3][2] = {{0, -5},{0, 5},{10, 0}};
  for(int i = 0; i < 3; i++){
    float ytemp = rPos[i][0];
    rPos[i][0] = posx + rPos[i][0]*cos(rot) - rPos[i][1]*sin(rot);
    rPos[i][1] = posy + ytemp*sin(rot) + rPos[i][1]*cos(rot);
  }
  display.fillRect(rPos[0][0]-15, rPos[0][1]-15, 30, 30, WHITE);
  display.fillTriangle(rPos[0][0], rPos[0][1], rPos[1][0], rPos[1][1], rPos[2][0], rPos[2][1], BLACK);
  
  drawTaskBar();
  display.refresh();

  firstRun = false;

  if(buttonA.wasPressed()){
    currentPage = 1; // back to menu
    firstRun = true;
  }
  
  buttonA.read();
  buttonB.read();
  buttonUp.read();
  buttonLeft.read();
  buttonRight.read();
}
