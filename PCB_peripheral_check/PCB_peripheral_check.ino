
/* This code checked the peripherals on the PCB */

#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "Adafruit_DRV2605.h"

/* To enable SD Card and BLE */
#define ENABLE_SDCARD   0
#define ENABLE_BLE      0

/* Define to enable debug prints from individial component  */
#define ENABLE_RTC_DEBUG_PRINT        1
#define ENABLE_FSR_DEBUG_PRINT        1
#define ENABLE_HAPTIC_DEBUG_PRINT     1
#define ENABLE_SD_CARD_DEBUG_PRINT    1
#define ENABLE_BLE_DEBUG_PRINT        1
#define ENABLE_BATT_MON_DEBUG_PRINT   1

/* Pin Mappings */
#define LED_STATUS_1_PIN     7   /* Digital Pin 7   */
#define LED_STATUS_2_PIN     12  /* Digital Pin 12  */   
#define FSR_PIN              A4  /* Analog Pin 4    */   
#define SD_CARD_CS_PIN       SS  /* SPI chip select */
#define BLE_ENABLE_PIN       11  /* Digital Pin 11  */
#define USR_BLE_ENABLE_PIN   4   /* Digital Pin 4   */
#define BATT_MON_ENABLE_PIN  A1  /* Analog Pin 1    */
#define BATT_ADC_PIN         A5  /* Analog Pin 5    */
#define BATT_CHARGE_PIN      10  /* Digital Pin 10  */

/* This is the value from the FSR analog read which will trigger the Haptic */
#define HAPTIC_FSR_THRESHOLD    900

/* RTC instance */
RTC_PCF8523 rtc;

/* Object for haptic control */
Adafruit_DRV2605 drv;

/* Chosen from Adafruit_DRV2605 library */
const int hapticEffect = 1;

/* To store the FSR Data */
int fsrData = 0;

#if ENABLE_BLE
/* BLE configuration via User button */
bool userBleEnabled = 0;
bool BleConfigured = 0;
#endif

/* Make a string for assembling the data to log */
String logDataString = "";

/* Local functions */
void Serial_Init(void);

void LED1_Init(void);
void LED2_Init(void);
void LED1_Check(void);
void LED2_Check(void);

void RTC_Init(void);
void RTC_Check(void);

void FSR_Init(void);
void FSR_Check(void);

void HAPTIC_Init(void);
void HAPIC_Check(void);

#if ENABLE_SDCARD
void SDcard_Init(void);
void SDcard_Check(void);
void SDCard_Read(void);
#endif

#if ENABLE_BLE
void BLE_Init(void);
void BLE_Config(void);
void BLE_Check(void);
#endif

void Battery_Init(void);
void Battery_Check(void);

void Log_Print(void);

void setup() 
{

  /* Serial port */
  Serial_Init();

  /* LED 1 init*/
  LED1_Init();

  /* LED 2 init*/
  LED1_Init();

  /* RTC init*/
  RTC_Init();
  
  /* Force Sensor init*/
  FSR_Init();

  /* Haptic Init */
  HAPTIC_Init();
  
#if ENABLE_SDCARD
  /* SD Card Init */
  SDcard_Init();
#endif

#if ENABLE_BLE
  /* Bluetooth Init */
  BLE_Init();
#endif

  /* Battery monitor Init */
  Battery_Init();

  /* Instrucions to the user */
  Serial.println();
  Serial.println("## Apply pressure on Force Sensor to get haptick feedback.");
  Serial.println("## Press BLE enable button to configure Bluetooth.");
  Serial.println("## Press any key to on the serial terminal to read data from SD Card.");
  Serial.println();
}

void loop() 
{
  LED1_Check();

  LED2_Check();

  RTC_Check();

  FSR_Check();

  HAPTIC_Check();

#if ENABLE_SDCARD
  SDcard_Check();
  SDCard_Read();
#endif

#if ENABLE_BLE
  BLE_Check();
#endif

  Battery_Check();

  /* Print the looged string onto the serial terminal */
  Log_Print();

  /* Clear the log string after each loop iteration */
  logDataString = "";

  /* Empty line to space logs from each itration */
  Serial.println();

  //delay(1000);
}

/********************************/
/**** INITIALIZATION FUNCTION ***/
/********************************/

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
#endif


  // Open serial port 1 communications and wait for port to open:
  Serial1.begin(115200);
#if 0
  while (!Serial1) {
    ;
  }
#else
    /* This delay is given so that we can open the Serial terminal
     to see the prints */
  delay(5000);
  Serial1.println("\n--- Serial Port Initialized ---\n");
#endif
}

void LED1_Init(void)
{
  /* Initializing as INPUT as it is a dual LED */
  pinMode(LED_STATUS_1_PIN, INPUT);
  Serial.println("LED 1 Initialization...done!");
}

void LED2_Init(void)
{
  /* LED Status 2 */
  pinMode(LED_STATUS_2_PIN, OUTPUT);
  Serial.println("LED 2 Initialization...done!");
}

