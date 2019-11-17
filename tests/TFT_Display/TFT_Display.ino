//Uncomment in User_Setup.h
//  #define ST7735_Driver
//  #define ST7736_GREENTAB

//pinout D6=RST, D0=A0/DC, D8=CS, D7=SDA, D5=SCL
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Ticker.h>     //for interrupts

#define pwmOut D3       //output pin for PWM, could be led
#define autoBrightnessDef //comment to use step brightness
//#define useLED        //uncomment to use leds
#define pwm 6       //bits of range
#define pwmOutput (1<<pwm)-1 //1023=default

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
Ticker tftBrightness;

uint16_t i;
uint8_t j=0;
bool ledOutput = false;
volatile uint16_t rawAnalog = 0;
volatile uint16_t newBrightness = 0;
float voltage= 0;
uint8_t oldBrightness = 0;

void printOut();
void ledFlip();
void setupLED();
void setupPWM();
void setupTimer();
void autoBrightness();
void stepIncreaseBrightness();
void printADC();
void updateBrightness();

void updateBrightness(){
    rawAnalog = analogRead(A0);
    newBrightness = rawAnalog>>(10-pwm);
    if (newBrightness < 1){ //else display off
        newBrightness = 1;
    }
    if (newBrightness > oldBrightness){
        oldBrightness++;
        analogWrite(pwmOut, oldBrightness); //1024>>6 to 16 bit
    }
    if (newBrightness < oldBrightness){
        oldBrightness--;
        analogWrite(pwmOut, oldBrightness); //1024>>6 to 16 bit
    }
#ifdef autoBrightnessDef
    Serial.print("ADC: ");
    Serial.print(rawAnalog);
    Serial.print(" >> ");
    Serial.print(newBrightness);
    Serial.print("/");
    Serial.print(oldBrightness);
    Serial.print(" ");
    j++;
    if(j>3){
        j = 0;
        Serial.print("\n");
    }
#endif
}

void setupLED(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void setupPWM(){
#ifdef autoBrightnessDef
    Serial.print("Brightness range: ");
    Serial.println(pwmOutput, BIN);
#endif
    analogWriteRange(pwmOutput);
    analogWriteFreq(1000);
    pinMode(pwmOut, OUTPUT);
    analogWrite(pwmOut, 1);
}

void setupTimer(){
    tftBrightness.attach_ms(500, updateBrightness); //for display brightness
}

void setup(void) {
    setupLED();     //allow LED to flash
    Serial.begin(115200);
    Serial.println("\nStarting");
    setupPWM();     //allow pwm output

#ifdef autoBrightnessDef
    setupTimer();
#endif

    tft.init();
    tft.setRotation(2);
    printOut();
}

void loop(){
#ifndef autoBrightnessDef
    stepIncreaseBrightness(); //for going through each brightness
#endif
    delay(2000);
}

void stepIncreaseBrightness(){
    for (i=1; i<=16; i++){
        Serial.print("Brightness: ");
        Serial.println(i);
        printADC();
        analogWrite(pwmOut, i);
#ifdef useLED
        ledFlip();
#endif
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
    uint16_t cursorY1 = 0;
    uint16_t cursorY2 = 0;
    uint16_t cursorX1 = 0;
    uint16_t cursorX2 = 0;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);

    int a[6] = {1, 2, 4, 6, 7, 8};

    for (int i = 0; i < 6; i++){
        tft.setTextSize(a[i]);
        cursorY1 = tft.getCursorY();
        cursorX1 = tft.getCursorX();

        tft.print("H");
        cursorX2 = tft.getCursorX();
        tft.print("\n");
        cursorY2 = tft.getCursorY();

        Serial.print(a[i]);

        Serial.print("Y: ");
        Serial.print(cursorY2);
        Serial.print("-");
        Serial.print(cursorY1);
        Serial.print("=");
        Serial.print(cursorY2-cursorY1);

        Serial.print(" X: ");
        Serial.print(cursorX2);
        Serial.print("-");
        Serial.print(cursorX1);
        Serial.print("=");
        Serial.println(cursorX2-cursorX1);
    }
}

