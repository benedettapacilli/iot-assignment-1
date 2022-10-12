#include "TimerOne.h"
#include <avr/sleep.h>
#include "EnableInterrupt.h"

#define FADE_DELAY 30
#define INITIAL_DELAY 10000000
const int Ls = 3;
const int L[] = {2, 4, 5, 6};
const int B[] = {7, 8, 9, 10};
int T[] = {0, 6000, 6000};
bool initPhase = true;
bool initMessagePrinted = false;
int brightness = 0;
int fadeStep = 5;
bool sleeping = false;
bool timerInitialized = false;
int startTime = millis();
bool beingPressed = false;
int score = 0;
int penalties = 0;

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
    sleeping = false;
    initPhase = true;
    brightness = 0;
    initMessagePrinted = false;
    initPhase = true;
    timerInitialized = false;
    startTime = millis();
}

bool isStartButtonPressed()
{
    return digitalRead(B[0]) == HIGH;
}

bool isAButtonPressed()
{
    return digitalRead(B[0]) == HIGH || digitalRead(B[1]) == HIGH || digitalRead(B[2]) == HIGH || digitalRead(B[3]) == HIGH;
}

void givePenalty()
{
    penalties++;
    Serial.println("Penalty!");
    digitalWrite(Ls, HIGH);
    delay(1000);
    digitalWrite(Ls, LOW);
    if (penalties > 3)
    {
        // END GAME
    }
}

void deepSleep()
{
    sleeping = true;
    digitalWrite(Ls, LOW);
    delay(100);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    Serial.println("WAKING UP");
    sleep_disable();
    delay(1000);
}

void game()
{
    digitalWrite(Ls, LOW);
    Serial.println("Go!");
    delay(T[0]);
    int pattern[4];

    Serial.println("Turning leds on");
    for (int i = 0; i < 4; i++)
    {
        pattern[i] = random(0, 2);
        digitalWrite(L[i], pattern[i]);
    }
    int patternOnTime = millis();
    while (true)
    {
        if (millis() - patternOnTime > T[1])
            break;

        if (isAButtonPressed())
            givePenalty();
    }
    Serial.println("Turning leds off");
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(L[i], LOW);
    }

    int startGameTime = millis();
    while (true)
    {
        if (millis() - startGameTime < T[2])
        {
            for (int i = 0; i < 4; i++)
            {
                if (digitalRead(B[i]) == HIGH)
                {
                    if (pattern[i] == 0)
                    {
                        givePenalty();
                    }
                    else
                    {
                    }
                }
            }
        }
        else
        {
            givePenalty();
            break;
        }
    }
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
    randomSeed(analogRead(0));
    T[0] = random(3000, 7000);
}

void loop()
{
    if (initPhase)
    {
        if (millis() - startTime > 10000)
        {
            deepSleep();
        }
        fadeRed();
        if (!initMessagePrinted)
        {
            Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
            initMessagePrinted = true;
        }
    }

    bool buttonPressed = isStartButtonPressed();
    if (buttonPressed && !beingPressed)
    {
        beingPressed = true;
        initPhase = false;
        game();
    }
    else if (!buttonPressed)
    {
        beingPressed = false;
    }
}
