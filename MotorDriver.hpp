/*
    MotorDriver.hpp
    Joel Goodman, 2020

    @brief A set of easy to use Classes and Functions to help Arduino users control DC motors.
*/
#ifndef MOTORDRV_HPP
#define MOTORDRV_HPP


/* STL/C++ */
#include <math.h>

/* Others */
#include <Arduino.h>


class MotorDriver
{
protected:
    // static constexpr unsigned int m_timeout = 100000;

    /* Arduino stuff. */
    const unsigned int m_inputPin;

    /* The scale factor will be applied to the max and min values. This is useful if you want to, say,
       give your motors a speed limit but still wat to use the full physucal range of your transmitter's
       sticks. */
    float m_scaleFactor = 1.0;

    /* If this is true, we're taking in input from an analog source and we need to initialize
       m_inputPin. */
    const bool m_analogInput;

    /* These values should be determined experimentally if you are using a hobby transmitter or any
       other type of controller that you bought off the shelf. */
    const int m_inputFloor;
    const int m_inputCeiling;
    /* You may want to set a range where the motor doesn't move to account for oversensitivity in
       transmitters. */
    const int m_deadZoneMax;
    const int m_deadZoneMin;

    void setScaleFactor(const float& scaleFactor)
    {
        if(scaleFactor < 0.0)
            m_scaleFactor = 0.0;
        else if(scaleFactor > 1.0)
            m_scaleFactor = 1.0;
        else
            m_scaleFactor = scaleFactor;
    }

    virtual void moveMotor(const int inputVal) const = 0;

public:

    MotorDriver(const int inputFloor, const int inputCeiling,
                const int deadZoneMax, const int deadZoneMin) :
        m_inputPin(999),
        m_analogInput(false),
        m_inputFloor(inputFloor),
        m_inputCeiling(inputCeiling),
        m_deadZoneMax(deadZoneMax),
        m_deadZoneMin(deadZoneMin)
    {
    }

    MotorDriver(const unsigned int inputPin,
                const int inputFloor, const int inputCeiling,
                const int deadZoneMax, const int deadZoneMin) :
        m_inputPin(inputPin),
        m_analogInput(true),
        m_inputFloor(inputFloor),
        m_inputCeiling(inputCeiling),
        m_deadZoneMax(deadZoneMax),
        m_deadZoneMin(deadZoneMin)
    {
        /* Set input pin(s) */
        pinMode(inputPin, INPUT);
    }


    void operator()() const
    {
        if(!m_analogInput)
            return;

        /* Read the input pin */
        const int inputVal = pulseIn(m_inputPin, HIGH, 100000);
        moveMotor(inputVal);

    }


    void operator()(const int inputVal) const
    {
        moveMotor(inputVal);
    }
};


class HBridge : public MotorDriver
{
    const unsigned int m_posPin;
    const unsigned int m_negPin;
    const unsigned int m_pwmPin;

public:
    HBridge(const unsigned int posPin, const unsigned int negPin,
            const unsigned int pwmPin,
            const int inputFloor=-255, const int inputCeiling=255,
            const int deadZoneMax=0, const int deadZoneMin=0) :
        MotorDriver(inputFloor, inputCeiling, deadZoneMax, deadZoneMin),
        m_posPin(posPin),
        m_negPin(negPin),
        m_pwmPin(pwmPin)
    {
        pinMode(m_posPin, OUTPUT);
        pinMode(m_negPin, OUTPUT);
        pinMode(m_pwmPin, OUTPUT);
    }

    HBridge(const unsigned int posPin, const unsigned int negPin,
            const unsigned int pwmPin, const unsigned int inputPin,
            const int inputFloor=-255, const int inputCeiling=255,
            const int deadZoneMax=10, const int deadZoneMin=-10) :
        MotorDriver(inputPin, inputFloor, inputCeiling, deadZoneMax, deadZoneMin),
        m_posPin(posPin),
        m_negPin(negPin),
        m_pwmPin(pwmPin)
    {
        pinMode(m_posPin, OUTPUT);
        pinMode(m_negPin, OUTPUT);
        pinMode(m_pwmPin, OUTPUT);
    }

protected:
    void moveMotor(const int inputVal) const
    {
        const int lowerEndOfRange = round(-255 * m_scaleFactor);
        const int upperEndOfRange = round( 255 * m_scaleFactor);

        const int controlVal = constrain(map(inputVal, m_inputFloor, m_inputCeiling, -255, 255),
                                         lowerEndOfRange,
                                         upperEndOfRange);

        if(controlVal < m_deadZoneMin)
        {
            digitalWrite(m_posPin, LOW);
            digitalWrite(m_negPin, HIGH);
            analogWrite(m_pwmPin, controlVal);
        }
        else if(controlVal > m_deadZoneMax)
        {
            digitalWrite(m_posPin, HIGH);
            digitalWrite(m_negPin, LOW);
            analogWrite(m_pwmPin, controlVal);
        }
        else
        {
            digitalWrite(m_posPin, LOW) ;
            digitalWrite(m_negPin, HIGH) ;
            analogWrite(m_pwmPin, 0) ;
        }
    }
};


