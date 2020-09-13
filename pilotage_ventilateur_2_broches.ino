#include <LiquidCrystal.h>
#include <LibPrintf.h>

#define PWM_PIN     9
#define SENSOR1 A0
#define SENSOR2 A1
#define PRINTF_MAX_FLOAT 2
#define SEUIL_DECLENCHEMENT 35

LiquidCrystal LCD(12, 11, 5, 4, 3, 2);

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("FAN CONTROL");
  LCD.begin(16, 2);
  LCD.setCursor(0, 0);
  LCD.print("Programme :");
  LCD.setCursor(0, 1);
  LCD.print("Pilotage ventilo");
  delay(1000);
  LCD.clear();

  TCCR1B = TCCR1B & B11111000 | B00000101; //permet le réglage de la fréquence du PWM le mieux pour le ventilateur Nidec ultraflo est de se raprocher de la fréquence du courant qui est normalement en sortie d'une alimentation

  digitalWrite(PWM_PIN, LOW);
  pinMode(PWM_PIN, OUTPUT);

  LCD.setCursor(3, 0);
  LCD.print("ATTENTION");
  LCD.setCursor(0, 1);
  LCD.print("Lancement 100%");
  analogWrite(PWM_PIN, 254);
  delay(1222);
  LCD.clear();
  analogWrite(PWM_PIN, 0);
}

float CalculTempLM61(int ra) {
  float temp;


  Serial.println();
  Serial.println("calcul temp via LM61");
  Serial.println(ra);
  temp = 5;
  temp /= 1023;
  temp *= ra;
  temp *= 1000;
  temp -= 600;
  temp /= 10;
  //temp -= 1;
  //je soustrais 1°C car différence avec les valeurs prélevée par 3 autres appareils complètement indépendant,
  //cela est peut être du au fait que le capteur soit dans une coque.
  Serial.print(temp, 1);
  Serial.println("°C");
  Serial.println();
  return temp;
}

float CalculTempLM19(int ra) {
  float temp;
  Serial.println("calcul temp via LM19");
  Serial.print("Valeur lu : ");
  Serial.println(ra);
  temp = 5;
  temp /= 1023;
  temp *= ra;
  temp = 1.8639 - temp;
  temp *= pow(10, 6);
  temp /= 3.88;
  temp += 2.1962 * pow(10, 6);
  temp = sqrt(temp);
  temp -= 1481.96;

  Serial.println(temp, 1);
  return temp;
}


void AffichageTemp(float temp1, float temp2) {
  Serial.println();
  Serial.println();
  printf("T1 : %f;\nT2: %f", temp1, temp2);
  LCD.setCursor(0, 1);
  LCD.print("T1:");
  LCD.print(temp1, 1);
  LCD.print("  T2:");
  LCD.print(temp2, 1);
}


void loop() {
  int pwm, pwmGpio;
  float tempS1, tempS2, moyTemp, tampon;


  tempS1 = CalculTempLM61(analogRead(SENSOR1));
  tempS2 = CalculTempLM61(analogRead(SENSOR2));

  moyTemp = tempS1 + tempS2;
  moyTemp /= 2;
  if (tempS1 < 0 || tempS2 < 0) {
    Serial.println();
    Serial.println("température trop basse ou erronée");
    Serial.println();
  } else {
    if (tempS1 < SEUIL_DECLENCHEMENT) {
      Serial.println();
      printf("température infèrieure au seuil fixé : %d", SEUIL_DECLENCHEMENT);
      pwm = 0;
    }
    else {
      //pwm = -300.951+94.532*log(tempS1);
      tampon = log(tempS1);
      tampon *= 94.532;
      tampon -= 300.951;
      pwm = tampon;
      printf("tampon %f\n",tampon);
      printf("pwm %d",pwm);
      Serial.println();
    }
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");

    pwmGpio = pwm * 2.54;
    printf("pwm = %d%\npwmGpio = %d", pwm, pwmGpio);

    if (pwm < 0 || pwm > 100) {
      pwmGpio = 0;
      Serial.println();
      LCD.clear();
      printf("Arret des ventilateur\nValeur non admise temp=%d pwm=%d", tempS1, pwm);

      LCD.setCursor(0, 0);
      LCD.print("Attention T1=");
      LCD.print(tempS1);
      LCD.print("C");
      LCD.setCursor(0, 1);
      LCD.print("pwm ");
      LCD.print(pwm);
      LCD.print("%");
      delay(1000);
      LCD.clear();
    }
    else {
      LCD.setCursor(0, 0);
      LCD.print(" Alim ==> ");
      LCD.print(pwm);
      LCD.print("%");
      AffichageTemp(tempS1, tempS2);
      analogWrite(PWM_PIN, pwmGpio);
      delay(850);
      Serial.println();
    }
  }
}














void old_loop()
{
  int pwm, pwmGpio;
  float tempS1, tempS2, moyTemp;


  for (pwm = 30 ; pwm <= 100 ; pwm += 10) {
    tempS1 = CalculTempLM61(analogRead(SENSOR1));
    tempS2 = CalculTempLM61(analogRead(SENSOR2));

    moyTemp = tempS1 + tempS2;
    moyTemp /= 2;
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");


    Serial.print(pwm);
    Serial.print("%: (");
    pwmGpio = pwm * 2.54;
    Serial.print(pwmGpio);
    Serial.print(") ");
    LCD.setCursor(0, 0);
    LCD.print(" Alim ==> ");
    LCD.print(pwm);
    LCD.print("%");
    AffichageTemp(tempS1, tempS2);
    analogWrite(PWM_PIN, pwmGpio);
    delay(850);
    Serial.println();
  }
  for (pwm = 95 ; pwm >= 35 ; pwm -= 10) {
    tempS1 = CalculTempLM61(analogRead(SENSOR1));
    tempS2 = CalculTempLM61(analogRead(SENSOR2));

    moyTemp = tempS1 + tempS2;
    moyTemp /= 2;
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");
    Serial.print(pwm);
    Serial.print("%: (");
    pwmGpio = pwm * 2.54;
    Serial.print(pwmGpio);
    Serial.print(") ");
    LCD.setCursor(0, 0);
    LCD.print(" Alim <== ");
    LCD.print(pwm);
    LCD.print("%");
    AffichageTemp(tempS1, tempS2);
    analogWrite(PWM_PIN, pwmGpio);
    delay(1000);
    Serial.println();

  }
  LCD.clear();
}
