#include "TimerOne.h"
#include <avr/sleep.h>
#include "EnableInterrupt.h"

#define FADE_DELAY 30
#define INITIAL_DELAY 10000000
const int Ls = 3;
const int L[] = {2, 4, 7, 8};
const uint8_t B[] = {5, 6, 12, 13};
bool initPhase = true;
bool initMessagePrinted = false;
int brightness = 0;
int fadeStep = 5;
bool sleeping = false;
bool timerInitialized = false;

void fadeRed()
{
    analogWrite(Ls, brightness);

    brightness += fadeStep;

    if (brightness == 0 || brightness == 255)
    {
        fadeStep = -fadeStep;
    }

    delay(FADE_DELAY);
}

void initInitialState()
{
    Serial.println("WAKING UP");
    //sleep_disable();
    sleeping = false;
    initPhase = true;
    brightness = 0;
    initMessagePrinted = false;
    initPhase = true;
    timerInitialized = false;
}

bool isStartButtonPressed()
{
    return digitalRead(B[0]) == HIGH;
}

void deepSleep()
{
    sleeping = true;
    digitalWrite(Ls, LOW);
    delay(100);
    //Serial.flush();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    sleep_disable();
    delay(250);
    Serial.println("AAAAAA");
}

void setup()
{
    Serial.begin(9600);
    pinMode(Ls, OUTPUT);
    for (int i = 0; i < 4; i++)
    {
        pinMode(L[i], OUTPUT);
        pinMode(B[i], INPUT);
        enableInterrupt(B[i], initInitialState, RISING);
    }
    
}

void loop()
{
    if (initPhase)
    {
        if(!timerInitialized) {
            timerInitialized = true;
            Timer1.initialize(INITIAL_DELAY);
            Timer1.attachInterrupt(deepSleep);
        }
        fadeRed();
        if (!initMessagePrinted)
        {
            Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
            initMessagePrinted = true;
        }
    }
    
    if (isStartButtonPressed())
    {
        initPhase = false;
        // TODO: Complete game
        
    }
    
}
