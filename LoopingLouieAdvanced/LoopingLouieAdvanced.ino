/*
* software for motor control which can be controlled via WiFi 
*
*/

// Includes 
//#include <ESP8266WebServer.h>

// Defines
#define ROUNDTIME       1000 // milli secs
#define BREAKTIME       200 // milli secs
#define ACCELERATETIME  100 // milli secs
#define MINIMUM_PWM     100
#define NUMBER_OF_SPEEDS   5
#define NEW_SPEED_PROABILITY           30
#define DROP_PROABILITY                 4
#define REVERSE_DIRECTION_PROABILITY    15
#define HOLD_PROABILITY               100 - (NEW_SPEED_PROABILITY + DROP_PROABILITY + REVERSE_DIRECTION_PROABILITY)
//#define CLOCKWISE 1
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
  HOLD_DROP,
  ACCELERATE
  };

enum DIRECTION{
  CLOCKWISE,
  COUNTERCLOCKWISE
};

// Globals and consts
STATE state = NEW_SPEED;
STATE stateAfterBreak = NEW_SPEED;
STATE stateAfterAccelerate = HOLD_SPEED;
uint32_t timer = 0;
bool isTimerRunning = false;
DIRECTION pwmDirection = CLOCKWISE;
int pwmSpeedValues[NUMBER_OF_SPEEDS] = {0, MINIMUM_PWM};
int pwmSpeedIndex = 1;
int accelerateTargetIndex = 1;
DIRECTION accelerateTargetDirection = CLOCKWISE;

//const char* ssid = "Tower";  // SSID
//const char* pass = "loopinglouie"; // must be >= 8 characters

//IPAddress ip(192,168,4,1); // should be 192.168.4.x
//IPAddress gateway(192,168,4,1);  // should be 192.168.4.x
//IPAddress subnet(255,255,255,0);
//ESP8266WebServer server(80);

String state_str[] = {"HOLD_SPEED", "NEW_SPEED", "REVERSE_DIRECTION", "HOLD_REVERSE", "BREAK", "HOLD_DROP", "ACCELERATE"};

// Functions
void SetPWM(DIRECTION direction, int index);
bool IsTimerExpired();
void SetTimer(uint32_t millisecs);
void SetSeed();
void ToggleBuiltInLed();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUT_PIN_A, OUTPUT);
  pinMode(OUT_PIN_B, OUTPUT);
  Serial.begin(115200);
  Serial.println(" ");
  delay(10);
  Serial.println(" ");
  Serial.println("start");

  //WiFi.softAPConfig(ip, gateway, subnet); 
  //WiFi.softAP(ssid);
//  delay(500);
  for(int i = 1; i < NUMBER_OF_SPEEDS; i++)
  {
    pwmSpeedValues[i] = map(i, 1, NUMBER_OF_SPEEDS-1, MINIMUM_PWM, 0xFF);
  }
  SetSeed();
}

void loop() {
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
      accelerateTargetIndex = random(1, NUMBER_OF_SPEEDS);
      accelerateTargetDirection = CLOCKWISE;
    }
    break;
    
    case (REVERSE_DIRECTION):
    {
      SetPWM(COUNTERCLOCKWISE, 1);
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
      if(pwmSpeedIndex != accelerateTargetIndex && IsTimerExpired()){
        SetPWM(pwmDirection, pwmSpeedIndex - 1);
        if(pwmSpeedIndex != accelerateTargetIndex) {
          SetTimer(BREAKTIME);
        }
        else
        {
          SetTimer(BREAKTIME * 3);
        }
      }
    }
    break;

    case (HOLD_DROP):
    {      
      if(!isTimerRunning)
      {
        SetTimer(500);
      }
    }
    break;
    
    case (ACCELERATE):
    {      
      if(pwmSpeedIndex != accelerateTargetIndex && IsTimerExpired()){
        SetPWM(accelerateTargetDirection, pwmSpeedIndex + 1);
        SetTimer(ACCELERATETIME);
      }
    }
    break;
  }

  STATE oldState = state;
  switch(state)
  {
    case (HOLD_SPEED):
    {
      if(IsTimerExpired())
      {
        int randomNumber = (int)random(0, 101);
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
          accelerateTargetIndex = 0;
          stateAfterBreak = HOLD_DROP;
          state = BREAK;
        }
        else
        {
          accelerateTargetIndex = 0;
          stateAfterBreak = REVERSE_DIRECTION;
          state = BREAK;
        }
      }
    }
    break;
    
    case (NEW_SPEED):
    {
      if(pwmSpeedIndex < accelerateTargetIndex)
      {
        stateAfterAccelerate = HOLD_SPEED;
        state = ACCELERATE;
      }
      else if (pwmSpeedIndex == accelerateTargetIndex)
      {
        state = HOLD_SPEED;
      }
      else
      {
        stateAfterBreak = HOLD_SPEED;
        state = BREAK;
      }
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
        stateAfterBreak = NEW_SPEED;
        state = BREAK;
      }
    }
    break;
    
    case (BREAK):
    {
      if(pwmSpeedIndex == accelerateTargetIndex && IsTimerExpired()){
        state = stateAfterBreak;
      }
    }
    break;

    case (HOLD_DROP):
    {
      if(IsTimerExpired())
      {
        state = NEW_SPEED;
      }
    }
    break;

    case (ACCELERATE):
    {      
      if(pwmSpeedIndex == accelerateTargetIndex && IsTimerExpired()){
        state = stateAfterAccelerate;
      }
    }
    break;

  }
  if(oldState != state)
  {
    Serial.println(state_str[(int)state]);
  }
}

void SetPWM(DIRECTION direction, int index)
{
  pwmDirection = direction;
  pwmSpeedIndex = index;
  int pwmValue = pwmSpeedValues[index];;

  if(direction == CLOCKWISE) {
    analogWrite(OUT_PIN_A, pwmValue);
    digitalWrite(OUT_PIN_B, LOW);
  } 
  else if(0 == index)
  {
    digitalWrite(OUT_PIN_A, LOW);
    digitalWrite(OUT_PIN_B, LOW);
  }
  else
  {
    digitalWrite(OUT_PIN_A, LOW);
    analogWrite(OUT_PIN_B, pwmValue);
  }

  Serial.print("index: ");
  if(direction == COUNTERCLOCKWISE)
  {
    Serial.print("-");
  }
  Serial.print(index);
  Serial.print(" pwm: ");
  Serial.println(pwmValue);

//  ToggleBuiltInLed();
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
    delay(50);
    Serial.println(adcValue);
    ToggleBuiltInLed();
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

void ToggleBuiltInLed()
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
}
