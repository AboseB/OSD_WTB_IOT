
/* This sketch will connect to the AWS Cloud using WiFi and publish a message.
 *  it will also toggle the LED on ESP32 from the AWS Cloud 
 */

/* Import required libraries */
#include <AWS_IOT.h>
#include <ArduinoJson.h>
#include "WiFi.h"

/* AWS_IOT instance */
AWS_IOT hornbill;
AWS_IOT shadow;

/* LED Pin */
#define LED_PIN 23

/* Network Credentials */
#if 0
const char* ssid ="Outerspace1a";
const char* password ="osdg_wireless_1";
#else
const char* ssid ="BoseOppo";
const char* password ="manu@0906";
#endif
int status = WL_IDLE_STATUS;

/* AWS Custom Endpoint Address */
char HOST_ADDRESS[]="aynd9wcdbmxsz-ats.iot.us-east-2.amazonaws.com"; 

/* Device details */
char CLIENT_ID[]= "WTB-IOT1";
char TOPIC_NAME[]= "OSD/WTB-IOT1";
//char CLIENT_ID[]= "OSD-WTB-Thing1";
char SHADOW_GET[]= "$aws/things/OSD-WTB-Thing1/shadow/get/accepted";
char SENT_GET[]= "$aws/things/OSD-WTB-Thing1/shadow/get";
char SHADOW_UPDATE[]= "$aws/things/OSD-WTB-Thing1/shadow/update";

/* Globals for communication with the AWS cloud */
char payload[100];
int msgReceived=0;
char reportpayload[512];
char rcvdPayload[512];

/* Defines and globals for publishing wave pattern */
#define WAVE_LOWER_LIMIT    -5
#define WAVE_UPPER_LIMIT     5
signed int value = 0;
signed char increment = 1;

void setup(){

  /* Initialization of serial ports */
  Serial.begin(115200);
  Serial2.begin(115200);

  Serial.println("**** OSD Internal IOT Demo *****");

  /* Connect to WiFi */
  ConnectWifi();

/* The connection to AWS for subscribe and publish seems to be mutually exclusive.
 *  Hence only one type of connection (Publsh or Subscribe) is done at a time.
 *  @TODO: Need to explore this in detail
 */
#if 0
  /* Connect to AWS (for Publish) */
  ConnectAWS();
#else
  /* Connect to AWS (for Subscribe, ie. shadow opeations) */
  ConnectShadow();
#endif

  Serial.println("-------------------------------------------------");
}

void loop()
{
  
  /* This will construct a wave pattern (-5..0..5..0..-5) */
  //PublishWave();
  
  /* This will construct a message from the serial port (user input) */
  //PublishSerial();

  /* This will construct a message received from the MCU over UART (Serial Port 2) */
  PublishMsgFromMCU();

  /* This will check messages received from AWS and take action */
  //CheckMsgFromAWS();
}

void ConnectWifi(void)
{
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Trying to connect to SSID: ");
    Serial.println(ssid);
  }
  
  Serial.print("Connected to WiFi. Local IP: ");
  
  // Print local IP Address
  Serial.println(WiFi.localIP());
}

void ConnectAWS(void)
{
  /* Connect to AWS using Host Address and Client ID */
  if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0)
  {
    Serial.println("Connected to AWS!");
    delay(1000);
  }
  else
  {
    Serial.println("AWS connection failed, Check the HOST Address.");
    while(1);
  }  
}

void ConnectShadow(void)
{
  /* Connect to AWS IoT Core */
  if(shadow.connect(HOST_ADDRESS,CLIENT_ID)== 0)
  {
      Serial.println("Connected to AWS!");
      delay(1000);

      /* Subscribe to Accepted GET Shadow Service */
      if(0==shadow.subscribe(SHADOW_GET,mySubCallBackHandler))
      {
          Serial.println("Subscribe Successfull !");
      }
      else
      {
          Serial.println("Subscribe Failed, Check the Thing Name and Certificates.");
          while(1);
      }
  }
  else
  {
      Serial.println("AWS connection failed, Check the HOST Address.");
      while(1);
  }

  delay(3000); 
  
  /*Sent Empty string to fetch Shadow desired state*/ 
  if(shadow.publish(SENT_GET,"{}") == 0)
  {       
      Serial.print("Empty String Published!\n");
  }
  else
  {
      Serial.println("Empty String Publish failed.\n");
  }  /*Sent Empty string to fetch Shadow desired state*/   
}

