/*******************************************************************************************
*
*   raylib [core] example - window scale letterbox (and virtual mouse)
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Anata (@anatagawa) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Anata (@anatagawa) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define MAX_GRID_SIZE 1000
#define CELL_SIZE 100

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))

bool grid[MAX_GRID_SIZE][MAX_GRID_SIZE] = { false };
bool gridNext[MAX_GRID_SIZE][MAX_GRID_SIZE] = { false };
int currentGridSize = 50;

// Clamp Vector2 value with min and max and return a new vector2
// NOTE: Required for virtual mouse, to clamp inside virtual game size
Vector2 ClampValue(Vector2 value, Vector2 min, Vector2 max)
{
    Vector2 result = value;
    result.x = (result.x > max.x)? max.x : result.x;
    result.x = (result.x < min.x)? min.x : result.x;
    result.y = (result.y > max.y)? max.y : result.y;
    result.y = (result.y < min.y)? min.y : result.y;
    return result;
}

int getNumberOfNeighbor(int x, int y) {
    int count = 0;

    for (int _x = x -1; _x < x +2; _x++) {
        for (int _y = y -1; _y < y +2; _y++) {
            if (_x < currentGridSize && _y < currentGridSize && _x > -1 && _y > -1) {
                if (!(_x == x && _y == y)) {
                    if (grid[_x][_y]) {
                        count += 1;
                    }
                }
                
            }
        }
    }

    return count;
}

int main(void)
{
    const int windowWidth = 800;
    const int windowHeight = 450;

    // Enable config flags for resizable window and vertical synchro
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(windowWidth, windowHeight, "raylib [core] example - window scale letterbox");
    SetWindowMinSize(320, 240);

    int gameScreenWidth = 1920;
    int gameScreenHeight = 1080;

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0.0f , 0.0f };
    camera.offset = (Vector2){ gameScreenWidth/2.0f, gameScreenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Render texture initialization, used to hold the rendering result so we can easily resize it
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);  // Texture scale filter to use

    Color colors[10] = { 0 };
    for (int i = 0; i < 10; i++) colors[i] = (Color){ GetRandomValue(100, 250), GetRandomValue(50, 150), GetRandomValue(10, 100), 255 };

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------


    for (int x = 0; x < MAX_GRID_SIZE; x++) {
        for (int y = 0; y < MAX_GRID_SIZE; y++) {
            grid[x][y] = false;
        }
    }
    

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {

        if (IsKeyPressed(KEY_EQUAL)) {
            currentGridSize+= 1;
        }
        if (IsKeyPressed(KEY_MINUS)) {
            currentGridSize -= 1;
        }

        int count = 0;

        if (IsKeyDown(KEY_UP)) camera.target.y -= 900 * GetFrameTime();
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 900 * GetFrameTime();
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 900 * GetFrameTime();
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 900 * GetFrameTime();

        camera.zoom += GetMouseWheelMove() * 5 * GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            for (int _x = 0; _x < currentGridSize; _x++) {
                for (int _y = 0; _y < currentGridSize; _y++) {
                    count = getNumberOfNeighbor(_x, _y);
                    if (grid[_x][_y]) {
                        if (!(count == 3 || count == 2)) {
                            gridNext[_x][_y] = false;
                        } else{
                            gridNext[_x][_y] = true;
                        }
                    } else {
                        if (count == 3) {
                            gridNext[_x][_y] = true;
                        } else {
                            gridNext[_x][_y] = false;
                        }
                    }
                }
            }

            for (int _x = 0; _x < currentGridSize; _x++) {
                for (int _y = 0; _y < currentGridSize; _y++) {
                    grid[_x][_y] = gridNext[_x][_y];
                }
            }
        }



        // Update
        //----------------------------------------------------------------------------------
        // Compute required framebuffer scaling
        float scale = min((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);

        if (IsKeyPressed(KEY_SPACE))
        {
            // Recalculate random colors for the bars
            for (int i = 0; i < 10; i++) colors[i] = (Color){ GetRandomValue(100, 250), GetRandomValue(50, 150), GetRandomValue(10, 100), 255 };
        }

        // Update virtual mouse (clamped mouse value behind game screen)
        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (gameScreenWidth*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (gameScreenHeight*scale))*0.5f)/scale;
        virtualMouse = ClampValue(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)gameScreenWidth, (float)gameScreenHeight });

        // Apply the same transformation as the virtual mouse to the real mouse (i.e. to work with raygui)
        //SetMouseOffset(-(GetScreenWidth() - (gameScreenWidth*scale))*0.5f, -(GetScreenHeight() - (gameScreenHeight*scale))*0.5f);
        //SetMouseScale(1/scale, 1/scale);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Draw everything in the render texture, note this will not be rendered on screen, yet

        int cellx =  ceil(GetScreenToWorld2D(virtualMouse, camera).x / (CELL_SIZE)) - 1;
        int celly =  ceil(GetScreenToWorld2D(virtualMouse, camera).y / (CELL_SIZE)) - 1;


        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            if (cellx < currentGridSize && celly < currentGridSize && cellx > -1 && celly > -1) 
            grid[cellx][celly] = true;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (cellx < currentGridSize && celly < currentGridSize && cellx > -1 && celly > -1) 
            grid[cellx][celly] = false;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            printf("X: %d, Y: %d\n", cellx, celly);
            printf("Count: %d\n", getNumberOfNeighbor(cellx,celly));
        }

        BeginTextureMode(target);
            ClearBackground(RAYWHITE);  // Clear render texture background color
            BeginMode2D(camera);
                DrawRectangleLinesEx(
                    (Rectangle){
                        0,0, currentGridSize * CELL_SIZE, currentGridSize * CELL_SIZE
                    },
                    10,
                    BLACK
                );
                for (int x = 0; x < currentGridSize; x++) {
                    for (int y = 0; y < currentGridSize; y++) {
                        if (grid[x][y]) {
                            DrawRectangle(x * CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
                        }
                    }
                }
            EndMode2D();
        EndTextureMode();
        
        BeginDrawing();
            ClearBackground(BLACK);     // Clear screen background

            // Draw render texture to screen, properly scaled
            DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                           (Rectangle){ (GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f, (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
                           (float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
        //--------------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);        // Unload render texture

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}