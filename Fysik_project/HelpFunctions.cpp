#include "HelpFunctions.h"
#include <cmath>
#include <random>
#include <fstream>
//help functions

Vector3 CalculatePropellerThrustForce(PlaneObject& plane, Vector3 forwardVector) {
    float engineActualPower = plane.P_e * powf((plane.density / 1.225f), 0.8f);

    float planeSpeed = Vector3Length(plane.m_VelocityVector);
    float advanceRatio = fmax(0.0f,(planeSpeed / (plane.propellerRPS * plane.propellerDiameter)));

    float propellerEfficiency = plane.MaxPropellerEfficiency * sin((PI * advanceRatio)/(2 * plane.MaxAdvanceRatio));

    propellerEfficiency = fmax(propellerEfficiency, 0.4f);
    float thrust;
    if (planeSpeed != 0) {
        thrust = (propellerEfficiency * engineActualPower * plane.throttle) / (planeSpeed + 5.0f);
    }
    else
    {
        float PropellerArea = PI * powf((plane.propellerDiameter / 2.0f), 2.0f);
        thrust = powf((2 * plane.density * PropellerArea * engineActualPower * engineActualPower * propellerEfficiency * propellerEfficiency), (1.0f / 3.0f));
        thrust = thrust * plane.throttle;
    } 
        
    plane.thrust = thrust;
    Vector3 thrustForce = Vector3Scale(forwardVector, thrust);

    return thrustForce;
}
Vector3 CalculateJetThrustForce(PlaneObject& plane, Vector3 forwardVector) {
    float v0 = Vector3Length(plane.m_VelocityVector);
    float intakeArea = 4.0f;
    float intakeVelocity = 0.5f * sqrt(1.4f * 287.0f * plane.temperature);             // v_t = 0.5 * sqrt(y * R * T)

    float mDot_max = plane.density * intakeArea * intakeVelocity;
    
    float mDot = plane.throttle * mDot_max;

    float engineActualPower = plane.P_e * powf(plane.density / 1.225f, 0.8f);

    float v_e = 0.0f;
    float thrust = 0.0f;
    if (mDot > 0.001f) {
        v_e = sqrtf(fmaxf((2.0f * engineActualPower / mDot) + v0 * v0, 0.0f));
        thrust = mDot * (v_e - v0);

    }
    plane.thrust = thrust;

    Vector3 thrustForce = Vector3Scale(forwardVector, thrust);

    return thrustForce;
}



float GetMonteCarloWindVectorAngle() {
    float RandomNum = (float)rand() / (float)RAND_MAX;
    float windVectorAngle = RandomNum * 2 * PI;

    return windVectorAngle;
}
float GetMonteCarloWindSpeed() {
    float RandomNum = (rand() + 1.0f) / (RAND_MAX + 2.0f);
    float lamda = 8.0f; 
    float k_value = 2.0f;
    // Weibull invers of CDF
    float windSpeed = lamda * powf((-log(1 - RandomNum)), (1.0f / k_value));
    return windSpeed;
}
void UpdateWind(PlaneObject& plane, float dt) {
    plane.windUpdateTimer += dt;

    if (plane.windUpdateTimer >= plane.windUpdateInterval) {
        plane.windUpdateTimer = 0.0f;

        float newSpeed = GetMonteCarloWindSpeed();
        float newDirection = GetMonteCarloWindVectorAngle();

        // Lerp speed
        float lerpFactor = 0.3f;
        plane.windSpeed = plane.windSpeed * (1 - lerpFactor) + newSpeed * lerpFactor;

        // Lerp direction — handle wrap-around at 0/2PI boundary
        float diff = newDirection - plane.windDirection;
        if (diff > PI)  diff -= 2 * PI;
        if (diff < -PI) diff += 2 * PI;
        plane.windDirection += lerpFactor * diff;

        // Normalize
        while (plane.windDirection < 0)      plane.windDirection += 2 * PI;
        while (plane.windDirection >= 2 * PI)  plane.windDirection -= 2 * PI;

        // Rebuild velocity from lerped speed + direction
        plane.windVelocity.x = plane.windSpeed * sinf(plane.windDirection);
        plane.windVelocity.y = 0.0f;
        plane.windVelocity.z = plane.windSpeed * cosf(plane.windDirection);
    }
}

