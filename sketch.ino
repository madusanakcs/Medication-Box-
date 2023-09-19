#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define BUZZER 4
#define LED 2
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 12
const int LDRpin = 36;
const int servoPin = 18;

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     19800
#define UTC_OFFSET_DST 0
int Selected_UTC_OFFSET =UTC_OFFSET;

int n_notes =8;
int C =262;
int D=294;
int E=330;
int F=349;
int G=392;
int A=440;
int B=494;
int C_H =523;
int notes[] = {C,D,E,F,G,A,B,C_H};

int current_mode = 0;
int max_modes = 5;
String modes[]={"1-Set time","  2-Set     alarm 1","  3-Set     alarm 2","  4-Set     alarm 3","5-Disable   alarm"};

int days=0;
int hours=0;
int minutes =0;
int seconds =0;
String month;
String date;
String current_time;

bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[]={8,13,18};
int alarm_minutes[]={0,0,0};
bool alarm_triggered[] = {false,false,false};

char tempAr[6]="30";
char LDR_chr[6];
float LDR_val=0.0;
int minAngle;
int crlFact;
int theta=0;
bool isSchedule=false;
unsigned long scheduleOnTime;

////////////////////////////////////////

Servo servo;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


WiFiClient espClient;
PubSubClient mqttClient(espClient);

//////////////////////////////////////////


Adafruit_SSD1306 display (SCREEN_WIDTH,SCREEN_HEIGHT,&Wire, OLED_RESET);
DHTesp dhtSensor;



// 'be18908825a6de8e36cc1407b7790e36', 128x64px
const unsigned char medibox_logo [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x03, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf0, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x7f, 0xfc, 0x7f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x7f, 0xff, 0xbf, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfd, 0xff, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfd, 0xff, 0x1f, 0x7f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf8, 0xfe, 0x1e, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0xfe, 0x1c, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0xfe, 0x1c, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x7c, 0x08, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x7c, 0x49, 0x9f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe2, 0x7c, 0xc1, 0x9f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xc6, 0x3c, 0xc3, 0x9f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc7, 0x38, 0xe3, 0xcf, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x8f, 0x19, 0xe7, 0xcf, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x8f, 0x99, 0xef, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x91, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x83, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc7, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xc7, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xe7, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe7, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  // Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 1040)
  const int epd_bitmap_allArray_LEN = 1;
  const unsigned char* epd_bitmap_allArray[1] = {
    medibox_logo
  };




void setup(){




    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS)){
        Serial.println(F("SSD1306 aalocation is failed."));
        for(;;);
    }

    display.display();
    delay(1000);
    display.clearDisplay();

    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      print_line("connecting to wifi",0,0,2);
      display.display();
      display.clearDisplay();
    }
    display.clearDisplay();
    print_line("welcome to medibox",0,10,2);
    display.display();
    delay(1000);
    display.clearDisplay();

    display.drawBitmap(0,1,medibox_logo,128,64,WHITE);
    display.display();
    delay(1000);
    display.clearDisplay();

    pinMode(LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(PB_CANCEL, INPUT);
    pinMode(PB_OK, INPUT);
    pinMode(PB_UP, INPUT);
    pinMode(PB_DOWN, INPUT);
    
    dhtSensor.setup(DHTPIN,DHTesp::DHT22);
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

 /////////////////////////////////////////////////////

    servo.attach(servoPin, 500, 2400);
    setupWifi();
    setupMqtt();
    timeClient.begin();
    timeClient.setTimeOffset(5.5*3600);

//////////////////////////////////////////////////////

  }

void loop(){
    configTime(Selected_UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER); //update time if user input different UTC time zone
    update_time_with_check_alarm();
    if (digitalRead(PB_OK)==LOW){
      delay(200);
      update_time();
      go_to_menu();
    }

    check_temp();
    display.display();

/////////////////////////////////////////////////
    if (!mqttClient.connected())
    {
        connectToBroker();
    }

    LDR_val = analogRead(LDRpin)/4063.0;
    // Serial.println(LDR_val);
    updateTemp();
    // Serial.println(tempAr);
    String(LDR_val, 2).toCharArray(LDR_chr, 6);
    mqttClient.loop();
    mqttClient.publish("Temperature", tempAr);
    mqttClient.publish("Intensity", LDR_chr);
    

    checkSchedule();
    delay(1000);
    theta=minAngle + (180-minAngle)*crlFact*LDR_val;
    servo.write(theta);
    delay(50);

    Serial.println(theta);

///////////////////////////////////////////////////

  }


///////////////////////////////////////////////////////




void updateTemp(){
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    String(data.temperature, 2).toCharArray(tempAr, 6);
}

void checkSchedule(){
    if(isSchedule){
        unsigned long currentTime=getTime();
        if(currentTime>scheduleOnTime){
            buzzer_on(true);
            isSchedule=false;
            Serial.println("Scheduled ON");
            mqttClient.publish("OnOffESP", "1");
            mqttClient.publish("SchESPOn", "0");
        }
    }
}

unsigned long getTime(){
    timeClient.update();
    return timeClient.getEpochTime();
}

