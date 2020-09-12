#include <LiquidCrystal.h>
#include <LibPrintf.h>

#define PWM_PIN     9
#define SENSOR1 A0
#define SENSOR2 A1


LiquidCrystal LCD(12,11,5,4,3,2);

void setup()
{ 
  Serial.begin(9600);
  Serial.println("FAN CONTROL");
  LCD.begin(16,2);
  LCD.setCursor(0,0);
  LCD.print("Programme :");
  LCD.setCursor(0,1);
  LCD.print("Pilotage ventilo");
  delay(1000);
  LCD.clear();

  TCCR1B = TCCR1B & B11111000 | B00000101; //permet le réglage de la fréquence du PWM le mieux pour le ventilateur Nidec ultraflo est de se raprocher de la fréquence du courant qui est normalement en sortie d'une alimentation
  
  digitalWrite(PWM_PIN, LOW);
  pinMode(PWM_PIN, OUTPUT);

  LCD.setCursor(0,1);
  LCD.print("Lancement 100%");
  analogWrite(PWM_PIN,254);
  delay(2000);
  LCD.clear();
  analogWrite(PWM_PIN,0);
}

float CalculTempLM61(int ra){
  float temp;
  
  Serial.println("calcul temp via LM61");
  Serial.println(ra);
  temp = 5;
  temp /= 1023;
  temp *= ra;
  temp *= 1000;
  temp -= 600;
  temp /= 10;
  temp-=0.5;
  //je soustrais 5°C car différence avec les valeurs prélevée par 3 autres appareils complètement indépendant,
  //cela est peut être du au fait que le capteur soit dans une coque.
  Serial.print(temp,1);
  Serial.println("°C");
  Serial.println();
  return temp;
  }

float CalculTempLM19(int ra){
  float temp;
  Serial.println("calcul temp via LM19");
  Serial.print("Valeur lu : ");
  Serial.println(ra);
  temp = 5;
  temp /= 1023;
  temp *= ra;
  temp = 1.8639 - temp;
  temp *= pow(10,6);
  temp /= 3.88;
  temp += 2.1962*pow(10,6);
  temp = sqrt(temp);
  temp -= 1481.96;

  Serial.println(temp,1);
  return temp;
  }


void AffichageTemp(float temp1,float temp2){
  Serial.println();
  Serial.println();
  printf("T1 : %f; T2: 1%f",temp1,temp2);
  LCD.setCursor(0,1);
  LCD.print("T1:");
  LCD.print(temp1,1);
  LCD.print("  T2:");
  LCD.print(temp2,1);
  }

void loop()
{
  int pwm, pwmGpio;
  float temp_C1,temp_C2, moyTemp;
  
  
  for (pwm = 35 ; pwm <= 100 ; pwm += 10) {
    temp_C1 = CalculTempLM61(analogRead(SENSOR1));
    temp_C2 = CalculTempLM61(analogRead(SENSOR2));
    
    moyTemp = temp_C1+temp_C2;
    moyTemp /= 2;
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");

    
    Serial.print(pwm);
    Serial.print("%: (");
    pwmGpio = pwm * 2.54;
    Serial.print(pwmGpio);
    Serial.print(") ");
    LCD.setCursor(0,0);
    LCD.print(" Alim ==> ");
    LCD.print(pwm);
    LCD.print("%");
    AffichageTemp(temp_C1,temp_C2);
    analogWrite(PWM_PIN, pwmGpio);
    delay(850);
    Serial.println();
  }
  for (pwm = 100 ; pwm >= 35 ; pwm -= 10) {
    temp_C1 = CalculTempLM61(analogRead(SENSOR1));
    temp_C2 = CalculTempLM61(analogRead(SENSOR2));
    
    moyTemp = temp_C1+temp_C2;
    moyTemp /= 2;
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");
    Serial.print(pwm);
    Serial.print("%: (");
    pwmGpio = pwm * 2.54;
    Serial.print(pwmGpio);
    Serial.print(") ");
    LCD.setCursor(0,0);
    LCD.print(" Alim <== ");
    LCD.print(pwm);
    LCD.print("%");
    AffichageTemp(temp_C1,temp_C2);
    analogWrite(PWM_PIN, pwmGpio);
    delay(1000);
    Serial.println();
    
  }
  LCD.clear();
}