float GetRealAngleOfAttack(PlaneObject& plane, Vector3 relativeVelocityVector, Vector3 planeForwardVector, Vector3 planeUpVector)
{
    Vector3 relVelNorm = Vector3Normalize(relativeVelocityVector);

    float forwardComponent  = Vector3DotProduct(relVelNorm, planeForwardVector);

    float upComponent = Vector3DotProduct(relVelNorm, planeUpVector);

    float AOA = atan2f(upComponent, forwardComponent) * RAD2DEG;
    while (AOA > 180.0f) AOA -= 360.0f;
    while (AOA < -180.0f) AOA += 360.0f;

    plane.angleOfAttack = AOA;

    return AOA;
}
float GetNewClBasedOnAOA(PlaneObject& plane, float AOA, float relativeSpeed) {
    const float cl_0 = 0.4f;
    const float ChangeStrength = 0.1f;

    float cl = cl_0 + ChangeStrength * AOA;

    if (relativeSpeed > 0.01) {
        if (AOA > 15.0f) {
            float stallFactor = 1.0f - (AOA - 15.0f) / 20.0f;
            stallFactor = fmax(stallFactor, 0.3f);
            cl = cl * stallFactor;
        }
        else if (AOA < -10.0f) {
            float stallFactor = 1.0f - fabs(AOA + 10.0f) / 20.0f;
            stallFactor = fmax(stallFactor, 0.3f);
            cl = cl * stallFactor;
        }
    }

    cl = fmin(cl, 1.6f);
    cl = fmax(cl, -0.8f);


    plane.cl = cl;
    return cl;
}


void UpdateAtmosphere(PlaneObject& plane) {
    float T0 = 288.15f;
    float P0 = 101325.0f;
    float L = 0.0065f;

    plane.altitude = plane.m_position.y;
    if (plane.altitude < 0) plane.altitude = 0;

    plane.temperature = T0 - L * plane.altitude;
    plane.pressure = P0 * powf(1.0f - (L * plane.altitude) / T0, 5.256f);
    plane.density = plane.pressure / (287.05f * plane.temperature);
}

