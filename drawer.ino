/* Turn an LED on/off based on a command send via BlueTooth
**
** Credit: The following example was used as a reference
** Rui Santos: http://randomnerdtutorials.wordpress.com
*/
#include <SoftwareSerial.h>
#define TIMEOUT 5000 // mS
#define DEBUG true
#define echoPin 10 // Echo Pin
#define trigPin 11 // Trigger Pin
#define ledPin 13 // use the built in LED on pin 13 of the Uno
#define WIFIPWR 3 // WiFi power mode
#define maximumRange 200; // Maximum range needed
#define minimumRange 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance

SoftwareSerial esp8266(6, 5);

int open = 1;
int state = '1';
int flag = 0;

void setup() {
    // sets the pins as outputs:
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    pinMode(WIFIPWR, OUTPUT);
//    digitalWrite(WIFIPWR, HIGH);
//    delay(5500);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.begin(9600); // Default connection rate for my BT module
    esp8266.begin(9600);
//    sendCommand("AT\r\n",1000,DEBUG);
/*
    // wifi part
    Serial.begin(9600);
    mySerialW.begin(115200);
    SendCommand("AT+RST", "Ready");
    delay(5000);
    SendCommand("AT+CWMODE=1","OK");
    SendCommand("AT+CIFSR", "OK");
    SendCommand("AT+CIPMUX=1","OK");
    SendCommand("AT+CIPSERVER=1,80","OK");
    mySerial.println("setup done"); */
}

void loop() {
    if(Serial.available() > 0){
      state = Serial.read();
      delay(5);
      while (Serial.available()) {
        Serial.read();
      }
      if (state == '0') {
        Serial.println("Lock OFF");
        digitalWrite(ledPin, LOW);
      } else {
        Serial.println("Lock ON");
        digitalWrite(ledPin, HIGH);
      }
    }
    if (state == '1') {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = duration/58.2; //Calculate the distance (in cm) based on the speed of sound.
      Serial.print(distance);
  //    /* Send the distance to the computer using Serial protocol, and
  //    turn LED OFF to indicate successful reading. */
       if (distance > 15) {
        if (open == 0) {
          open = 1;
          Serial.print(" -> sending");
//         digitalWrite(ledPin, HIGH);
         digitalWrite(WIFIPWR, HIGH);
         delay(2000);
         sendCommand("AT+CIPSTART=\"TCP\",\"y-team.herokuapp.com\",80\r\n",2000,DEBUG);
         sendHTTPPOST(0,"123");
         delay(2000);
         digitalWrite(WIFIPWR, LOW);
         Serial.println();
        }
        Serial.println();
       } else if (distance > 0) {
        Serial.println(": dw closed");
//         digitalWrite(ledPin, LOW);
         open = 0;
       }

       delay(1000);
    }

    // ---- wifi ----
/*
 String IncomingString="";
 boolean StringReady = false;
 
 if(mySerialW.available() > 0){
  flag=0;
   IncomingString=mySerialW.readString();
   StringReady= true;
  }
 
  if (StringReady){
    Serial.println("Received String: " + IncomingString);
  
  if (IncomingString.indexOf("LED=ON") != -1) {
        digitalWrite(ledPin, LOW);
        if(flag == 0){
          mySerial.println("LED: off");
          flag = 1;
        }
   }
 
  if (IncomingString.indexOf("LED=OFF") != -1) {
        digitalWrite(ledPin, HIGH);
        if(flag == 0){
          mySerial.println("LED: on");
          flag = 1;
        }
   }
  }
*/
}

/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    esp8266.listen();

    int dataSize = command.length();
    char data[dataSize];
    command.toCharArray(data,dataSize);

    esp8266.write(data,dataSize); // send the read character to the esp8266
    if(debug)
    {
      Serial.println("\r\n====== Transmission From Arduino ======");
      Serial.write(data,dataSize);
      Serial.println("\r\n========================================");
    }

    long int time = millis();

    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {

        // The esp has data so display its output to the serial window
        char c = esp8266.read(); // read the next character.
        response+=c;
      }
    }

    if(debug)
    {
      Serial.print(response);
    }

    return response;
}

/*
* Name: sendHTTPResponse
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPResponse(int connectionId, String content)
{
     esp8266.listen();
     // build HTTP response
     String httpResponse;
     String httpHeader;
     // HTTP Header
     httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n";
     httpHeader += "Content-Length: ";
     httpHeader += content.length();
     httpHeader += "\r\n";
     httpHeader +="Connection: close\r\n\r\n";
     httpResponse = httpHeader + content + " "; // There is a bug in this code: the last character of "content" is not sent, I cheated by adding this extra space
     sendCIPData(connectionId,httpResponse);
}
/*
* Name: sendHTTPPOST
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPPOST(int connectionId, String content)
{
    esp8266.listen();
     // build HTTP response
     String httpResponse;
     String httpHeader;
     // HTTP Header
     httpHeader = "POST /alert/ HTTP/1.1\r\n";
     httpHeader += "Host: y-team.herokuapp.com\r\n";
     httpHeader += "Connection: close\r\nContent-Type: application/x-www-form-urlencoded\r\n";
     httpHeader += "Content-Length: ";
     httpHeader += content.length();
     httpHeader += "\r\n\r\n";
     httpResponse = httpHeader + content; // There is a bug in this code: the last character of "content" is not sent, I cheated by adding this extra space
     sendCIPData(connectionId,httpResponse);
}

/*
* Name: sendCIPDATA
* Description: sends a CIPSEND=<connectionId>,<data> command
*
*/
void sendCIPData(int connectionId, String data)
{
    esp8266.listen();
   String cipSend = "AT+CIPSEND=";
//   cipSend += connectionId;
//   cipSend += ",";
   cipSend +=data.length();
   cipSend +="\r\n";
   sendCommand(cipSend,2000,DEBUG);
   sendData(data,2000,DEBUG);
}

/*
* Name: sendCommand
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendCommand(String command, const int timeout, boolean debug)
{
    esp8266.listen();
    String response = "";

    esp8266.print(command); // send the read character to the esp8266

    long int time = millis();

    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {

        // The esp has data so display its output to the serial window
        char c = esp8266.read(); // read the next character.
        response+=c;
      }
    }

    if(debug)
    {
      Serial.print(response);
    }

    return response;
}

void espRST(boolean debug) {
    esp8266.listen();
    esp8266.begin(baudRate()); //finds initial baud rate
    sendCommand("AT+RST\r\n", 2000, false);
    esp8266.begin(115200);
    sendCommand("AT+CIOBAUD=9600\r\n", 2000, false);
    esp8266.begin(9600);
}

/* finds baud rate */
long baudRate()
{
  esp8266.begin(9600);
  sendCommand("AT\r\n", 1000, DEBUG);
   if(esp8266.find((char*) "OK")){
    return 9600;
   } else {
     return 115200;
   }
}

