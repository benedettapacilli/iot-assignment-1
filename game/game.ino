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
}

bool isStartButtonPressed()
{
    return digitalRead(B[0]) == HIGH;
}

bool isAButtonPressed()
{
    return digitalRead(B[0]) == HIGH || digitalRead(B[1]) == HIGH || digitalRead(B[2]) == HIGH || digitalRead(B[3]) == HIGH;
}

void givePoint()
{
    score++;
    Serial.println("New point! Score: " + score);
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
        Serial.println("Game Over. Final Score: " + score);
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
}

bool patternChecker(int pattern[4], int userPattern[4])
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
