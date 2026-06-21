
#define RAYMATH_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"
#include "struct.h"
#include <cmath>
#include "HelpFunctions.h"
#include <random>
Texture2D LoadTextureWithSTB(const char* filename)
{
    Image img = LoadImage(filename);
    if (img.data == NULL) return Texture2D{ 0 };
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}
void DrawSimpleCompass(Camera3D camera, PlaneObject plane)
{
    // ===== COMPASS (right side) =====
    int x = GetScreenWidth() - 100;
    int y = GetScreenHeight() - 100;
    int radius = 80;

    // Calculate aircraft heading (yaw) in degrees
    float heading = plane.yaw * RAD2DEG;

    // Normalize heading to 0-360 degrees
    while (heading < 0) heading += 360;
    while (heading >= 360) heading -= 360;

    // Find camera's horizontal direction (for the arrow)
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward.y = 0;  // Ignore vertical component
    forward = Vector3Normalize(forward);
    float camHeading = atan2f(forward.x, forward.z) * RAD2DEG;

    // Background circle
    DrawCircle(x, y, radius, ColorAlpha(BLACK, 0.6f));
    DrawCircleLines(x, y, radius, WHITE);

    // Draw compass ring with degree marks
    for (int i = 0; i < 360; i += 30)
    {
        float rad = i * DEG2RAD;
        float innerRadius = (i % 90 == 0) ? radius - 15 : radius - 8;
        float outerRadius = radius - 5;

        Vector2 start = { x + cosf(rad) * innerRadius, y + sinf(rad) * innerRadius };
        Vector2 end = { x + cosf(rad) * outerRadius, y + sinf(rad) * outerRadius };
        DrawLineEx(start, end, 2, WHITE);

        // Draw text for cardinal directions
        if (i % 90 == 0)
        {
            const char* dir;
            Color dirColor;
            switch (i)
            {
            case 0: dir = "N"; dirColor = RED; break;
            case 90: dir = "E"; dirColor = YELLOW; break;
            case 180: dir = "S"; dirColor = LIGHTGRAY; break;
            case 270: dir = "W"; dirColor = YELLOW; break;
            default: dir = ""; dirColor = WHITE;
            }
            Vector2 textPos = { x + cosf(rad) * (radius - 25) - 5,
                               y + sinf(rad) * (radius - 25) - 8 };
            DrawText(dir, textPos.x, textPos.y, 18, dirColor);
        }
    }

    // Draw heading marker (triangle pointing to current heading)
    float headingRad = (-heading + 90) * DEG2RAD;  // 0 degrees = North = top of circle
    Vector2 headingMarker = { x + cosf(headingRad) * (radius - 15),
                              y + sinf(headingRad) * (radius - 15) };
    DrawCircleV(headingMarker, 5, RED);
    DrawCircleV(headingMarker, 3, WHITE);

    // Draw an arrow showing where the camera is pointing (relative to the plane)
    float camHeadingRad = (-camHeading + 90) * DEG2RAD;
    Vector2 camDir = { x + cosf(camHeadingRad) * (radius - 25),
                       y + sinf(camHeadingRad) * (radius - 25) };
    DrawLineEx(Vector2{ (float)x, (float)y }, camDir, 3, BLUE);

    // Draw triangle in the center pointing forward (plane's direction)
    Vector2 trianglePoints[3];
    trianglePoints[0] = { (float)x, (float)(y - radius + 10) };  // Top
    trianglePoints[1] = { (float)(x - 8), (float)(y - radius + 25) };
    trianglePoints[2] = { (float)(x + 8), (float)(y - radius + 25) };
    DrawTriangle(trianglePoints[0], trianglePoints[1], trianglePoints[2], GREEN);

    // Draw text for current heading
    DrawText(TextFormat("HDG: %.0f°", heading), x - 35, y + radius + 5, 15, WHITE);

    // Draw horizon (pitch) indicator
    int pitchX = x - radius - 20;
    int pitchY = y;
    int pitchHeight = 60;

    // Pitch background
    DrawRectangle(pitchX - 5, pitchY - pitchHeight / 2, 10, pitchHeight, ColorAlpha(BLACK, 0.6f));
    DrawRectangleLines(pitchX - 5, pitchY - pitchHeight / 2, 10, pitchHeight, WHITE);

    // Pitch marker (shows aircraft pitch)
    float pitchAngle = plane.pitch;
    int pitchMarkY = pitchY - (pitchAngle / 90.0f) * (pitchHeight / 2);
    pitchMarkY = fmax(pitchY - pitchHeight / 2, fmin(pitchMarkY, pitchY + pitchHeight / 2));

    DrawCircle(pitchX, pitchMarkY, 4, RED);

    // Pitch text
    DrawText(TextFormat("P:%.0f", pitchAngle * RAD2DEG), pitchX - 25, pitchY - 10, 12, WHITE);

    // Draw roll indicator (artificial horizon)
    int rollX = x + radius + 20;
    int rollY = y;
    int rollWidth = 40;
    int rollHeight = 40;

    // Roll background
    DrawRectangle(rollX - rollWidth / 2, rollY - rollHeight / 2, rollWidth, rollHeight, ColorAlpha(BLACK, 0.6f));
    DrawRectangleLines(rollX - rollWidth / 2, rollY - rollHeight / 2, rollWidth, rollHeight, WHITE);

    // Horizontal line that rotates with roll
    float rollRad = plane.roll;
    Vector2 horizonStart = { rollX - cosf(rollRad) * 15, rollY - sinf(rollRad) * 15 };
    Vector2 horizonEnd = { rollX + cosf(rollRad) * 15, rollY + sinf(rollRad) * 15 };
    DrawLineEx(horizonStart, horizonEnd, 3, RED);

    // Roll text
    DrawText(TextFormat("R:%.0f", plane.roll * RAD2DEG), rollX - 20, rollY + rollHeight / 2 + 5, 12, WHITE);

    // ===== HUD INFO (left side) =====
    int hudX = 10;
    int hudY = 10;
    int lineHeight = 30;
    int startY = 40;

    DrawFPS(hudX, hudY);

    DrawText(TextFormat("Speed: %.1f m/s (%.1f km/h) (%.1f, %.1f, %.1f)",
        Vector3Length(plane.m_VelocityVector),
        Vector3Length(plane.m_VelocityVector) * 3.6f,
        plane.m_VelocityVector.x,
        plane.m_VelocityVector.y,
        plane.m_VelocityVector.z),
        hudX, startY, 20, BLACK);

    DrawText(TextFormat("Plane Position: (%.1f, %.1f, %.1f)",
        plane.m_position.x, plane.m_position.y, plane.m_position.z),
        hudX, startY + lineHeight, 20, BLACK);

    DrawText(TextFormat("Altitude: %.1f m", plane.altitude),
        hudX, startY + lineHeight * 2, 20, BLACK);

    DrawText(TextFormat("Throttle: %.0f%%", plane.throttle * 100),
        hudX, startY + lineHeight * 3, 20, BLACK);

    DrawText(TextFormat("Pitch: %.1f° Roll: %.1f° Yaw: %.1f°",
        -plane.pitch * RAD2DEG, plane.roll * RAD2DEG, plane.yaw * RAD2DEG),
        hudX, startY + lineHeight * 4, 20, BLACK);

    DrawText(TextFormat("Lift: %.1f kN Drag: %.1f kN Thrust: %.1f kN",
        plane.lift / 1000, plane.drag / 1000, plane.thrust / 1000),
        hudX, startY + lineHeight * 5, 20, BLACK);

    DrawText(TextFormat("Density: %.3f kg/m³ Press: %.0f Pa Temp: %.1f K",
        plane.density, plane.pressure, plane.temperature),
        hudX, startY + lineHeight * 6, 20, BLACK);

    DrawText(TextFormat("Wind Speed: %.1f m/s", plane.windSpeed),
        hudX, startY + lineHeight * 7, 20, BLACK);

    DrawText(TextFormat("Wind Direction: %.0f°", plane.windDirection * RAD2DEG),
        hudX, startY + lineHeight * 8, 20, BLACK);

    DrawText(TextFormat("ClMax: %.1f", plane.cl),
        hudX, startY + lineHeight * 9, 20, BLACK);

    DrawText(TextFormat("AOA: %.1f°", plane.angleOfAttack),
        hudX, startY + lineHeight * 10, 20, BLACK);

    DrawText("Controls: W/S = Throttle/Brake  Up/Down = Pitch  A/D = Roll  Q/E = Yaw",
        hudX, GetScreenHeight() - 30, 20, DARKGRAY);

    // ===== WIND MARKER on compass =====
    float windRad = (-(plane.windDirection + PI) + 90) * DEG2RAD;
    Vector2 windMarker = { x + cosf(windRad) * 70,
                           y + sinf(windRad) * 70 };
    DrawLineEx(Vector2{ (float)x, (float)y }, windMarker, 2, SKYBLUE);
    DrawCircleV(windMarker, 4, SKYBLUE);
    DrawText("WIND", windMarker.x - 15, windMarker.y - 15, 10, SKYBLUE);
}