void PublishMsgToAWS(void)
{
  /* Print to serial port 0 for debug */
  //Serial.println(payload);

  /* Publish the message to AWS */
  if(hornbill.publish(TOPIC_NAME,payload) == 0)
  {        
    Serial.print("Publish Message:");
    Serial.println(payload);
  }
  else
  {
    Serial.println("Publish failed");
  }
}

void PublishWave(void)
{
  /* Construct the message */
  sprintf(payload,"{\"DeviceID\":WTB-IOT1, \"Value\":%d}", value); // Create the payload for publishing

  /* This will publish the message to the AWS cloud */
  PublishMsgToAWS();
  
  /* Increment or decrement the value */
  value += increment;

  /* Toggle the increment/decrement when it hits lower/upper limit respectively*/
  if(value == WAVE_LOWER_LIMIT || value == WAVE_UPPER_LIMIT)
  {
    increment *= -1;
  }

  /* 10 Sec delay between messages */
  delay(2000);
}

void PublishSerial(void)
{
  char message[50];
  memset(message, 0x00, 50);

  /* User input via serial port 2 
     Messege is accepted when enter is pressed */
  if(Serial2.available()) 
  {
    /* Read the serial port until 'enter' is pressed */
    Serial2.readBytesUntil('\r', message, 50);
    Serial2.println(message);

    /* Construct the message for publishing */
    sprintf(payload,"{\"DeviceID\":WTB-IOT1, \"Value\":%s}", message);

    /* This will publish the message to the AWS cloud */
    PublishMsgToAWS();

    /* Flush the serial ports */
    Serial.flush();
    Serial2.flush();
  }
}

void PublishMsgFromMCU(void)
{
  int msgReceived = 0;
  char message[50];
  
  /* Receive data from MCU and print on terminal */
  memset(message, '\0', 50);
  while(Serial2.available() != 0) 
  {
    msgReceived = 1;
    /* Read the serial port until 'enter' is pressed */
    Serial2.readBytes(message, 50);
  }

  //Serial.flush();
  //Serial1.flush();

  if(msgReceived)
  {
    msgReceived = 0;
    
    /* Construct the message for publishing */
    sprintf(payload,"{\"DeviceID\":WTB-IOT1, \"Value\":%s}", message);

    /* This will publish the message to the AWS cloud */
    PublishMsgToAWS();
    
    Serial.print("Received from MCU: ");
    Serial.println(message);
    
    LED_Blink();
  }

  //delay(1000);
}

void CheckMsgFromAWS(void)
{
  if(msgReceived == 1)
  {
    msgReceived = 0;
    Serial.print("Received Message:");
    Serial.println(rcvdPayload);
    StaticJsonDocument<256> doc;
    deserializeJson(doc, rcvdPayload);
    
    /* Test if parsing succeeds. */
    if (doc.isNull()) { 
      Serial.println("parseObject() failed");
      return;
    }
  
    int power = doc["state"]["desired"]["power"];

    /* Toggle LED based on the shadow update */
    Serial.println(power);
    if(power == 1)
      digitalWrite(LED_PIN, HIGH);
    else if(power == 0)
        digitalWrite(LED_PIN, LOW);
  updateShadow(power);  
  }  
}

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}

void updateShadow (int power)
{ 
  sprintf(reportpayload,"{\"state\": {\"reported\": {\"power\": \"%d\"}}}",power);
  delay(3000);   
    if(shadow.publish(SHADOW_UPDATE,reportpayload) == 0)
      {       
        Serial.print("Publish Message:");
        Serial.println(reportpayload);
      }
    else
      {
        Serial.println("Publish failed");
        Serial.println(reportpayload);   
      }  
} 

void LED_Blink(void)
{
  int i;

  for(i=0; i<10; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
}