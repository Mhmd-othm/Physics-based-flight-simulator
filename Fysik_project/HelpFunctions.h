#pragma once
#include "raylib.h"
#include "raymath.h"
#include "struct.h"
float GetMonteCarloWindVectorAngle();
float GetMonteCarloWindSpeed();
void UpdateWind(PlaneObject& plane, float dt);
float GetRealAngelOfAttack(PlaneObject& plane, Vector3 relativeVelocityVector, Vector3 planeForwardVector, Vector3 planeUpVector);
float GetNewClBasedOnAOA(PlaneObject& plane, float AOA, float relativeSpeed);


void UpdateAtmosphere(PlaneObject& plane);
void UpdatePlanePhysics(PlaneObject& plane, float dt);
void UpdateControls(PlaneObject& plane, float dt);