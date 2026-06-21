#pragma once
#include "raylib.h"

struct PlaneObject
{
    Model m_model;
    Vector3 m_position;
    Vector3 m_VelocityVector;
    float mass;

    float roll, pitch, yaw;

    // Motor
    float throttle;
    float propellerDiameter = 2.0f;
    float propellerRPS = 45.0f;
    float P_e;
    float MaxPropellerEfficiency = 0.8f;
    float MaxAdvanceRatio = 2.778f; // J_max = V_max(250)/(propellerDiameter * propellerRPS)

    float wingArea;
    float angleOfAttack;
    float cl;
    float cdZero;

    float altitude;
    float temperature;
    float pressure;
    float density;
    float lossFactor;

    float thrust;
    float drag;
    float lift;
    float turnRadius;


    // Stabilitet
    float pitchAngularVelocity;
    float pitchInertia;
    float pitchStiffness;
    float pitchDamping;

    float rollAngularVelocity;
    float rollInertia;

    // Wind parameters
    Vector3 windVelocity;      
    float windSpeed;           
    float windDirection;       
    float windUpdateTimer;     
    float windUpdateInterval; // In seconds


    PlaneObject() :
        m_position{ 0,0,0 },
        m_VelocityVector{ 0,0,0 },
        mass(40000.0f),
        roll(0), pitch(0), yaw(0),
        throttle(0.5f),
        P_e(40000000.0f),
        wingArea(122.0f),
        cl(0.0f),
        cdZero(0.05f),
        altitude(0),
        temperature(288.15f),
        pressure(101325.0f),
        density(1.225f),
        lossFactor(0),
        thrust(0),
        drag(0),
        lift(0),
        turnRadius(0),
        pitchAngularVelocity(0),
        pitchInertia(8000.0f),
        pitchStiffness(40000.0f),
        pitchDamping(20000.0f),
        rollAngularVelocity(0),
        rollInertia(9000.0f),
        windVelocity{ 0, 0, 0 },
        windSpeed(0),
        windDirection(0),
        windUpdateTimer(0),
        windUpdateInterval(5.0f)
    {
    }
};