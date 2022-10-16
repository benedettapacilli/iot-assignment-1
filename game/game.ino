#include <avr/sleep.h>
#include "EnableInterrupt.h"

#define FADE_DELAY 30
#define INITIAL_DELAY 10000000
const int Ls = 11;
const int L[] = {10, 9, 8, 7};
const int B[] = {5, 4, 3, 2};
const int POT = A0;
int T[] = {0, 6000, 6000};
float F = 1;
bool initPhase = true;
bool initMessagePrinted = false;
int brightness = 0;
int fadeStep = 5;
bool sleeping = false;
bool timerInitialized = false;
long startTime = millis();
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
    Serial.println("INIT STATE");
    initPhase = true;
    initMessagePrinted = false;
    brightness = 0;
    fadeStep = 5;
    sleeping = false;
    timerInitialized = false;
    startTime = millis();
    beingPressed = false;
    score = 0;
    penalties = 0;
    F = 1;
}

bool isStartButtonPressed()
{
    return digitalRead(B[0]) == LOW;
}

bool isAButtonPressed()
{
    return digitalRead(B[0]) == LOW || digitalRead(B[1]) == LOW || digitalRead(B[2]) == LOW || digitalRead(B[3]) == LOW;
}

void givePoint()
{
    score++;
    Serial.print("New point! Score: ");
    Serial.println(score);
    F += 0.05;
}

void givePenalty()
{
    penalties++;
    Serial.println("Penalty!");
    digitalWrite(Ls, HIGH);
    delay(1000);
    digitalWrite(Ls, LOW);
    if (penalties >= 3)
    {
        Serial.print("Game Over. Final Score: ");
        Serial.println(score);
        delay(10000);
        initInitialState();
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
    initInitialState();
}

bool patternChecker(int *pattern, int *userPattern)
{
    for (int i = 0; i < 4; i++)
    {
        if (pattern[i] != userPattern[i])
            return false;
    }

    return true;
}

void game()
{
    digitalWrite(Ls, LOW);
    Serial.println("Go!");
    delay(T[0]);
    int pattern[4];
    int userPattern[] = {0, 0, 0, 0};

    Serial.println("Turning leds on");
    int emptyPattern[] = {0, 0, 0, 0};
    do
    {
        for (int i = 0; i < 4; i++)
        {
            pattern[i] = random(0, 2);
        }
    } while (patternChecker(pattern, emptyPattern));

    for (int i = 0; i < 4; i++)
    {
        digitalWrite(L[i], pattern[i]);
    }

    long patternOnTime = millis();
    while (true)
    {
        if (millis() - patternOnTime > (T[1] / F))
            break;

        if (isAButtonPressed())
        {
            givePenalty();
            if(penalties >= 3)
                break;
        }
    }

    Serial.println("Turning leds off");
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(L[i], LOW);
    }

    long startGameTime = millis();

    while (true)
    {
        if (millis() - startGameTime < T[2] / F)
        {
            for (int i = 0; i < 4; i++)
            {
                if (digitalRead(B[i]) == LOW)
                {
                    userPattern[i] = 1;
                }
            }
        }
        else
        {
            if (patternChecker(pattern, userPattern))
            {
                givePoint();
            }
            else
            {
                givePenalty();
            }
            break;
        }
    }
}

void voidFunction()
{
}

void setup()
{
    Serial.begin(9600);
    pinMode(Ls, OUTPUT);
    for (int i = 0; i < 4; i++)
    {
        pinMode(L[i], OUTPUT);
        pinMode(B[i], INPUT);
        enableInterrupt(B[i], voidFunction, RISING);
    }
    pinMode(POT, INPUT);
    randomSeed(analogRead(0));
    T[0] = random(3000, 7000);
    Serial.println("SETUP FINISHED");
}

void loop()
{
    if (initPhase)
    {
        if (millis() - startTime > 10000)
        {
            Serial.println("GOING TO SLEEP");
            deepSleep();
        }
        fadeRed();
        if (!initMessagePrinted)
        {
            Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
            initMessagePrinted = true;
        }

        int potentioMeterValue = analogRead(POT);
        int oldF = F;
        F = ceil(potentioMeterValue / 255.75);
        if (F == 0)
        {
            F = 1;
        }

        if (F != oldF)
        {
            Serial.print("Level: ");
            Serial.println(F);
        }

        bool buttonPressed = isStartButtonPressed();

        if (buttonPressed && !beingPressed)
        {
            beingPressed = true;
            initPhase = false;
        }
        else if (!buttonPressed)
        {
            beingPressed = false;
        }
    }
    else
    {
        game();
    }
}
