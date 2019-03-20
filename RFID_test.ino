#include <SoftwareSerial.h>

//esp8266 variable
SoftwareSerial esp8266(2,3);
int connectionId = 0;
const bool DEBUG = true;
const String sn = "SE4XDR5CFT";
String TCPconnect = "AT+CIPSTART=\"TCP\",\"www2.cs.ccu.edu.tw\",80\r\n";

//RFID variable
unsigned char command[] = {0x43,0x03,0x01};
String finalRSSI[20];
String sendRSSI = "";
String RSSI = "";
int index = 0;

//display variable
const int button_read = 4;
const int led_output_green = 5;
const int led_output_red = 6;
const int led_output_power = 7;
bool flag = false;

void setup() {
  pinMode(button_read,INPUT);
  pinMode(led_output_green,OUTPUT);
  pinMode(led_output_red,OUTPUT);
  pinMode(led_output_power,OUTPUT);
  Serial.begin(115200);
  esp8266.begin(9600);
  //String res = "";
  Serial.println("Arduino start");
  while(1){
    esp8266.print("AT+RST\r\n");
    if(esp8266.find("OK")){
      Serial.println("esp8266 ready......");
      break;
    }
  }
  while(1){
   esp8266.print("AT+CWMODE=1\r\n");
   if(esp8266.find("OK")){
     Serial.println("STA mode set");
     break;
    }
  }
  while(1){
   esp8266.print("AT+CIPMUX=0\r\n");
   if(esp8266.find("OK")){
     Serial.println("connection line set");
     break;
    }
  }
  while(1){
    esp8266.print("AT+CWSTARTSMART\r\n");
    if(esp8266.find("smartconfig connected wifi")){
      Serial.println("Smartconfig OK");
      esp8266.print("AT+CWSTOPSMART\r\n");
      digitalWrite(led_output_power,HIGH);
      break;
    }
  }
  digitalWrite(led_output_power,HIGH);
  Serial.println("esp8266 has set successful");
  Serial.println("RFID test begining......");
}

void loop() {
  int i = 0;
  int button_state = digitalRead(button_read);
  if(button_state == 1){
    digitalWrite(led_output_green,LOW);
    digitalWrite(led_output_red,LOW);
    digitalWrite(led_output_green,HIGH);
    for(int j=0;j<10;j++){
      Serial.println(j);
      if(flag){
        led_red_high();
      }
      else{
        led_green_high();
      }
      scantag();
    }
    delay(1000);
    sendJudge();
  }
}
boolean scantag()
{
  boolean flag = false;
  char letter[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  int state = 0;
  delay(1000);
  RSSI = "";
  Serial.write(command,3);
  while(Serial.available() > 0){
    int val = Serial.read();
    //Serial.println(String(state) + " " + String(val));
    //Serial.println(val,HEX);
    switch(state){
      case 0:
        if(val == 0x44){state = 1;}
        break;
      case 1:
        if(val == 0x16){state = 2;}
        else if(val == 0x05){state = 22;}
        break;
     case 2:
        state = 3;
        break;
     case 3:
        state = 4;
        break;
     case 4:case 5:case 6:
        state++;
        break;
     case 7:
        if(val == 0x0E){state = 8;}
        break;
     case 8:
        if(val == 0x30){state = 9;}
        break;
     case 9:
        if(val == 0x00){state = 10;}
        break;
     case 10:case 11:case 12:case 13:case 14:case 15:case 16:case 17:case 18:case 19:case 20:case 21:
        if(state == 10){
          RSSI = "";
        }
        int tenDigit,digit;
        tenDigit = val / 16;
        digit = val % 16;
        //Serial.print(String(tenDigit) + " " + String(digit) + "\n");
        RSSI.concat(String(letter[tenDigit]) + String(letter[digit]));
        state++;
        if(state == 22){
          state = 0;
          if(index == 0){
            finalRSSI[index++] = RSSI;
            sendRSSI.concat("-");
            sendRSSI += RSSI;
          }
          else{
            int i;
            for(i=0;i<index;i++){
              if(finalRSSI[i] == RSSI) break;
            }
            if(i == index){
              finalRSSI[index++] = RSSI;
              sendRSSI.concat("-");
              sendRSSI += RSSI;
              Serial.println(RSSI);
              Serial.println(sendRSSI);
            }
          }
          flag = true;
        }
        break;
      case 22:
        if(val == 0x00){state = 23;}
        break;
      case 23:
        if(val == 0x00){state = 24;}
        break;
      case 24:
        if(val == 0x00){
          state = 0;
          flag = true;
          Serial.println("No tag");
        }
        break;
      default: break;
    }
  }
  return flag;
}
void sendJudge()
{
  String getcommand = "GET http://www2.cs.ccu.edu.tw/~hhc104u/judge.php?sn="; 
  getcommand += sn;
  getcommand += "&tag=";
  getcommand += sendRSSI;
  getcommand += "&num=";
  getcommand += String(index);
  getcommand += "\r\n";
  Serial.println(getcommand);
  while(1){
    Serial.println(TCPconnect);
    esp8266.print(TCPconnect);
    if(esp8266.find("OK")){
      Serial.println("TCP connected");
      break;
    }
  }
  while(1){
    String senddata = "AT+CIPSEND=";
    senddata.concat(String(getcommand.length()));
    senddata.concat("\r\n");
    Serial.print(senddata);
    esp8266.print(senddata);
    if(esp8266.find(">")){
      esp8266.print(getcommand);
      break;
    }
  }
  digitalWrite(led_output_green,LOW);
  digitalWrite(led_output_red,LOW);
  delay(100);
  if(esp8266.find("YES")){
    Serial.println("Complete");
    digitalWrite(led_output_green,HIGH);
  }
  else{
    Serial.println("Something incomplete");
    digitalWrite(led_output_red,HIGH);
  }
  for(int i=0;i<20;i++){
    finalRSSI[i] = "";
  }
  sendRSSI = "";
  index = 0;
}
void led_green_high(){
  digitalWrite(led_output_green,HIGH);
  digitalWrite(led_output_red,LOW);
  flag = true;
}
void led_red_high(){
  digitalWrite(led_output_green,LOW);
  digitalWrite(led_output_red,HIGH);
  flag = false;
}

