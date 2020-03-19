void drawHackaday(){
  
  static boolean firstRun = true;
 
    
  if(firstRun == true){

    display.fillScreen(WHITE);
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.print("Connecting to ");
    display.println(ssid);
    display.refresh();

    if(WifiConnect(8)){
      display.print("Connected\n");
      display.refresh();
      
      HTTPClient http;
   
      http.begin("https://hackaday.com/blog/feed/"); //Specify the URL
      int httpCode = http.GET();                                        //Make the request
      
      if (httpCode > 0) { //Check for the returning code
          String payload = http.getString();
          //Serial.println(payload);
      
          for(int i = 0; i < 12; i++){
            while(1){
              int first_bracket = payload.indexOf('>');
              if(first_bracket < 0){
                break;
              }
              else if(payload.substring(first_bracket-6, first_bracket) == "<title"){
                String headline = payload.substring(first_bracket+1, first_bracket+80);
                headline = headline.substring(0, headline.indexOf('<'));
                display.println(headline);
                payload = payload.substring(first_bracket+1);
                display.refresh();
                break;
              }
              //Serial.println(payload.substring(first_bracket-5, first_bracket));
              payload = payload.substring(first_bracket+1);
            }
          }
          
        } 
      else {
        Serial.println("Error on HTTP request");
      }    
   
      http.end(); //Free the resources
      WiFi.disconnect();
    }
    else{
      display.print("Connection failed!\n");
    }
      
  }


  
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
