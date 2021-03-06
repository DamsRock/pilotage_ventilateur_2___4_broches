#include <LiquidCrystal.h>
#include <LibPrintf.h>

#define PWM_PIN     6//9
#define SENSOR1 A0
#define SENSOR2 A2
#define SEUIL_MAX 64
#define SEUIL_DECLENCHEMENT 33
#define DELAY 250//delay de 60~=1.2s en réel

LiquidCrystal LCD(12, 11, 5, 4, 3, 2);

unsigned long delayStart = 0;
bool delayRunning = false;
int pwm, pwmGpio;
float tempS1, tempS2, moyTemp, tempMax, tampon;


void setup()
{
  Serial.begin(9600);
  analogReference(EXTERNAL);
  Serial.println();
  Serial.println("FAN CONTROL");
  LCD.begin(16, 2);
  LCD.setCursor(0, 0);
  LCD.print("Programme :");
  LCD.setCursor(0, 1);
  LCD.print("Pilotage ventilo");
  delay(1000);
  LCD.clear();


  //permet le réglage de la fréquence du PWM le mieux pour le ventilateur Nidec ultraflo est de se raprocher de la fréquence du courant qui est normalement en sortie d'une alimentation
  //ATTENTION!!!!
  //Le TCCR0B change également la fréquence des delay
  //le delay est multiplié par environ 10 avec la valeur suivante
  TCCR0B = TCCR0B & B11111000 | B00000101; // for PWM frequency of 61.04 Hz
  TCCR1B = TCCR1B & B11111000 | B00000101; // for PWM frequency of 30.64 Hz

  digitalWrite(PWM_PIN, LOW);
  pinMode(PWM_PIN, OUTPUT);


  Serial.println();
  Serial.println("Lancement 100%");
  LCD.setCursor(4, 0);
  LCD.print("ATTENTION");
  LCD.setCursor(1, 1);
  LCD.print("Lancement 100%");
  analogWrite(PWM_PIN, 254);
  delay(DELAY / 3);
  LCD.clear();

}

float CalculMesureTempLM61(int ra, int sensorNumber) {
  float mesureTemp;

  mesureTemp = 3.3;//tension de référence interne si par defaut
  mesureTemp /= 1023;
  mesureTemp *= ra;
  mesureTemp *= 1000;
  mesureTemp -= 600;
  mesureTemp /= 10;

  Serial.println();
  //  Serial.println("calcul mesureTemp via LM61");
  printf("Valeur numérique lu via LM61 sur le port A%d : %d\nConvertion en degrés Celcius : %f°C\n", sensorNumber, ra, mesureTemp);

  return mesureTemp;
}



void AffichageTemp(float tempS1, float tempS2) {
  Serial.println();
  Serial.println();
  printf("T1 : %f;\nTM: %f", tempS1, tempS2);
  LCD.setCursor(0, 1);
  LCD.print("T1:");
  LCD.print(tempS1, 1);
  LCD.print("  TM:");
  LCD.print(tempS2, 1);
}


void loop() {



  tempS1 = CalculMesureTempLM61(analogRead(SENSOR1), 1);
  tempS2 = CalculMesureTempLM61(analogRead(SENSOR2), 2);

  moyTemp = tempS1 + tempS2;
  moyTemp /= 2;
  if (tempS1 > tempMax)tempMax = tempS1;
  if (tempS1 < 0 || tempS2 < 0) {
    analogWrite(PWM_PIN, 0);
    Serial.println();
    Serial.println("température  erronée");
    printf("T1: %f°C ; T2: %f°C ; tempMax: %f°C", tempS1, tempS2, tempMax);
    Serial.println();
    LCD.setCursor(0, 0);
    LCD.print("Attention!!");
    LCD.setCursor(0, 1);
    LCD.print("Thermal Error");
    delay(DELAY);
  }
  else {
    if ((tempS1 < SEUIL_DECLENCHEMENT) && (tempS2 < SEUIL_DECLENCHEMENT)) {
      pwm = 0;
      delayStart = millis();//lance un timer
      delayRunning = true;
      if (delayRunning && ((millis() - delayStart) >= 2500)) {
        analogWrite(PWM_PIN, 0);
        LCD.clear();
        LCD.noDisplay();
        Serial.println();
        Serial.println("Mise en veille des ventillateurs et du LCD");
      }
      else delayRunning = false;
      Serial.println();
      printf("Température infèrieure au seuil fixé (%d°C)\nExtinction des ventilateurs (%d%%)", SEUIL_DECLENCHEMENT, pwm);
      Serial.println();

    }
    else if (tempS1 > SEUIL_MAX && tempS2 > SEUIL_MAX) {
      pwm = 100;
      Serial.println();
      printf("Température au delà du seuil maxi (%d°C)\nMarche forcée des ventilateurs à %d%% ", SEUIL_MAX, pwm);
      Serial.println();

    }
    else {
      //pourcentage pwm=-422,533+128,689*Ln(température)


      if (tempS1 < tempS2) {
        tampon = tempS2;
        Serial.println("tempS2");
      }
      else {
        tampon = tempS1;
        Serial.println("tempS1");
      }

      
      tampon = log(tampon);
      tampon *= 128,689;
      tampon -= 422,533;
      pwm = tampon;

      
      //printf("tampon %f\n", tampon);
      //printf("pwm %d%", pwm);
      //Serial.println();
    }
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");
    printf("\nTemperature maxi : %f°C\n\n", tempMax);

    pwmGpio = pwm * 2.54;
    printf("pwm = %d%%\npwmGpio = %d", pwm, pwmGpio);

    if (pwm < 0 || pwm > 100) {
      if (pwm < 0)analogWrite(PWM_PIN, 0);
      else analogWrite(PWM_PIN, 100);
      Serial.println();
      LCD.clear();
      printf("Arret des ventilateur\nValeur non admise pwm n'est plus compris entre 0 et 100%\ntemp=%d pwm=%d", tempS1, pwm);

      LCD.setCursor(0, 0);
      LCD.print("Attention T1=");
      LCD.print(tempS1);
      LCD.print("C");
      LCD.setCursor(0, 1);
      LCD.print("pwm ");
      LCD.print(pwm);
      LCD.print("%");
      delay(DELAY);
      LCD.clear();
    }
    else {
      LCD.setCursor(2, 0);
      LCD.print("Alim ==>  ");
      LCD.print(pwm);
      LCD.print("%");
      AffichageTemp(tempS1, tempMax);
      analogWrite(PWM_PIN, pwmGpio);
      delay(DELAY);
      Serial.println();
    }
  }
  LCD.clear();
}