void buzzer_on(bool on){
    if(on){
        tone(BUZZER, 255);

            
    }else{
        noTone(BUZZER);
    }
}






void connectToBroker()
{
    while (!mqttClient.connected())
    {
        Serial.println("Attempting MQTT connect");
        if (mqttClient.connect("ESP32-4546"))
        {
            Serial.println("connected");
            mqttClient.subscribe("MinimumAngle");
            mqttClient.subscribe("ControlFactor");
            mqttClient.subscribe("Buzzer");
            mqttClient.subscribe("Switched_On");
            
        }
        else
        {
            Serial.print("failed");
            Serial.print(mqttClient.state());
            delay(5000);
        }
    }
}


void setupWifi()
{
    WiFi.begin("Wokwi-GUEST", "");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wifi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}





void setupMqtt()
{
    mqttClient.setServer("test.mosquitto.org", 1883);
    mqttClient.setCallback(receiveCallback);
}






void receiveCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arriced [");
    Serial.print(topic);
    Serial.print("] ");

    char payloadCharAr[length];
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        payloadCharAr[i] = (char)payload[i];
    }

    Serial.println();

    if (strcmp(topic, "MinimumAngle") == 0)
    {

        minAngle = atoi(payloadCharAr);
    }
    else if (strcmp(topic, "ControlFactor") == 0)
    {

        crlFact = atoi(payloadCharAr);
    }else if(strcmp(topic,"Buzzer")==0){
          buzzer_on(payloadCharAr[0]=='1'  );
        
    }else if(strcmp(topic,"Switched_On")==0){
          if(payloadCharAr[0]=='N'){
            isSchedule=false;
          }else{
            isSchedule=true;
scheduleOnTime=atol(payloadCharAr);
          }
        
    }

}











///////////////////////////////////////////////////////////

void print_line(String text, int coloumn, int row, int size){
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(coloumn,row);
  display.println(text);
  //display.display();

}

void print_time_now(void){
  display.clearDisplay();
  print_line(date,18,0,2);
  print_line(current_time,16,20,2);
  //display.display();
}

//////////////////////////////////////////////////////////////////////////////

void update_time(void){
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    char timeHour[3];
    strftime(timeHour,3,"%H",&timeinfo);
    hours = atoi(timeHour);

    char timeMinute[3];
    strftime(timeMinute,3,"%M",&timeinfo);
    minutes = atoi(timeMinute);

    char timeSecond[3];
    strftime(timeSecond,3,"%S",&timeinfo);
    seconds = atoi(timeSecond);

    char timeDays[3];
    strftime(timeDays,3,"%d",&timeinfo);
    days = atoi(timeDays);

    char timeMonth[5];
    strftime(timeMonth,5,"%b",&timeinfo);
    month = timeMonth;
    
    date = String(month) + " " + String(days) ;
    current_time = String(hours) + ":" + String(minutes) +":"+String(seconds);
  }

///////////////////////////////////////////////////////////////////////////////////////////////