void RTC_Init(void)
{
  if (!rtc.begin()) {
    Serial.print("RTC not found!");
    while (1);
  }
  else {
    Serial.print("RTC found and ");
  }

  if (!rtc.initialized()) {
    Serial.print("not running... ");
  }else {
    Serial.print("running...");
  }
  
  Serial.println("Initialized to system time!");
  /* Following line sets the RTC to the date & time from system time */
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void FSR_Init(void)
{
  /* Force Sensor */
  pinMode(FSR_PIN, INPUT);
  Serial.println("FSR Initialization...done!");
}

void HAPTIC_Init(void)
{
  Serial.print("HAPTIC Initialization...");

  drv.begin();  
  drv.selectLibrary(1);
  
  /* I2C trigger by sending 'go' command
     default, internal trigger when sending GO command */
  drv.setMode(DRV2605_MODE_INTTRIG);
  
  /* play effect  */
  drv.setWaveform(0, hapticEffect);
  
  /* end waveform  */
  drv.setWaveform(1, 0);

  Serial.println("done!");
}

#if ENABLE_SDCARD
void SDcard_Init(void)
{
  Serial.print("SD Card Initializing...");

  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CARD_CS_PIN)) {
    Serial.println("Card failed, or not present!");
    // don't do anything more:
    while (1);
  }

  /* Delete the existing file on SD Card */
  Serial.print("Deleting the existing log file...");
  SD.remove("WTB_log.txt");

  Serial.println("done!");
}
#endif

#if ENABLE_BLE
void BLE_Init(void)
{
  /* Reset BLE configuration flags */
  userBleEnabled = 0;
  BleConfigured = 0;

  /* User BLE enable button */
  /* User will press this button to enable the Bluetooth */
  pinMode(USR_BLE_ENABLE_PIN, INPUT);

  /* Reset Bluetooth and keep it in reset */
  pinMode(BLE_ENABLE_PIN, OUTPUT);
  digitalWrite(BLE_ENABLE_PIN, LOW);
  
  Serial.println("BLE Initializing...done!");
}
#endif

void Battery_Init(void)
{
  /* Enable battery monioring */
  pinMode(BATT_MON_ENABLE_PIN, OUTPUT);
  digitalWrite(BATT_MON_ENABLE_PIN, HIGH);

  /* Battery ACD input */
  pinMode(BATT_ADC_PIN, INPUT);

  /* Battery charging status */
  pinMode(BATT_CHARGE_PIN, INPUT);

  Serial.println("Battery Monitoring...enabled!");
}


/********************************/
/******* CHECKING FUNCTION ******/
/********************************/

void LED1_Check(void)
{
  /* Yellow LED */
  pinMode(LED_STATUS_1_PIN, OUTPUT);
  digitalWrite(LED_STATUS_1_PIN, LOW);
  delay(200);
  pinMode(LED_STATUS_1_PIN, INPUT);
  delay(200);
  
  /* Green LED */
  pinMode(LED_STATUS_1_PIN, OUTPUT);
  digitalWrite(LED_STATUS_1_PIN, HIGH);
  delay(200);
  pinMode(LED_STATUS_1_PIN, INPUT);
  delay(200);
}

void LED2_Check(void)
{
  /* Blue LED */
  digitalWrite(LED_STATUS_2_PIN, HIGH);
  delay(200);
  digitalWrite(LED_STATUS_2_PIN, LOW);
  delay(200);
}

void RTC_Check(void)
{
  String rtcData = "";
  
  /* Fetch current time from RTC */
  DateTime now = rtc.now();

  /* Construct the Date */
  rtcData = String(now.year());

  rtcData += "-";
  if(now.month()<10)
    rtcData += "0";
  rtcData += String(now.month());

  rtcData += "-";
  if(now.day()<10)
    rtcData += "0";
  rtcData += String(now.day());

  rtcData += " ";

  /* Construct the Time */
  if(now.hour()<10)
      rtcData += "0";
  rtcData += String(now.hour());

  rtcData += ":";
  if(now.minute()<10)
      rtcData += "0";
  rtcData += String(now.minute());

  rtcData += ":";
  if(now.second()<10)
    rtcData += "0";
    rtcData += String(now.second());

  rtcData += ", ";

#if ENABLE_RTC_DEBUG_PRINT
  Serial.print("RTC Check: ");
  Serial.println(rtcData);
#endif

  /* Add RTC data to the log */
  logDataString += rtcData; 
}

void FSR_Check(void)
{
  fsrData = 0;
  
  /* Analog reading from FSR */
  fsrData = analogRead(FSR_PIN);

#if ENABLE_FSR_DEBUG_PRINT  
  Serial.print("FSR Check: ");
  Serial.println(fsrData);
#endif

  /* Add FSR data to the log */
  logDataString += String(fsrData);
}

