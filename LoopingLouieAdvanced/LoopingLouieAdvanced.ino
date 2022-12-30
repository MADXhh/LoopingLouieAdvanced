/*
* software for motor control which can be controlled via WiFi 
*
*/

// Includes 
#include <ESP8266WebServer.h>

// Defines
#define ROUNDTIME  500 // milli secs
#define BREAKTIME  200 // milli secs
#define MINIMUM_SPEED     30
#define STEP_SIZE         30
#define NEW_SPEED_PROABILITY           12
#define DROP_PROABILITY                 6
#define REVERSE_DIRECTION_PROABILITY    6
#define HOLD_PROABILITY               100 - (NEW_SPEED_PROABILITY + DROP_PROABILITY + REVERSE_DIRECTION_PROABILITY)
#define CLOCKWISE 1
#if CLOCKWISE
#define OUT_PIN_A 14
#define OUT_PIN_B 12
#else
#define OUT_PIN_A 12
#define OUT_PIN_B 14
#endif


// Type defs
enum STATE{
  HOLD_SPEED = 0,
  NEW_SPEED,
  REVERSE_DIRECTION,
  HOLD_REVERSE,
  BREAK,
  DROP
  };

// Globals and consts
STATE state = HOLD_SPEED;
uint32_t timer = 0;
bool isTimerRunning = false;
int speedPercent = 50; // (-100%...100%) positive values clockwise, negative values counterclockwise 

const char* ssid = "Tower";  // SSID
//const char* pass = "loopinglouie"; // must be >= 8 characters

IPAddress ip(192,168,4,1); // should be 192.168.4.x
IPAddress gateway(192,168,4,1);  // should be 192.168.4.x
IPAddress subnet(255,255,255,0);
//ESP8266WebServer server(80);

String state_str[] = {"HOLD_SPEED", "NEW_SPEED", "REVERSE_DIRECTION", "HOLD_REVERSE", "BREAK", "DROP"};

// Functions
void SetPWM(int speedPercent);
bool IsTimerExpired();
void SetTimer(uint32_t millisecs);
void SetSeed();



void setup() {
  pinMode(OUT_PIN_A, OUTPUT);
  pinMode(OUT_PIN_B, OUTPUT);
  Serial.begin(115200);
  Serial.println(" ");
  delay(10);
  Serial.println(" ");
  Serial.println("start");

  //WiFi.softAPConfig(ip, gateway, subnet); 
  //WiFi.softAP(ssid);
  delay(500);
  
  SetSeed();
}

void loop() {
  //Serial.println(state_str[state]);
  switch(state)
  {
    case (HOLD_SPEED):
    {
      if(!isTimerRunning)
      {
        SetTimer(ROUNDTIME);
      } 
    }
    break;
    
    case (NEW_SPEED):
    {
      speedPercent = (int)random(MINIMUM_SPEED, 100);
      SetPWM(speedPercent);
    }
    break;
    
    case (REVERSE_DIRECTION):
    {
      SetPWM(0);
      delay(BREAKTIME);
      speedPercent = (int) -(random(60, 100));
      SetPWM(speedPercent);
    }
    break;
    
    case (HOLD_REVERSE):
    {
      if(!isTimerRunning)
      {
        SetTimer(random(400, 800));
      }
    }
    break;
    
    case (BREAK):
    {
      SetPWM(0);
      delay(BREAKTIME);   
    }
    break;

    case (DROP):
    {      
      if(!isTimerRunning)
      {
        SetPWM(0);
        SetTimer(1200);
      }
    }
    break;
  }

  switch(state)
  {
    case (HOLD_SPEED):
    {
      if(IsTimerExpired())
      {
        int randomNumber = (int)random(0, 100);
        if (randomNumber < HOLD_PROABILITY)
        {
          state = HOLD_SPEED;  
        }
        else if (randomNumber < (HOLD_PROABILITY + NEW_SPEED_PROABILITY))
        {
          state = NEW_SPEED;
        }
        else if (randomNumber < (HOLD_PROABILITY + NEW_SPEED_PROABILITY + DROP_PROABILITY))
        {
          state = DROP;
        }
        else
        {
          state = REVERSE_DIRECTION;
        }
      }
    }
    break;
    
    case (NEW_SPEED):
    {
      state = HOLD_SPEED;
    }
    break;
    
    case (REVERSE_DIRECTION):
    {
      state = HOLD_REVERSE; 
    }
    break;
    
    case (HOLD_REVERSE):
    {
      if(IsTimerExpired())
      {
        state = BREAK;
      }
    }
    break;
    
    case (BREAK):
    {
      state = NEW_SPEED;
    }
    break;

    case (DROP):
    {
      if(IsTimerExpired())
      {
        state = NEW_SPEED;
      }
    }
    break;

  }
}


void SetPWM(int speedPercent)
{
  int direction = -1;
  if(0 < speedPercent)
  {
    direction = 1;
  }
  int pwmValue = map(direction * speedPercent, 0, 100, 0, 255);

  if(0 < speedPercent) {
    analogWrite(OUT_PIN_A, pwmValue);
    digitalWrite(OUT_PIN_B, LOW);
  } 
  else if(0 == speedPercent)
  {
    digitalWrite(OUT_PIN_A, LOW);
    digitalWrite(OUT_PIN_B, LOW);
  }
  else
  {
    digitalWrite(OUT_PIN_A, LOW);
    analogWrite(OUT_PIN_B, pwmValue);
  }

  Serial.print("speed: ");
  Serial.print(speedPercent);
  Serial.print(" pwm: ");
  Serial.println(pwmValue);
}
void SetSeed()
{
  unsigned long rnd_seed;
  int adcValue;
  int i;
  for( i=0; i<32; i+=4)
  {
    adcValue = analogRead(0);
    rnd_seed = (adcValue & 0x0F) << i;
    delay(10);
    Serial.println(adcValue);
    //Serial.println(rnd_seed);
  }

  randomSeed(rnd_seed);
  Serial.print("seed setted: ");
  Serial.println(rnd_seed);
}

void SetTimer(uint32_t millisecs)
{
  isTimerRunning = true;
  timer = millis() + millisecs;
}

bool IsTimerExpired()
{
  if(millis() > timer)
  {
    isTimerRunning = false;
    return true;
  }
  return false;
}
