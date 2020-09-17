#include <LiquidCrystal.h>
#include <LibPrintf.h>

#define PWM_PIN     6//9
#define SENSOR1 A0
#define SENSOR2 A2
#define SEUIL_MAX 64
#define SEUIL_DECLENCHEMENT 33
#define DELAY 116//delay de 61.04~=1s en réel

LiquidCrystal LCD(12, 11, 5, 4, 3, 2);

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
  delay(DELAY);
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
  printf("T1 : %f;\nT2: %f", tempS1, tempS2);
  LCD.setCursor(0, 1);
  LCD.print("T1:");
  LCD.print(tempS1, 1);
  LCD.print("  T2:");
  LCD.print(tempS2, 1);
}


void loop() {
  int pwm, pwmGpio;
  float tempS1, tempS2, moyTemp, tampon;


  tempS1 = CalculMesureTempLM61(analogRead(SENSOR1), 1);
  tempS2 = CalculMesureTempLM61(analogRead(SENSOR2), 2);

  moyTemp = tempS1 + tempS2;
  moyTemp /= 2;
  if (tempS1 < 0 || tempS2 < 0) {
    analogWrite(PWM_PIN, 0);
    Serial.println();
    Serial.println("température  erronée");
    printf("T1: %d°C ; T2: %d°C", tempS1, tempS2);
    Serial.println();
    LCD.setCursor(0, 0);
    LCD.print("Attention!!");
    LCD.setCursor(0, 1);
    LCD.print("Thermal Error");
    delay(DELAY);
  } else {
    if (tempS1 < SEUIL_DECLENCHEMENT) {
      pwm = 0;
      Serial.println();
      printf("Température1 infèrieure au seuil fixé (%d°C)\nExtinction des ventilateurs (%d%%)", SEUIL_DECLENCHEMENT, pwm);
      Serial.println();

    }
    else if (tempS2 < SEUIL_DECLENCHEMENT) {
      pwm = 0;
      Serial.println();
      printf("Température2 infèrieure au seuil fixé (%d°C)\nExtinction des ventilateurs (%d%%)", SEUIL_DECLENCHEMENT, pwm);
      Serial.println();

    }
    else if (tempS1 > SEUIL_MAX) {
      pwm = 100;
      Serial.println();
      printf("Température1 au delà du seuil maxi (%d°C)\nMarche forcée des ventilateurs à %d%% ", SEUIL_MAX, pwm);
      Serial.println();

    }
    else if (tempS2 > SEUIL_MAX) {
      pwm = 100;
      Serial.println();
      printf("Température2 au delà du seuil maxi (%d°C)\nMarche forcée des ventilateurs à %d%% ", SEUIL_MAX, pwm);
      Serial.println();

    }
    else {
      //pwm = -347,907+107.699*log(tempS1);
      tampon = log(tempS1);
      tampon *= 107.699;
      tampon -= 347,907;
      pwm = tampon;
      //printf("tampon %f\n", tampon);
      //printf("pwm %d%", pwm);
      //Serial.println();
    }
    Serial.print("Moyenne température : ");
    Serial.print(moyTemp);
    Serial.println("°C");

    pwmGpio = pwm * 2.54;
    printf("pwm = %d%%\npwmGpio = %d", pwm, pwmGpio);

    if (pwm < 0 || pwm > 100) {
      pwmGpio = 0;
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
      AffichageTemp(tempS1, tempS2);
      analogWrite(PWM_PIN, pwmGpio);
      delay(DELAY);
      Serial.println();
    }
  }
  LCD.clear();
}
