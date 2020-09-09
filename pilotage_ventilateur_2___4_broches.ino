#include <LiquidCrystal.h>

#define FAN_WIRES   2
#define PWM_PIN     9
#define SENSOR_PIN  2


#if FAN_WIRES == 4
volatile unsigned int revs;
#endif

LiquidCrystal LCD(12,11,5,4,3,2);

void setup()
{
  Serial.begin(115200);
  Serial.println("FAN CONTROL");
  LCD.begin(16,2);
  LCD.setCursor(0,0);
  LCD.print("Programme :");
  LCD.setCursor(0,1);
  LCD.print("pilotage ventilateur");
  delay(500);
  LCD.scrollDisplayLeft();
  delay(2000);
  LCD.clear();

  // prescaller for 31.372KHz
  TCCR2B &= B11111000;
  TCCR2B |= (0 << CS22) | (0 << CS21) | (1 << CS20);
  digitalWrite(PWM_PIN, LOW);
  pinMode(PWM_PIN, OUTPUT);
#if FAN_WIRES == 4
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), interruptHandler, FALLING);
#endif
}

void loop()
{
  
  int pwm, pwmGpio;
  

  for (pwm = 0 ; pwm <= 100 ; pwm += 10) {
    LCD.setCursor(0,0);
    LCD.print("alimentation ->:");
    Serial.print(pwm);
    Serial.print("%: (");
    pwmGpio = pwm * 2.5;
    Serial.print(pwmGpio);
    Serial.print(") ");
    LCD.setCursor(0,1);
    LCD.print(pwm);
    LCD.print("% at ");
    LCD.print(pwmGpio);
    analogWrite(PWM_PIN, pwmGpio);
    delay(4000);
    LCD.clear();
    Serial.println();
  }
  for (pwm = 90 ; pwm >= 0 ; pwm -= 10) {
    LCD.setCursor(0,0);
    LCD.print("alimentation <-:");
    Serial.print(pwm);
    Serial.print("%: (");
    pwmGpio = pwm * 2.5;
    Serial.print(pwmGpio);
    Serial.print(") ");
    LCD.setCursor(0,1);
    LCD.print(pwm);
    LCD.print("% at ");
    LCD.print(pwmGpio);
    analogWrite(PWM_PIN, pwmGpio);
    delay(4000);
    Serial.println();
    LCD.clear();
  }
}