void HAPTIC_Check(void)
{
  /* Buzz if FSR reading is more that a threshold value */
  if(fsrData >= HAPTIC_FSR_THRESHOLD)
  {
#if ENABLE_HAPTIC_DEBUG_PRINT
    Serial.println("HAPTIC Check: Buzzzzzzttt....!! ");
#endif

    /* Add Haptic incidence to be logged */
    logDataString += " Buzzzzzzttt....!! "; 
    
    /* Send the log string over Serial Port 1 */
    Serial1.println(logDataString);
    
    /* Trigger Haptic */
    drv.go();
    
    /* Blink the ember LED to indicate Buzz */
    for(int i = 0; i < 10; i++)
    {
      pinMode(LED_STATUS_1_PIN, OUTPUT);
      digitalWrite(LED_STATUS_1_PIN, LOW);
      delay(20);
      pinMode(LED_STATUS_1_PIN, INPUT);
      delay(20);
    }
  }
  else
  {
#if ENABLE_HAPTIC_DEBUG_PRINT
    Serial.println("HAPTIC Check: FSR reading below threshold");
#endif
  }
}

#if ENABLE_SDCARD
void SDcard_Check(void)
{
  /* Open the file. Note that only one file can be open at a time */
  File dataFile = SD.open("WTB_log.txt", FILE_WRITE);
 
  /* If the file is available, write to it */
  if (dataFile) {
    dataFile.println(logDataString);
    dataFile.close();

#if ENABLE_SD_CARD_DEBUG_PRINT
    /* Print to serial terminal */
    Serial.print("SD Card Check: ");
    Serial.println(logDataString);
#endif
  }
  else {
#if ENABLE_SD_CARD_DEBUG_PRINT
    Serial.println("error opening WTB_log.txt");
#endif
  }
}

void SDCard_Read(void)
{
  /* CHeck if user has pressed any key */
  if(Serial.available())
  {
    while (Serial.read() >= 0)
    ; /* This is to flush the Seial buffer in case multiple key press received */
    
    Serial.println("\n-- Reading data from SD Card --");

    File dataFile = SD.open("WTB_log.txt");
  
    if (dataFile) {
      Serial.println("-- Start of File ---");
      while (dataFile.available()) {
        Serial.write(dataFile.read());
      }
      dataFile.close();
      Serial.println("-- End of File ---");
      Serial.println("Press any key to read data from SD Card\n");
    }
    else {
      Serial.println("Error opening logfile (WTB_log.txt) !!");
    }
  }
}
#endif

#if ENABLE_BLE
void BLE_Check(void)
{
  /* Configure BLE if not already configured */
  if(!BleConfigured)
  {
    /* Configure Bluetooth if user has pressed BLE configure button */
    if(digitalRead(USR_BLE_ENABLE_PIN) == LOW)
    {
      //Serial.println("BLE enabled by user!");
      userBleEnabled = 1;
    }
  
    /* Dont reconfigure if BLE is already configured once */
    if(userBleEnabled == 1)
    {
      userBleEnabled = 0;
      BLE_Config();
      BleConfigured = 1;
      for(int i = 0; i < 10; i++)
      {
        digitalWrite(LED_STATUS_2_PIN, HIGH);
        delay(20);
        digitalWrite(LED_STATUS_2_PIN, LOW);
        delay(20);
      }
    }
  }
  /* Print to BLE only if it is configured */
  if(BleConfigured)
  {
    /* Send log to bluetooth to be transmitted */
    Serial1.println(logDataString);

#if ENABLE_BLE_DEBUG_PRINT
    /* Print to serial terminal */
    Serial.print("BLE Check: ");
    Serial.println(logDataString);
#endif
  }
}

void BLE_Config(void)
{
  Serial.print("\nBLE Configuration...");
  
  /* Enable Bluetooth, out of reset */
  digitalWrite(BLE_ENABLE_PIN, HIGH);
  delay(200);

  // Open serial port 1 communications and wait for port to open:
  Serial1.begin(115200);
  while (!Serial1) {
    ;
  }

  /* Send command "$$$" to Bluetooth */
  delay(500);
  Serial1.print("$$$");

  /* Send command "S-,WalkToBeat" to Bluetooth */
  delay(500);
  Serial1.print("S-,WTB\r");

  /* Send command "SS,C0" to Bluetooth */
  delay(500);
  Serial1.print("SS,C0\r");

  Serial.println("Done!");
  Serial.println("Please connect to Bluetooth!\n");
}
#endif

void Battery_Check(void)
{
  float measuredvbat = 0, battPlugin = 0;

  measuredvbat = analogRead(BATT_ADC_PIN);

    /* Calculate battery voltage */
  measuredvbat *= 2; // we divided by 2, so multiply back
  measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

#if ENABLE_BATT_MON_DEBUG_PRINT
  Serial.print("Battery Monitoring Check: Battery Reading (V)  ");
  Serial.println(measuredvbat);
#endif
  battPlugin = digitalRead(BATT_CHARGE_PIN);

  /* Switch on the blue LED to indicate charging */
  if(battPlugin == 0)
  {
      digitalWrite(LED_STATUS_2_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_STATUS_2_PIN, LOW);
  }
}

void Log_Print(void)
{
  /* This is the final string that is logged to SD card and
     transmitted on Bluetooth */
  Serial.print("LOG String: ");
  Serial.println(logDataString);
}
