
/* This sketch will connect to the AWS Cloud using WiFi and publish a message.
 *  it will also toggle the LED on ESP32 from the AWS Cloud 
 */

/* LED Pin */
#define LED_PIN   23

void setup()
{
  /* Pin conected to LED */
  pinMode(LED_PIN, OUTPUT);
  
  /* Initialization of serial ports */
  Serial_Init();

  Serial.println("**** ATmega32u4 - ESP32 Interconnect Test *****");
  Serial.println("-----------------------------------------------");
}

char message[50];

void loop()
{
  int msgReceived = 0;
  
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
    Serial.print("Received from MCU: ");
    Serial.println(message);
    LED_Blink();
  }

  //delay(1000);
}

void Serial_Init(void)
{
  /* Serial port for logging */
  Serial.begin(115200);
#if 0
  while (!Serial) {
   ; // wait for serial port to connect. Needed for native USB port only
  }
#else
    /* This delay is given so that we can open the Serial terminal
     to see the prints */
  delay(5000);
  Serial.println("\n--- RESET ---\n");
  Serial.println("Serial0 OK!");
#endif

  // Open serial port 1 communications and wait for port to open:
  Serial2.begin(115200);
#if 0
  while (!Serial2) {
    ;
  }
  delay(5000);
#endif
  Serial.println("\nSerial2 OK!\n");
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
