#define trigPin 2
#define echoPin 3

#define FLOAT_SENSOR

#define buttonOn 4
int counterButtonOn;
#define BUTTON_MAX_ON 2
int counterButtonOff;
#define BUTTON_MAX_OFF 2
int counterButtonBoth;
#define BUTTON_MAX_BOTH 1
#define waterValve 6
#define floatSensor 5
#define WATER_FULL 100

#include <timer.h>
auto timer = timer_create_default();

#include <LCD_1602_RUS.h>
#define SHOW_DISTANCE 1
LCD_1602_RUS lcd(0x27, 16, 2);

long duration;
int distance;
int percentage;
int timeWater = 0;
byte timeCounter = 0;

byte distanceError = 0;
byte counterDistanceError = 0;
#define distanceErrorMAX 5

byte needStartWater;
byte waterStage = 0;
byte needStopWater;
byte needUpdateDisplay;
byte floatLevelWarning = 0;

void waterHandler(){
  if (waterStage == 2){
    if (percentage>=WATER_FULL){
      needStopWater = 1;
    }
  }
}

void stopWater(){
  needStopWater = 0;
  waterStage = 0;
  digitalWrite(waterValve, LOW);
  needUpdateDisplay = 1;
  delay(1000);
}

void startWater(){
  timeWater = 0;
  timeCounter = 0;
  needStartWater = 0;
  waterStage = 2;
  digitalWrite(waterValve, HIGH);
  needUpdateDisplay = 1;
  delay(1000);
}

bool timerLoop(void *) {
  if (waterStage != 0)
    if (timeCounter++>60){
      timeWater++;
      timeCounter= 1;
    } 
  
  if (!floatLevelWarning){
    if (waterStage == 0) {
      if (!digitalRead(buttonOn)) {
        if (++counterButtonOn >= BUTTON_MAX_ON){
         needStartWater = 1;
         waterStage = 1;
        }
      } else
        counterButtonOn = 0;
    }
  
    if (waterStage == 2) {
      if (!digitalRead(buttonOn)) {
        if (++counterButtonOff >= BUTTON_MAX_OFF)
         needStopWater = 1;
      } else 
        counterButtonOff = 0;
    }
  }

  if (floatLevelWarning){
    if ((!digitalRead(buttonOn))){
      if (++counterButtonBoth >= BUTTON_MAX_BOTH) {
          floatLevelWarning = 0;
          delay(1000);
      }
    } else 
      counterButtonBoth = 0;
  }
  

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  int oldDistance = distance;
  distance = duration*0.034/2;
  if (distanceError == 0)
  if (distance > 101){
    if (counterDistanceError++>distanceErrorMAX){
      distanceError = 1;
      needUpdateDisplay = 1;
      if (waterStage != 0)
        needStopWater = 1;
    }
  } else
    counterDistanceError = 0;
  int oldPercentage = percentage;
  percentage = map(distance, 95, 20, 0, 100);
  if (percentage > 100)
    percentage = 100;
  if (percentage < 0)
    percentage = 0;
  if ((oldPercentage != percentage) || (oldDistance != distance))
    needUpdateDisplay = 1;
  
  return true;
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonOn, INPUT);
  digitalWrite(buttonOn, HIGH);
  pinMode(waterValve, OUTPUT);
  digitalWrite(waterValve, LOW);
  pinMode(floatSensor, INPUT);
  digitalWrite(floatSensor, HIGH);

  lcd.init();
  lcd.backlight();

  timer.every(1000, timerLoop);
}

void loop() {
  timer.tick();
    
  if (needUpdateDisplay){
    needUpdateDisplay = 0;
    
    lcd.clear();
    lcd.setCursor(0,0);
    if (distanceError) {
      lcd.print("Ошибка");
      lcd.setCursor(4, 1 );
      lcd.print("датчика!!!");
    } else {
      if (!floatLevelWarning){
        lcd.print("Объем ");
        lcd.print(percentage);
        lcd.print("%");
        if (SHOW_DISTANCE){
          lcd.print("(");
          lcd.print(distance);
          lcd.print(")");
        }
        lcd.setCursor(0, 1 );
        if (waterStage == 0) {
          lcd.print("Кран ");
          
        } else {
          lcd.print("Набор. Кран ");
        }
    
        lcd.print(digitalRead(waterValve) ? "вкл." : "выкл");
      } else {
        lcd.print("Сработал");
        lcd.setCursor(4, 1 );
        lcd.print("попловок!!!");
      }
    }
  }

  #ifdef FLOAT_SENSOR
  if ((digitalRead(floatSensor)) && (digitalRead(waterValve))){
    needStopWater = 1;
    floatLevelWarning = 1;
  }
  #endif

  if (needStartWater){
    startWater();
  }
  if (needStopWater){
    stopWater();
  }
  if (waterStage != 0){
    waterHandler();
  }
}