int main()
{
    srand(time(0));
    InitWindow(1000, 500, "Physics Airplane - Realistic Flight Simulation");
    InitAudioDevice();

    SetTargetFPS(60);
    // Sound for engine
    Sound EngineSound = LoadSound("EngineSound.mp3");
    Sound EngineIsOn = LoadSound("EngineIsOn.mp3");

    Mesh groundMesh = GenMeshPlane(20000.0f, 20000.0f, 50, 50);
    Model groundModel = LoadModelFromMesh(groundMesh);

    // Create a texture that tiles well
    Image groundImage = GenImageColor(512, 512, DARKGREEN);
    // Add some noise/texture
    for (int i = 0; i < 512; i += 64) {
        for (int j = 0; j < 512; j += 64) {
            ImageDrawRectangle(&groundImage, i, j, 32, 32, GREEN);
        }
    }
    Texture2D groundTexture = LoadTextureFromImage(groundImage);
    SetTextureWrap(groundTexture, TEXTURE_WRAP_REPEAT);
    groundModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture;

    UnloadImage(groundImage);

    // Create airplane object
    PlaneObject plane;
    plane.m_position = Vector3{ 0.0f, 1.0f, 0.0f };
    plane.m_VelocityVector = Vector3{ 0.0f, 0.0f, 0.0f };
    plane.mass = 40000.0f;
    plane.roll = 0.0f;
    plane.pitch = 0.0f;
    plane.yaw = 0.0f;
    plane.throttle = 0.0f;
    plane.P_e = 40000000.0f;
    plane.wingArea = 122.0f;
    plane.cdZero = 0.05f;

    // Rotation matrix for model
    Matrix rotationX = MatrixRotateX(-90 * DEG2RAD);
    Matrix rotationY = MatrixRotateY(-90 * DEG2RAD);
    Matrix finalRotation = MatrixMultiply(rotationX, rotationY);

    // Camera
    Camera3D camera = { 0 };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    Vector3 cameraOffset = Vector3{ 0.0f, 5.0f, -19.0f };

    // Load model and textures
    Model planeModel = LoadModel("11803_Airplane_v1_l1.obj");

    Texture2D bodyTex = LoadTextureWithSTB("11803_Airplane_body_diff.png");
    Texture2D tailTex = LoadTextureWithSTB("11803_Airplane_tail_diff.png");
    Texture2D wingLeftTex = LoadTextureWithSTB("11803_Airplane_wing_big_L_diff.png");
    Texture2D wingRightTex = LoadTextureWithSTB("11803_Airplane_wing_big_R_diff.png");
    Texture2D wingDetailsLTex = LoadTextureWithSTB("11803_Airplane_wing_details_L_diff.png");
    Texture2D wingDetailsRTex = LoadTextureWithSTB("11803_Airplane_wing_details_R_diff.png");

    if (planeModel.materialCount >= 1) planeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = bodyTex;
    if (planeModel.materialCount >= 2) planeModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = wingRightTex;
    if (planeModel.materialCount >= 3) planeModel.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = wingDetailsRTex;
    if (planeModel.materialCount >= 4) planeModel.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = tailTex;
    if (planeModel.materialCount >= 5) planeModel.materials[4].maps[MATERIAL_MAP_DIFFUSE].texture = wingDetailsLTex;
    if (planeModel.materialCount >= 6) planeModel.materials[5].maps[MATERIAL_MAP_DIFFUSE].texture = wingLeftTex;

    float dt = 0.0f;

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        // Update controls
        UpdateControls(plane, dt);

        // Update physics
        UpdatePlanePhysics(plane, dt);

        float rollRad = plane.roll;
        float pitchRad = plane.pitch;
        float yawRad = plane.yaw;

        Matrix visualRotation = MatrixMultiply(
            MatrixRotateZ(rollRad),
            MatrixMultiply(
                MatrixRotateX(pitchRad),
                MatrixRotateY(yawRad)
            )
        );

        Matrix transform = MatrixMultiply(
            MatrixMultiply(
                MatrixMultiply(
                    MatrixScale(0.006f, 0.006f, 0.006f),
                    MatrixMultiply(finalRotation, visualRotation)
                ),
                MatrixTranslate(plane.m_position.x, plane.m_position.y, plane.m_position.z)
            ),
            MatrixIdentity()
        );

        // Camera follows the airplane
        Matrix camRotation = MatrixMultiply(
            MatrixRotateZ(plane.roll),
            MatrixMultiply(
                MatrixRotateX(plane.pitch),
                MatrixRotateY(plane.yaw)
            )
        );

        Vector3 rotatedOffset = Vector3Transform(cameraOffset, camRotation);
        camera.position.x = plane.m_position.x + rotatedOffset.x;
        camera.position.y = plane.m_position.y + rotatedOffset.y;
        camera.position.z = plane.m_position.z + rotatedOffset.z;
        camera.target = plane.m_position;

        camera.up = Vector3Transform(Vector3{ 0.0f, 1.0f, 0.0f }, camRotation);
        camera.up = Vector3Normalize(camera.up);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Draw ground model
        DrawModel(groundModel, Vector3{ 0.0f, -0.5f, 0.0f }, 1.0f, WHITE);
        DrawGrid(50000, 100.0f);

        // Play Sound of plane
        if (plane.throttle >= 0.1) {
            if (plane.m_position.y < 10) {
                if (!IsSoundPlaying(EngineSound)) {
                    PlaySound(EngineSound);
                }
                StopSound(EngineIsOn);
                SetSoundVolume(EngineSound, 0.3f + (plane.throttle * 0.7f));
            }
            else {
                if (!IsSoundPlaying(EngineIsOn)) {
                    PlaySound(EngineIsOn);
                }
                StopSound(EngineSound);
                SetSoundVolume(EngineIsOn, 0.3f + (plane.throttle * 0.7f));
            }
        }
        else {
            StopSound(EngineSound);
            StopSound(EngineIsOn);
        }

        // Draw the airplane
        for (int i = 0; i < planeModel.meshCount; i++)
        {
            DrawMesh(planeModel.meshes[i], planeModel.materials[planeModel.meshMaterial[i]], transform);
        }

        EndMode3D();
        DrawSimpleCompass(camera, plane);
        EndDrawing();
    }

    UnloadModel(planeModel);
    UnloadTexture(bodyTex);
    UnloadTexture(tailTex);
    UnloadTexture(wingLeftTex);
    UnloadTexture(wingRightTex);
    UnloadTexture(wingDetailsLTex);
    UnloadTexture(wingDetailsRTex);
    UnloadSound(EngineSound);
    UnloadSound(EngineIsOn);
    CloseWindow();
    return 0;
}