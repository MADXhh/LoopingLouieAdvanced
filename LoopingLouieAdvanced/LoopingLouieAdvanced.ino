
#define ROUNDTIME 500 // milli secs
#define BREAKTIME  300 // milli secs
#define MINIMUM_SPEED     20
#define HOLD_PROABILITY              80
#define CHANGE_SPEED_PROABILITY       8
#define CHANGE_DIRECTION_PROABILITY 100 - (HOLD_PROABILITY + 2 * CHANGE_SPEED_PROABILITY)
#define OUT_PIN_A 14
#define OUT_PIN_B 12

enum STATE{
  HOLD_SPEED = 0,
  DECREASE_SPEED,
  INCREASE_SPEED,
  CHANGE_DIRECTION,
  BREAK
  };
STATE state = HOLD_SPEED;

int speedPercent = 50; // (-100%...100%) positive values clockwise, negative values counterclockwise 

void SetPWM(int speedPercent);

void setup() {
pinMode(OUT_PIN_A, OUTPUT);
pinMode(OUT_PIN_B, OUTPUT);
Serial.begin(115200);
Serial.println("start");
}

void loop() {
  switch(state)
  {
    case (HOLD_SPEED):
    {
      SetPWM(speedPercent);
      delay(ROUNDTIME);
    }
    break;
    
    case (DECREASE_SPEED):
    {
      speedPercent -= 30;
      if (speedPercent < MINIMUM_SPEED && speedPercent > -(MINIMUM_SPEED) )
      {
        SetPWM(0);
        delay(BREAKTIME);
        speedPercent = -(MINIMUM_SPEED);
      }
      if (speedPercent < -100)
      {
        speedPercent = -100;
      }
    }
    break;
    
    case (INCREASE_SPEED):
    {
      speedPercent += 30;
      if (speedPercent < MINIMUM_SPEED && speedPercent > -(MINIMUM_SPEED) )
      {
        SetPWM(0);
        delay(BREAKTIME);
        speedPercent = MINIMUM_SPEED;
      }
      
      if (speedPercent > 100)
      {
        speedPercent = 100;
      }      
    }
    break;
    
    case (CHANGE_DIRECTION):
    {
      speedPercent *= -1;    
    }
    break;
    
    case (BREAK):
    {
      SetPWM(0);
      delay(BREAKTIME);   
    }
    break;
  }

  switch(state)
  {
    case (HOLD_SPEED):
    {
      int randomNumber = random(0, 100);
      if (randomNumber < HOLD_PROABILITY)
      {
        state = HOLD_SPEED;  
      }
      else if (randomNumber < (HOLD_PROABILITY + CHANGE_SPEED_PROABILITY) )
      {
        state = INCREASE_SPEED;
      }
      else if (randomNumber < (HOLD_PROABILITY + 2 * CHANGE_SPEED_PROABILITY))
      {
        state = DECREASE_SPEED;
      }
      else
      {
        state = BREAK;
      }
    }
    break;
    
    case (DECREASE_SPEED):
    {
      state = HOLD_SPEED;
    }
    break;
    
    case (INCREASE_SPEED):
    {
      state = HOLD_SPEED; 
    }
    break;
    
    case (CHANGE_DIRECTION):
    {
      state = HOLD_SPEED;
    }
    break;
    
    case (BREAK):
    {
      state = CHANGE_DIRECTION;
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