class HBridgePair
{
    HBridge& m_a;
    HBridge& m_b;

public:
    HBridgePair(HBridge& a, HBridge& b) :
        m_a(a),
        m_b(b)
    {}

    void operator()()
    {
        m_a();
        m_b();
    }

    void operator()(const int& inputValA, const int& inputValB)
    {
        m_a(inputValA);
        m_b(inputValB);
    }
};


class HalfBridge : public MotorDriver
{
private:
    const unsigned int m_enablePinA;
    const unsigned int m_enablePinB;
    const unsigned int m_pwmPinA;
    const unsigned int m_pwmPinB;

protected:
    void moveMotor(const int inputVal) const
    {
        const int lowerEndOfRange = round(-255 * m_scaleFactor);
        const int upperEndOfRange = round( 255 * m_scaleFactor);

        const int controlVal = constrain(map(inputVal, m_inputFloor, m_inputCeiling, -255, 255),
                                         lowerEndOfRange,
                                         upperEndOfRange);

        if(controlVal < m_deadZoneMin)
        {
            digitalWrite(m_enablePinA, HIGH);
            digitalWrite(m_enablePinB, HIGH);
            analogWrite(m_pwmPinA, 0) ;
            analogWrite(m_pwmPinB, abs(controlVal)) ;
        }
        else if(controlVal > m_deadZoneMax)
        {
            digitalWrite(m_enablePinA, HIGH);
            digitalWrite(m_enablePinB, HIGH);
            analogWrite(m_pwmPinA, abs(controlVal)) ;
            analogWrite(m_pwmPinB, 0) ;
        }
        else
        {
            digitalWrite(m_enablePinA, LOW);
            digitalWrite(m_enablePinB, LOW);
            analogWrite(m_pwmPinA, 0) ;
            analogWrite(m_pwmPinB, 0) ;
        }
    }

public:
    HalfBridge(const unsigned int posPin, const unsigned int negPin,
         const unsigned int pwmPinA, const unsigned int pwmPinB,
         const unsigned int enablePinA, const unsigned int enablePinB,
         const int inputFloor=-255, const int inputCeiling=255,
         const int deadZoneMax=0, const int deadZoneMin=0) :
        MotorDriver(inputFloor, inputCeiling, deadZoneMax, deadZoneMin),
        m_enablePinA(enablePinA),
        m_enablePinB(enablePinB),
        m_pwmPinA(pwmPinA),
        m_pwmPinB(pwmPinB)
    {
        pinMode(m_enablePinA, OUTPUT);
        pinMode(m_enablePinB, OUTPUT);
        pinMode(m_pwmPinA, OUTPUT);
        pinMode(m_pwmPinB, OUTPUT);
    }

    HalfBridge(const unsigned int posPin, const unsigned int negPin,
         const unsigned int pwmPinA, const unsigned int pwmPinB,
         const unsigned int enablePinA, const unsigned int enablePinB,
         const unsigned int inputPin,
         const int inputFloor=-255, const int inputCeiling=255,
         const int deadZoneMax=10, const int deadZoneMin=-10) :
        MotorDriver(inputPin, inputFloor, inputCeiling, deadZoneMax, deadZoneMin),
        m_enablePinA(enablePinA),
        m_enablePinB(enablePinB),
        m_pwmPinA(pwmPinA),
        m_pwmPinB(pwmPinB)
    {
        pinMode(m_enablePinA, OUTPUT);
        pinMode(m_enablePinB, OUTPUT);
        pinMode(m_pwmPinA, OUTPUT);
        pinMode(m_pwmPinB, OUTPUT);
    }
};


#endif /* MOTORDRV_HPP */