void UpdatePlanePhysics(PlaneObject& plane, float dt) {

    UpdateAtmosphere(plane);
    UpdateWind(plane, dt);

    Vector3 relativeVelocity = Vector3Subtract(plane.m_VelocityVector, plane.windVelocity);
    float relativeSpeed = Vector3Length(relativeVelocity);

    // Calculate rotation matrix from aircraft Euler angles (roll, pitch, yaw)
    float rollRad = plane.roll;
    float pitchRad = plane.pitch;
    float yawRad = plane.yaw;

    // Combine rotations in correct order: Z (roll) * X (pitch) * Y (yaw)
    Matrix rotation = MatrixMultiply(
        MatrixRotateZ(rollRad),
        MatrixMultiply(
            MatrixRotateX(pitchRad),
            MatrixRotateY(yawRad)
        )
    );

    // Get aircraft's local axes transformed to world space
    Vector3 localForward = { 0.0f, 0.0f, 1.0f };
    Vector3 localUp = { 0.0f, 1.0f, 0.0f };
    Vector3 localRight = { 1.0f, 0.0f, 0.0f };

    // Transform local axes to world coordinates using rotation matrix
    Vector3 planeForward = Vector3Normalize(Vector3Transform(localForward, rotation));
    Vector3 planeUp = Vector3Normalize(Vector3Transform(localUp, rotation));
    Vector3 planeRight = Vector3Normalize(Vector3Transform(localRight, rotation));

    // Calculate angle of attack (angle between aircraft nose and relative wind)

    float AOA = GetRealAngleOfAttack(plane, relativeVelocity, planeForward, planeUp);

    if (relativeSpeed > 0.01f) {
        if (AOA > 15.0f || AOA < -10.0f) {
            plane.cdZero = 0.15f;
        }
        else {
            plane.cdZero = 0.05f;
        }
    }

    // Aircraft stability
    float tau_go_back_pitch = -plane.pitchStiffness * (plane.pitch);
    float tau_damp_pitch = -plane.pitchDamping * plane.pitchAngularVelocity;
    float pitchAcceleration = (tau_go_back_pitch + tau_damp_pitch) / plane.pitchInertia;
    plane.pitchAngularVelocity += pitchAcceleration * dt;
    plane.pitch += plane.pitchAngularVelocity * dt;

    float tau_go_back_roll = -plane.pitchStiffness * (plane.roll);
    float tau_damp_roll = -plane.pitchDamping * plane.rollAngularVelocity;
    float rollAcceleration = (tau_go_back_roll + tau_damp_roll) / plane.rollInertia;
    plane.rollAngularVelocity += rollAcceleration * dt;
    plane.roll += plane.rollAngularVelocity * dt;

    // Airspeed (velocity component along aircraft's forward direction)
    float airSpeed = Vector3DotProduct(relativeVelocity, planeForward);
    if (airSpeed < 0) airSpeed = 0;

    // Forces acting on the aircraft

    // Weight force
    Vector3 weight = { 0, -plane.mass * 9.81f, 0 };

    // Drag force)
    float dragForce = 0.5f * plane.density * relativeSpeed * relativeSpeed *
        plane.wingArea * plane.cdZero;
    Vector3 drag = { 0, 0, 0 };
    if (relativeSpeed > 0.01f) {
        drag = Vector3Scale(Vector3Normalize(relativeVelocity), -dragForce);
    }
    plane.drag = dragForce;

    // Lift force
    float newCl = GetNewClBasedOnAOA(plane, AOA, relativeSpeed);
    float liftForce = 0.5f * plane.density * relativeSpeed * relativeSpeed *
        plane.wingArea * newCl;

   
    Vector3 liftDirection = Vector3CrossProduct(Vector3Normalize(relativeVelocity), planeRight);
    
    
    Vector3 lift = Vector3Scale(liftDirection, liftForce);
    plane.lift = liftForce;

    // Thrust force
    Vector3 thrust = CalculateJetThrustForce(plane, planeForward);

    // Newton's Second Law
    Vector3 totalForce = Vector3Add(Vector3Add(weight, lift), Vector3Add(drag, thrust));
    Vector3 acceleration = Vector3Scale(totalForce, 1.0f / plane.mass);

    // Update velocity and position
    plane.m_VelocityVector = Vector3Add(plane.m_VelocityVector, Vector3Scale(acceleration, dt));
    plane.m_position = Vector3Add(plane.m_position, Vector3Scale(plane.m_VelocityVector, dt));

    // Limit maximum speed
    if (Vector3Length(plane.m_VelocityVector) > 250.0f) {
        plane.m_VelocityVector = Vector3Scale(Vector3Normalize(plane.m_VelocityVector), 250.0f);
    }

    // Ground collision detection
    if (plane.m_position.y <= 1.0f) {
        plane.m_position.y = 1.0f;

        if (plane.m_VelocityVector.y < 0)
            plane.m_VelocityVector.y = 0.0f;

        if (plane.throttle < 0.01f) {
            plane.m_VelocityVector.x = 0.0f;
            plane.m_VelocityVector.z = 0.0f;
        }
    }
}

void UpdateControls(PlaneObject& plane, float dt) {

    if (IsKeyPressed(KEY_F)) {
        ToggleFullscreen();
    }
    // Throttle (W/S)
    if (IsKeyDown(KEY_W)) plane.throttle += 0.5f * dt;
    if (IsKeyDown(KEY_S)) plane.throttle -= 0.5f * dt;
    plane.throttle = fmax(fmin(plane.throttle, 1.0f), 0.0f);

    // Pitch (Up/Down arrow)
    if (IsKeyDown(KEY_UP) && plane.m_position.y > 2) {
        plane.pitch += 1.0f * dt;
    }
    if (IsKeyDown(KEY_DOWN) && plane.m_position.y > 2) {
        plane.pitch -= 1.0f * dt;
    }

    // Roll (A/D)
    if (IsKeyDown(KEY_A) && plane.m_position.y > 4) {
        plane.roll -= 1.5f * dt;
    }
    if (IsKeyDown(KEY_D) && plane.m_position.y > 4) {
        plane.roll += 1.5f * dt;
    }

    // Yaw (Q/E)
    if (IsKeyDown(KEY_Q)) plane.yaw += 1.0f * dt;
    if (IsKeyDown(KEY_E)) plane.yaw -= 1.0f * dt;

    // Limit angles
    plane.pitch = fmax(fmin(plane.pitch, 30.0f * DEG2RAD), -20.0f * DEG2RAD);
    plane.roll = fmax(fmin(plane.roll, 45.0f * DEG2RAD), -45.0f * DEG2RAD);

    // Yaw can be unlimited or limited (wrap around 0-360 degrees)
    if (plane.yaw > 360.0f * DEG2RAD) plane.yaw -= 360.0f * DEG2RAD;
    if (plane.yaw < 0) plane.yaw += 360.0f * DEG2RAD;
}