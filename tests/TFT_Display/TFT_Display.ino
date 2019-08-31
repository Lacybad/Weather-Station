//Uncomment in User_Setup.h
//  #define ST7735_Driver
//  #define ST7736_GREENTAB2

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

#define pwmOut D3       //output pin for PWM, could be led

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

uint16_t i;
bool ledOutput = false;
uint16_t rawAnalog = 0;
float voltage= 0;

void printOut();
void ledFlip();
void setupLED();
void setupPWM();
void stepIncreaseBrightness();
void printADC();

void setupLED(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void setupPWM(){
    analogWriteRange(16);
    analogWriteFreq(1000);
    pinMode(pwmOut, OUTPUT);
    analogWrite(pwmOut, 1);
}

void setup(void) {
    setupLED();     //allow LED to flash
    Serial.begin(115200);
    Serial.println("\nStarting");
    setupPWM();     //allow pwm output
    tft.init();
    tft.setRotation(2);
    printOut();
}

void loop(){
    stepIncreaseBrightness();
    delay(2000);
}

void stepIncreaseBrightness(){
    for (i=1; i<=16; i++){
        Serial.print("Brightness: ");
        Serial.println(i);
        printADC();
        analogWrite(pwmOut, i);
        ledFlip();
        delay(2000);
    }
}

void printADC(){
    rawAnalog = analogRead(A0);
    voltage = rawAnalog * (3.3 / 1023.0); //force to be a float
    Serial.print("ADC reads: ");
    Serial.print(rawAnalog);
    Serial.print(" (");
    Serial.print(voltage);
    Serial.println("V)");
}

void ledFlip(){
    if (ledOutput) {digitalWrite(LED_BUILTIN, ledOutput);}
    else {digitalWrite(LED_BUILTIN, ledOutput);}
    ledOutput = !ledOutput;
}

void printOut() {
    uint16_t cursor1 = 0;
    uint16_t cursor2 = 0;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);

    int a[6] = {1, 2, 4, 6, 7, 8};

    for (int i = 0; i < 6; i++){
        tft.setTextSize(a[i]);
        cursor1 = tft.getCursorY();
        Serial.print(a[i]); Serial.print("s:");
        Serial.println(cursor1);

        tft.println("H");
        cursor2 = tft.getCursorY();
        Serial.print(a[i]); Serial.print("e:");
        Serial.print(cursor2); Serial.print(" t:");
        Serial.println(cursor2-cursor1);
    }
}


