//for testing the button and pir motion sensor
#define pirMotion D2
#define button D1

bool pirLast = LOW;
bool pirInput = LOW;
bool buttonLast = LOW;
bool buttonInput = LOW;
uint8_t count = 0;

void printBool(bool in);

void setupLED(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void setup(void) {
    Serial.begin(115200);
    setupLED();
    pinMode(pirMotion, INPUT);
    pinMode(button, INPUT);
    Serial.println();
    Serial.println("Ready");
}

void loop(){
    pirInput = digitalRead(pirMotion);
    digitalWrite(LED_BUILTIN, !pirInput);
    if ((pirInput == HIGH) && (pirLast == LOW)){
        Serial.print(".");
        count++;
    }
    pirLast = pirInput; //get previous

    buttonInput = digitalRead(button);
    if ((buttonInput == HIGH) && (buttonLast == LOW)){
        Serial.print("#");
        count++;
    }
    buttonLast = buttonInput;

    delay(100); //ms
    if (count > 25){
        Serial.println();
    }
}

void printBool(bool in){
    if (in){
        Serial.print("T");
    }
    else {
        Serial.print("F");
    }
}
