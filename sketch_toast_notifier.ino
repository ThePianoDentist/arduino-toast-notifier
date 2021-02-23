#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(2, 3);      // RX, TX for ESP8266

bool DEBUG = true;   //show more logs
int responseTime = 40; //communication timeout

#define trigPin 12
#define echoPin 11

long duration;
int distance;
unsigned long lastReadyTime; // millis() resets after 50 days TODO handle rollover

void setup()
{
  pinMode(13,OUTPUT);  //set build in led as output
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT

  // Open serial communications and wait for port to open esp8266:
  Serial.begin(9600);
 wifiSerial.begin(9600);
 //delay(100);
 //sendToWifi("AT+CIPMODE=0",responseTime,DEBUG);
 delay(100);
 sendToWifi("AT+CIPMUX=1",responseTime,DEBUG);
 delay(100);
    Serial.println("setup complete");
  

}

void sendCmdsToWifi(){
    while(wifiSerial.available() > 0) 
    {
      char a = wifiSerial.read();
      if(a == '\0')
        continue;
      if(a != '\r' && a != '\n' && (a < 32))
        continue;
      Serial.print(a);
    }
    
    while(Serial.available() > 0)
    {
      char a = Serial.read();
      Serial.write(a);
      wifiSerial.write(a);
    }
}

void loop()
{
  distance = distance_measure();
  if (distance < 10){
    unsigned long now = millis();
    if (lastReadyTime == 0 || (now - lastReadyTime) > 10000){ // 2mins
      while (true){
        String fr = sendReadyReq();
        char final_response[fr.length()];
        fr.toCharArray(final_response, fr.length());
        if (fr.length() > 0 && final_response[0] == '\r'){
          Serial.println("breaking");
          break;
        }
        delay(100);
      }
      lastReadyTime = millis();
    }
  }
  delay(1000);
}

String sendReadyReq(){
  Serial.println("sending ready req");
  sendToWifi("AT+CIPSTART=4,\"TCP\",\"kettle-on.com\",80",responseTime,DEBUG);
  delay(50);
  sendToWifi("AT+CIPSEND=4,77",responseTime,DEBUG);
  delay(50);
  sendToWifi("GET /ready/ HTTP/1.1",responseTime,DEBUG);
  delay(50);
  sendToWifi("Host: kettle-on.com",responseTime,DEBUG);
  delay(50);
  sendToWifi("Content-Type: application/json",responseTime,DEBUG);
  delay(50);
  String out = sendToWifi("",responseTime,DEBUG);
  return out;
}



/*
* Name: sendToWifi
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToWifi(String command, const int timeout, boolean debug){
  //Serial.println("sending:" + command);
  String response = "";
  wifiSerial.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
    char c = wifiSerial.read(); // read the next character.
    response+=c;
    }  
  }
  if(debug)
  {
    Serial.println(response);
  }
  return response;
}

int distance_measure(){
  digitalWrite(trigPin, LOW); // Clears the trigPin condition
  delayMicroseconds(2);

  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);  //s * 10^3
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println(" cm");
  return distance;
}