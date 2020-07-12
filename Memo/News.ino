void drawNews(){
  
  static boolean firstRun = true;
  display.setFont();
    
  if(firstRun == true){

    display.fillScreen(WHITE);
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.print("Connecting to ");
    display.println(ssid);
    display.refresh();
    #ifdef ANCS
    notifications.stop();
    delay(200);
    #endif    

    if(WifiConnect(8)){
      display.print("Connected\n");
      display.refresh();
    
      HTTPClient http;
   
      http.begin("http://www.tagesschau.de/xml/atom/"); //Specify the URL
      int httpCode = http.GET();                                        //Make the request
      
      if (httpCode > 0) { //Check for the returning code
          String payload = http.getString();
          payload.replace("ü", String(char(129))); //ü
          payload.replace("ä", String(char(132))); //ä
          payload.replace("ö", String(char(148))); //ö
          payload.replace("Ü", String(char(154))); //Ü
          payload.replace("Ä", String(char(142))); //Ä
          payload.replace("Ö", String(char(153))); //Ö
          payload.replace("ß", String(char(224))); //ß
      
          for(int i = 0; i < 20; i++){
            while(1){
              int first_bracket = payload.indexOf('>');         
              if(first_bracket < 0){
                break;
              }
              else if(payload.substring(first_bracket-5, first_bracket) == "title"){
                String headline = payload.substring(first_bracket+1, first_bracket+80);
                headline = headline.substring(0, headline.indexOf('<'));
                display.print(headline);
                payload = payload.substring(first_bracket+1);
                display.refresh();
                break;
              }
              payload = payload.substring(first_bracket+1);
            }
          }
          
        } 
      else {
        Serial.println("Error on HTTP request");
      }    
   
      http.end(); //Free the resources
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      delay(200);
      
    }
    else{
      display.print("Connection failed!\n");
    }
    
    
  }


  display.setFont();
  drawTaskBar();
  display.refresh();

  firstRun = false;

  if(buttonA.wasPressed()){
    currentPage = 0; // back to menu
    firstRun = true;
  }
  
  buttonA.read();
  buttonB.read();
}