void ring_alarm(){
  display.clearDisplay();
  print_line("Medicine   time",0,0,2);
  display.display();

  digitalWrite(LED, HIGH);

  bool break_happen = false;
  while ( break_happen == false && digitalRead(PB_CANCEL)==HIGH){
      display.clearDisplay();
      print_line("Medicine   time",0,0,2);
      display.display();

      digitalWrite(LED, HIGH);
      for (int i=0; i<n_notes; i++){
        //Serial.println(i);
        if (digitalRead(PB_CANCEL)==LOW){
          //Serial.println("pushed");
          break_happen = true;
          delay(1000);
          
          break;
        }
        tone(BUZZER,notes[i]);
        delay(500);
        noTone(BUZZER);
        delay(2);
      }

      delay(200);
      digitalWrite(LED, LOW);
    }
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

//ringing alarm in correct time
void update_time_with_check_alarm(){
    display.clearDisplay();
    update_time();
    print_time_now();

    if (alarm_enabled){
      
      for (int i=0; i<n_alarms; i++){
        if (alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]){
          ring_alarm();
          alarm_triggered[i]=true;
        }
      }
    } 
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////

//button press
int wait_for_button_press(){
    while (true){
      if (digitalRead(PB_UP) == LOW){
        delay(200);
        return PB_UP;
      }

      if (digitalRead(PB_DOWN) == LOW){
        delay(200);
        return PB_DOWN;
      }

      if (digitalRead(PB_OK) == LOW){
        delay(200);
        return PB_OK;
      }

      if (digitalRead(PB_CANCEL) == LOW){
        delay(200);
        return PB_CANCEL;
      }
    }
  }

///////////////////////////////////////////////////////////////////////////////////////////////////////////

//menu navigation
void go_to_menu(){
    while (digitalRead(PB_CANCEL) == HIGH){
      display.clearDisplay();
      print_line(modes[current_mode],0,20,2);
      display.display();

      int pressed = wait_for_button_press();

      if (pressed == PB_UP){
      current_mode += 1;
      current_mode %= max_modes;
      delay(200); 
      }

      else if (pressed == PB_DOWN){
        current_mode -= 1;
        if (current_mode < 0){
          current_mode = max_modes - 1;
        }
        delay(200);
      }

      else if(pressed == PB_OK){
        //Serial.println(current_mode);
        delay(200);
        run_mode(current_mode);
      }
    }
  }

/////////////////////////////////////////////////////////////////////////////////////////////////////

//running selected mode from menu
void run_mode(int mode){
      if (mode == 0){
        set_time();
      }

      else if (mode == 1 || mode == 2 || mode==3){
        set_alarm(mode -1);
      }

      else if (mode == 4){
        alarm_enabled = false;
        display.clearDisplay();
        print_line("Alarm is disabled.",0,20,2);
        display.display();
        delay(1000);
      }

    }

////////////////////////////////////////////////////////////////////////////////////////////////////////

//setting time according to UTC offset
void set_time(){

      int temp_hour = hours;
      while (true){
        display.clearDisplay();
        print_line("Enter offset hour:  "+String(temp_hour),0,0,2);
        display.display();

        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
          delay(200);
          temp_hour +=1;
          temp_hour = temp_hour %15;
        }

        else if (pressed == PB_DOWN){
          delay(200);
          temp_hour -=1;
          if (temp_hour <-12){
            temp_hour =0;
          }
        }

        else if (pressed == PB_OK){
          delay(200);
          hours = temp_hour;
          break;
        }

        else if (pressed == PB_CANCEL){
          delay(200);
          break;

        }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////////

      int temp_minute = minutes;
      while (true){
        display.clearDisplay();
        print_line("Enter offset mins:   "+String(temp_minute),0,0,2);
        display.display();

        int pressed = wait_for_button_press();
        if (pressed == PB_UP){
          delay(200);
          temp_minute +=1;
          temp_minute = temp_minute %60;
        }

        else if (pressed == PB_DOWN){
          delay(200);
          temp_minute -=1;
          if (temp_minute <0){
            temp_minute =59;
          }
        }

        else if (pressed == PB_OK){
          delay(200);
          minutes = temp_minute;
          break;
        }

        else if (pressed == PB_CANCEL){
          delay(200);
          break;
        }
      }
      Selected_UTC_OFFSET = hours*3600 + minutes*60;
      display.clearDisplay();
      print_line("Time is   set",0,20,2);
      display.display();
      delay(1000);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//setting alarm
void set_alarm(int alarm){
  int temp_hour = alarm_hours[alarm];
  while (true){
    display.clearDisplay();
    print_line("Enter hour: "+String(temp_hour),0,0,2);
    display.display();

    int pressed = wait_for_button_press();
    if (pressed == PB_UP){
      delay(200);
      temp_hour +=1;
      temp_hour = temp_hour %24;
    }

    else if (pressed == PB_DOWN){
      delay(200);
      temp_hour -=1;
      if (temp_hour <0){
        temp_hour =23;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      hours = temp_hour;
      alarm_hours[alarm] = hours;
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;

    }
  }


  ///////////////////////////////////////////////////////////////////////////////////////////////////////

  int temp_minute = alarm_minutes[alarm];
  while (true){
    display.clearDisplay();
    print_line("Enter minute: "+String(temp_minute),0,0,2);
    display.display();

    int pressed = wait_for_button_press();
    if (pressed == PB_UP){
      delay(200);
      temp_minute +=1;
      temp_minute = temp_minute %60;
    }

    else if (pressed == PB_DOWN){
      delay(200);
      temp_minute -=1;
      if (temp_minute <0){
        temp_minute =59;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      minutes = temp_minute;
      alarm_minutes[alarm] = minutes;
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  print_line("Alarm is set",0,20,2);
  display.display();
  delay(1000);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////

//temp and humidity checking
void check_temp(){
      TempAndHumidity data = dhtSensor.getTempAndHumidity();
      if (data.temperature > 32 && data.humidity > 80){
        display.clearDisplay();
        print_line(current_time,16,0,2);
        print_line("Temp high",10,20,2);
        print_line("HUM high",16,40,2);
      }
      else if(data.temperature < 26 && data.humidity <60 ){
        display.clearDisplay();
        print_line(current_time,16,0,2);
        print_line("Temp low",12,20,2);
        print_line("HUM low",16,40,2);
      }
      else if(data.temperature > 32 && data.humidity <60){
        display.clearDisplay();
        print_line(current_time,16,0,2);
        print_line("Temp high",10,20,2);
        print_line("HUM low",16,40,2);
      }
      else if(data.temperature < 26 && data.humidity > 80 ){
        display.clearDisplay();
        print_line(current_time,16,0,2);
        print_line("Temp low",16,20,2);
        print_line("HUM high",16,40,2);
      }
      else{
        if (data.temperature > 32){
        //display.clearDisplay();
        print_line("Temp high",10,40,2);
        //display.display();
        }

        if (data.temperature < 26){
          //display.clearDisplay();
          print_line("Temp low",16,40,2);
          //display.display();
        }

        if (data.humidity > 80){
          //display.clearDisplay();
          print_line("HUM high",16,40,2);
          //display.display();
        }

        if (data.humidity <60){
          //display.clearDisplay();
          print_line("HUM low",16,40,2);
          //display.display();
      }
      }
      
    }
