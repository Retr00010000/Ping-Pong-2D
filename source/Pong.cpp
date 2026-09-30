#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

#include "Texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// Helper to locate assets whether running from project root or Debug/
string FindAssetPath(const string& relPath) {
    FILE* f = fopen(relPath.c_str(), "rb");
    if (f) {
        fclose(f);
        return relPath;
    }
    string parentPath = "../" + relPath;
    f = fopen(parentPath.c_str(), "rb");
    if (f) {
        fclose(f);
        return parentPath;
    }
    return relPath;
}

// ==========================================
// CONSTANTS & GAME SETTINGS
// ==========================================
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

const float PLAYFIELD_TOP = 0.92f;
const float PLAYFIELD_BOTTOM = -0.92f;

const float PADDLE_WIDTH = 0.032f;
const float PADDLE_HEIGHT = 0.25f;
const float PADDLE_SPEED = 1.75f; // units per second

const float BALL_SIZE = 0.028f; // Tiny classic square ball
const float BALL_INITIAL_SPEED = 1.15f;
const float BALL_MAX_SPEED = 2.4f;
const float BALL_SPEED_INCREMENT = 1.05f;

const int WINNING_SCORE = 7; // First to 7 points wins the match

// Sound helper using Windows Beep without blocking the main game loop
void PlaySoundAsync(int freq, int durationMs) {
#ifdef _WIN32
    std::thread([=]() {
        Beep(freq, durationMs);
    }).detach();
#else
    (void)freq;
    (void)durationMs;
#endif
}

// ==========================================
// 5x3 RETRO BITMAP FONT
// ==========================================
// Each character is 5 rows high by 3 columns wide.
// Bit 2 is left column, Bit 1 is middle, Bit 0 is right column.
const uint8_t* GetCharBitmap(char c) {
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

    static const uint8_t FONT_0[5] = { 0b111, 0b101, 0b101, 0b101, 0b111 };
    static const uint8_t FONT_1[5] = { 0b010, 0b110, 0b010, 0b010, 0b111 };
    static const uint8_t FONT_2[5] = { 0b111, 0b001, 0b111, 0b100, 0b111 };
    static const uint8_t FONT_3[5] = { 0b111, 0b001, 0b111, 0b001, 0b111 };
    static const uint8_t FONT_4[5] = { 0b101, 0b101, 0b111, 0b001, 0b001 };
    static const uint8_t FONT_5[5] = { 0b111, 0b100, 0b111, 0b001, 0b111 };
    static const uint8_t FONT_6[5] = { 0b111, 0b100, 0b111, 0b101, 0b111 };
    static const uint8_t FONT_7[5] = { 0b111, 0b001, 0b001, 0b001, 0b001 };
    static const uint8_t FONT_8[5] = { 0b111, 0b101, 0b111, 0b101, 0b111 };
    static const uint8_t FONT_9[5] = { 0b111, 0b101, 0b111, 0b001, 0b111 };

    static const uint8_t FONT_A[5] = { 0b111, 0b101, 0b111, 0b101, 0b101 };
    static const uint8_t FONT_B[5] = { 0b110, 0b101, 0b110, 0b101, 0b110 };
    static const uint8_t FONT_C[5] = { 0b111, 0b100, 0b100, 0b100, 0b111 };
    static const uint8_t FONT_D[5] = { 0b110, 0b101, 0b101, 0b101, 0b110 };
    static const uint8_t FONT_E[5] = { 0b111, 0b100, 0b111, 0b100, 0b111 };
    static const uint8_t FONT_F[5] = { 0b111, 0b100, 0b110, 0b100, 0b100 };
    static const uint8_t FONT_G[5] = { 0b111, 0b100, 0b101, 0b101, 0b111 };
    static const uint8_t FONT_H[5] = { 0b101, 0b101, 0b111, 0b101, 0b101 };
    static const uint8_t FONT_I[5] = { 0b111, 0b010, 0b010, 0b010, 0b111 };
    static const uint8_t FONT_J[5] = { 0b001, 0b001, 0b001, 0b101, 0b111 };
    static const uint8_t FONT_K[5] = { 0b101, 0b110, 0b100, 0b110, 0b101 };
    static const uint8_t FONT_L[5] = { 0b100, 0b100, 0b100, 0b100, 0b111 };
    static const uint8_t FONT_M[5] = { 0b101, 0b111, 0b101, 0b101, 0b101 };
    static const uint8_t FONT_N[5] = { 0b101, 0b111, 0b111, 0b101, 0b101 };
    static const uint8_t FONT_O[5] = { 0b111, 0b101, 0b101, 0b101, 0b111 };
    static const uint8_t FONT_P[5] = { 0b111, 0b101, 0b111, 0b100, 0b100 };
    static const uint8_t FONT_Q[5] = { 0b111, 0b101, 0b101, 0b111, 0b001 };
    static const uint8_t FONT_R[5] = { 0b111, 0b101, 0b110, 0b101, 0b101 };
    static const uint8_t FONT_S[5] = { 0b111, 0b100, 0b111, 0b001, 0b111 };
    static const uint8_t FONT_T[5] = { 0b111, 0b010, 0b010, 0b010, 0b010 };
    static const uint8_t FONT_U[5] = { 0b101, 0b101, 0b101, 0b101, 0b111 };
    static const uint8_t FONT_V[5] = { 0b101, 0b101, 0b101, 0b101, 0b010 };
    static const uint8_t FONT_W[5] = { 0b101, 0b101, 0b101, 0b111, 0b101 };
    static const uint8_t FONT_X[5] = { 0b101, 0b101, 0b010, 0b101, 0b101 };
    static const uint8_t FONT_Y[5] = { 0b101, 0b101, 0b010, 0b010, 0b010 };
    static const uint8_t FONT_Z[5] = { 0b111, 0b001, 0b010, 0b100, 0b111 };

    static const uint8_t FONT_COLON[5] = { 0b000, 0b010, 0b000, 0b010, 0b000 };
    static const uint8_t FONT_DASH[5]  = { 0b000, 0b000, 0b111, 0b000, 0b000 };
    static const uint8_t FONT_EXCL[5]  = { 0b010, 0b010, 0b010, 0b000, 0b010 };
    static const uint8_t FONT_SLASH[5] = { 0b001, 0b001, 0b010, 0b100, 0b100 };
    static const uint8_t FONT_DOT[5]   = { 0b000, 0b000, 0b000, 0b000, 0b010 };
    static const uint8_t FONT_SPACE[5] = { 0b000, 0b000, 0b000, 0b000, 0b000 };

    switch (c) {
        case '0': return FONT_0; case '1': return FONT_1; case '2': return FONT_2;
        case '3': return FONT_3; case '4': return FONT_4; case '5': return FONT_5;
        case '6': return FONT_6; case '7': return FONT_7; case '8': return FONT_8;
        case '9': return FONT_9;
        case 'A': return FONT_A; case 'B': return FONT_B; case 'C': return FONT_C;
        case 'D': return FONT_D; case 'E': return FONT_E; case 'F': return FONT_F;
        case 'G': return FONT_G; case 'H': return FONT_H; case 'I': return FONT_I;
        case 'J': return FONT_J; case 'K': return FONT_K; case 'L': return FONT_L;
        case 'M': return FONT_M; case 'N': return FONT_N; case 'O': return FONT_O;
        case 'P': return FONT_P; case 'Q': return FONT_Q; case 'R': return FONT_R;
        case 'S': return FONT_S; case 'T': return FONT_T; case 'U': return FONT_U;
        case 'V': return FONT_V; case 'W': return FONT_W; case 'X': return FONT_X;
        case 'Y': return FONT_Y; case 'Z': return FONT_Z;
        case ':': return FONT_COLON;
        case '-': return FONT_DASH;
        case '!': return FONT_EXCL;
        case '/': return FONT_SLASH;
        case '.': return FONT_DOT;
        default:  return FONT_SPACE;
    }
}

// Render a single character using small quads
void RenderChar(char c, float startX, float startY, float pixelW, float pixelH, GLuint offsetLoc, GLuint scaleLoc) {
    const uint8_t* rows = GetCharBitmap(c);
    glUniform2f(scaleLoc, pixelW * 0.95f, pixelH * 0.95f);

    for (int r = 0; r < 5; r++) {
        uint8_t rowVal = rows[r];
        float py = startY + (4 - r) * pixelH;

        if (rowVal & 0b100) {
            glUniform2f(offsetLoc, startX, py);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        if (rowVal & 0b010) {
            glUniform2f(offsetLoc, startX + pixelW, py);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        if (rowVal & 0b001) {
            glUniform2f(offsetLoc, startX + 2.0f * pixelW, py);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}

// Render a text string
void RenderString(const string& text, float startX, float startY, float pixelW, float pixelH, GLuint offsetLoc, GLuint scaleLoc) {
    float curX = startX;
    float charAdvance = 4.0f * pixelW;

    for (char c : text) {
        RenderChar(c, curX, startY, pixelW, pixelH, offsetLoc, scaleLoc);
        curX += charAdvance;
    }
}

// Helper to calculate string width
float GetStringWidth(const string& text, float pixelW) {
    if (text.empty()) return 0.0f;
    return (float)text.length() * (4.0f * pixelW) - pixelW;
}

// ==========================================
// MAIN FUNCTION & GAME LOOP
// ==========================================
int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ping Pong 2D", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 2. Load Shaders & Textures
    Shader pongShader(FindAssetPath("assets/shaders/pong.vert").c_str(), FindAssetPath("assets/shaders/pong.frag").c_str());
    Shader bgShader(FindAssetPath("assets/shaders/default.vert").c_str(), FindAssetPath("assets/shaders/default.frag").c_str());

    Texture bgTexture(FindAssetPath("assets/textures/background.png").c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    bgTexture.texUnit(bgShader, "tex0", 0);

    // Fullscreen Background Quad [-1, 1] x [-1, 1]
    GLfloat bgVertices[] = {
        // Positions   // TexCoords
        -1.0f, -1.0f,  0.0f, 0.0f, // Bottom-Left
         1.0f, -1.0f,  1.0f, 0.0f, // Bottom-Right
         1.0f,  1.0f,  1.0f, 1.0f, // Top-Right
        -1.0f,  1.0f,  0.0f, 1.0f  // Top-Left
    };

    GLuint bgIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    VAO bgVAO;
    bgVAO.Bind();
    VBO bgVBO(bgVertices, sizeof(bgVertices));
    EBO bgEBO(bgIndices, sizeof(bgIndices));
    bgVAO.LinkAttrib(bgVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    bgVAO.LinkAttrib(bgVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    bgVAO.Unbind();
    bgVBO.Unbind();
    bgEBO.Unbind();

    // 3. Unit Quad Setup [0, 1] x [0, 1]
    GLfloat quadVertices[] = {
        0.0f, 0.0f, // Bottom-Left
        1.0f, 0.0f, // Bottom-Right
        1.0f, 1.0f, // Top-Right
        0.0f, 1.0f  // Top-Left
    };

    GLuint quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    VAO quadVAO;
    quadVAO.Bind();
    VBO quadVBO(quadVertices, sizeof(quadVertices));
    EBO quadEBO(quadIndices, sizeof(quadIndices));
    quadVAO.LinkAttrib(quadVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    quadVAO.Unbind();
    quadVBO.Unbind();
    quadEBO.Unbind();

    // 4. Uniform Locations
    pongShader.Activate();
    GLuint offsetLoc = glGetUniformLocation(pongShader.ID, "offset");
    GLuint scaleLoc  = glGetUniformLocation(pongShader.ID, "scale");
    GLuint colorLoc  = glGetUniformLocation(pongShader.ID, "color");

    // 5. Game State Variables
    // Paddles: Y position is centered
    float p1_x = -0.92f;
    float p1_y = 0.0f;

    float p2_x = 0.92f - PADDLE_WIDTH;
    float p2_y = 0.0f;

    // Ball
    float ball_x = 0.0f;
    float ball_y = 0.0f;
    float ball_vx = 0.0f;
    float ball_vy = 0.0f;
    float currentBallSpeed = BALL_INITIAL_SPEED;

    // Scores & Match Wins
    int p1_score = 0;
    int p2_score = 0;
    int p1_wins = 0;
    int p2_wins = 0;

    bool matchOver = false;
    int winner = 0; // 1 or 2

    bool isPaused = false;
    bool pKeyWasPressed = false;

    // Serve / Pause Timer
    float serveTimer = 1.0f; // Wait 1 sec before initial serve
    int nextServeDirection = 1; // 1 = towards P2, -1 = towards P1

    srand((unsigned int)time(NULL));

    auto ResetBall = [&](int serveDir) {
        ball_x = 0.0f;
        ball_y = 0.0f;
        currentBallSpeed = BALL_INITIAL_SPEED;

        // Angle between -35 and +35 degrees
        float angle = ((float)(rand() % 70) - 35.0f) * (3.14159265f / 180.0f);
        ball_vx = serveDir * currentBallSpeed * cos(angle);
        ball_vy = currentBallSpeed * sin(angle);

        serveTimer = 0.8f;
    };

    auto ResetRound = [&]() {
        p1_score = 0;
        p2_score = 0;
        p1_y = 0.0f;
        p2_y = 0.0f;
        matchOver = false;
        winner = 0;
        isPaused = false;
        ResetBall(nextServeDirection);
    };

    ResetRound();

    float lastFrameTime = (float)glfwGetTime();

    // 6. Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = (float)glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        if (deltaTime > 0.05f) deltaTime = 0.05f; // Cap delta to prevent tunnelling

        // ==========================================
        // 1. INPUT HANDLING
        // ==========================================
        // Pause Toggle on 'P'
        bool pKeyPressed = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
        if (pKeyPressed && !pKeyWasPressed) {
            isPaused = !isPaused;
        }
        pKeyWasPressed = pKeyPressed;

        if (!matchOver && !isPaused) {
            // Player 1 (W / S)
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                p1_y += PADDLE_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                p1_y -= PADDLE_SPEED * deltaTime;
            }

            // Player 2 (UP / DOWN arrows)
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                p2_y += PADDLE_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                p2_y -= PADDLE_SPEED * deltaTime;
            }
        }
        else if (matchOver) {
            // Restart match on SPACE or ENTER
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                ResetRound();
            }
        }

        // Reset all stats on R
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            p1_wins = 0;
            p2_wins = 0;
            ResetRound();
        }

        // Clamp Paddles within playfield
        float halfPaddleH = PADDLE_HEIGHT / 2.0f;
        if (p1_y + halfPaddleH > PLAYFIELD_TOP)    p1_y = PLAYFIELD_TOP - halfPaddleH;
        if (p1_y - halfPaddleH < PLAYFIELD_BOTTOM) p1_y = PLAYFIELD_BOTTOM + halfPaddleH;

        if (p2_y + halfPaddleH > PLAYFIELD_TOP)    p2_y = PLAYFIELD_TOP - halfPaddleH;
        if (p2_y - halfPaddleH < PLAYFIELD_BOTTOM) p2_y = PLAYFIELD_BOTTOM + halfPaddleH;

        // ==========================================
        // 2. BALL PHYSICS & GAME LOGIC
        // ==========================================
        if (!matchOver && !isPaused) {
            if (serveTimer > 0.0f) {
                serveTimer -= deltaTime;
            }
            else {
                // Move Ball
                ball_x += ball_vx * deltaTime;
                ball_y += ball_vy * deltaTime;

                float halfBall = BALL_SIZE / 2.0f;

                // Bounce off Top and Bottom walls
                if (ball_y + halfBall >= PLAYFIELD_TOP) {
                    ball_y = PLAYFIELD_TOP - halfBall;
                    ball_vy = -abs(ball_vy);
                    PlaySoundAsync(320, 20); // Wall bounce
                }
                else if (ball_y - halfBall <= PLAYFIELD_BOTTOM) {
                    ball_y = PLAYFIELD_BOTTOM + halfBall;
                    ball_vy = abs(ball_vy);
                    PlaySoundAsync(320, 20); // Wall bounce
                }

                // Collision with Player 1 Paddle (Left)
                float p1_top = p1_y + halfPaddleH;
                float p1_bot = p1_y - halfPaddleH;
                float p1_right = p1_x + PADDLE_WIDTH;

                if (ball_vx < 0.0f &&
                    ball_x - halfBall <= p1_right &&
                    ball_x + halfBall >= p1_x &&
                    ball_y + halfBall >= p1_bot &&
                    ball_y - halfBall <= p1_top)
                {
                    ball_x = p1_right + halfBall; // push out
                    currentBallSpeed = min(currentBallSpeed * BALL_SPEED_INCREMENT, BALL_MAX_SPEED);

                    // Angle reflection based on where ball hits paddle (-1 to 1)
                    float relativeHit = (ball_y - p1_y) / halfPaddleH;
                    if (relativeHit > 1.0f) relativeHit = 1.0f;
                    if (relativeHit < -1.0f) relativeHit = -1.0f;

                    float bounceAngle = relativeHit * (55.0f * 3.14159265f / 180.0f);
                    ball_vx = currentBallSpeed * cos(bounceAngle);
                    ball_vy = currentBallSpeed * sin(bounceAngle);

                    PlaySoundAsync(540, 30); // Paddle hit
                }

                // Collision with Player 2 Paddle (Right)
                float p2_top = p2_y + halfPaddleH;
                float p2_bot = p2_y - halfPaddleH;
                float p2_left = p2_x;

                if (ball_vx > 0.0f &&
                    ball_x + halfBall >= p2_left &&
                    ball_x - halfBall <= p2_x + PADDLE_WIDTH &&
                    ball_y + halfBall >= p2_bot &&
                    ball_y - halfBall <= p2_top)
                {
                    ball_x = p2_left - halfBall; // push out
                    currentBallSpeed = min(currentBallSpeed * BALL_SPEED_INCREMENT, BALL_MAX_SPEED);

                    float relativeHit = (ball_y - p2_y) / halfPaddleH;
                    if (relativeHit > 1.0f) relativeHit = 1.0f;
                    if (relativeHit < -1.0f) relativeHit = -1.0f;

                    float bounceAngle = relativeHit * (55.0f * 3.14159265f / 180.0f);
                    ball_vx = -currentBallSpeed * cos(bounceAngle);
                    ball_vy = currentBallSpeed * sin(bounceAngle);

                    PlaySoundAsync(540, 30); // Paddle hit
                }

                // SCORING: Ball leaves left side -> Player 2 scores
                if (ball_x < -1.08f) {
                    p2_score++;
                    PlaySoundAsync(750, 100);

                    if (p2_score >= WINNING_SCORE) {
                        p2_wins++;
                        matchOver = true;
                        winner = 2;
                        PlaySoundAsync(880, 250);
                    }
                    else {
                        nextServeDirection = 1; // serve to scoring side or alternate
                        ResetBall(1);
                    }
                }
                // SCORING: Ball leaves right side -> Player 1 scores
                else if (ball_x > 1.08f) {
                    p1_score++;
                    PlaySoundAsync(750, 100);

                    if (p1_score >= WINNING_SCORE) {
                        p1_wins++;
                        matchOver = true;
                        winner = 1;
                        PlaySoundAsync(880, 250);
                    }
                    else {
                        nextServeDirection = -1;
                        ResetBall(-1);
                    }
                }
            }
        }

        // Update Window Title
        string title = "Ping Pong 2D | P1: " + to_string(p1_score) + " (Wins: " + to_string(p1_wins) + ")"
                     + "  -  P2: " + to_string(p2_score) + " (Wins: " + to_string(p2_wins) + ")"
                     + (isPaused ? " [PAUSED - PRESS P TO RESUME]" : (matchOver ? " [PRESS SPACE]" : ""));
        glfwSetWindowTitle(window, title.c_str());

        // ==========================================
        // 3. RENDERING
        // ==========================================
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // A. Draw Background Texture
        bgShader.Activate();
        bgTexture.Bind();
        bgVAO.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        bgVAO.Unbind();
        bgTexture.Unbind();

        // B. Draw Game Elements
        pongShader.Activate();
        quadVAO.Bind();

        // White color for classic Pong
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

        // A. Draw Top and Bottom Playfield Borders
        float borderH = 0.02f;
        // Top border
        glUniform2f(scaleLoc, 2.0f, borderH);
        glUniform2f(offsetLoc, -1.0f, PLAYFIELD_TOP);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Bottom border
        glUniform2f(scaleLoc, 2.0f, borderH);
        glUniform2f(offsetLoc, -1.0f, PLAYFIELD_BOTTOM - borderH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // B. Draw Center Dotted / Dashed Net
        float dashW = 0.012f;
        float dashH = 0.045f;
        float dashGap = 0.035f;
        float netX = -dashW / 2.0f;

        glUniform2f(scaleLoc, dashW, dashH);
        for (float y = PLAYFIELD_BOTTOM + 0.01f; y < PLAYFIELD_TOP - 0.02f; y += (dashH + dashGap)) {
            glUniform2f(offsetLoc, netX, y);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // C. Draw Paddles
        // Player 1 Paddle (Left)
        glUniform2f(scaleLoc, PADDLE_WIDTH, PADDLE_HEIGHT);
        glUniform2f(offsetLoc, p1_x, p1_y - halfPaddleH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Player 2 Paddle (Right)
        glUniform2f(scaleLoc, PADDLE_WIDTH, PADDLE_HEIGHT);
        glUniform2f(offsetLoc, p2_x, p2_y - halfPaddleH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // D. Draw Ball
        glUniform2f(scaleLoc, BALL_SIZE, BALL_SIZE);
        glUniform2f(offsetLoc, ball_x - BALL_SIZE / 2.0f, ball_y - BALL_SIZE / 2.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // E. Draw Big Retro Score Numbers
        // Player 1 Score (Left of center net)
        string p1ScoreStr = to_string(p1_score);
        float scorePixelW = 0.022f;
        float scorePixelH = 0.026f;
        float p1ScoreW = GetStringWidth(p1ScoreStr, scorePixelW);
        RenderString(p1ScoreStr, -0.25f - p1ScoreW, 0.62f, scorePixelW, scorePixelH, offsetLoc, scaleLoc);

        // Player 2 Score (Right of center net)
        string p2ScoreStr = to_string(p2_score);
        RenderString(p2ScoreStr, 0.25f, 0.62f, scorePixelW, scorePixelH, offsetLoc, scaleLoc);

        // F. Draw Win Tracker on Each Player's Side
        // Player 1 Win Tracker (Top Left)
        float trackerPixelW = 0.010f;
        float trackerPixelH = 0.012f;

        string p1WinStr = "WINS: " + to_string(p1_wins);
        RenderString(p1WinStr, -0.85f, 0.82f, trackerPixelW, trackerPixelH, offsetLoc, scaleLoc);

        // Player 1 Win Tally Blocks
        for (int i = 0; i < p1_wins && i < 10; i++) {
            float boxSize = 0.018f;
            float boxX = -0.85f + i * 0.026f;
            float boxY = 0.77f;
            glUniform2f(scaleLoc, boxSize, boxSize);
            glUniform2f(offsetLoc, boxX, boxY);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // Player 2 Win Tracker (Top Right)
        string p2WinStr = "WINS: " + to_string(p2_wins);
        float p2WinStrW = GetStringWidth(p2WinStr, trackerPixelW);
        RenderString(p2WinStr, 0.85f - p2WinStrW, 0.82f, trackerPixelW, trackerPixelH, offsetLoc, scaleLoc);

        // Player 2 Win Tally Blocks
        for (int i = 0; i < p2_wins && i < 10; i++) {
            float boxSize = 0.018f;
            float boxX = (0.85f - boxSize) - i * 0.026f;
            float boxY = 0.77f;
            glUniform2f(scaleLoc, boxSize, boxSize);
            glUniform2f(offsetLoc, boxX, boxY);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // G. Draw Controls Helper (Bottom Left & Bottom Right)
        float tipPixelW = 0.007f;
        float tipPixelH = 0.009f;
        glUniform3f(colorLoc, 0.65f, 0.65f, 0.65f); // subtle dim gray

        RenderString("P1: W / S  |  P: PAUSE", -0.85f, -0.88f, tipPixelW, tipPixelH, offsetLoc, scaleLoc);

        string p2TipStr = "P2: UP / DN";
        float p2TipW = GetStringWidth(p2TipStr, tipPixelW);
        RenderString(p2TipStr, 0.85f - p2TipW, -0.88f, tipPixelW, tipPixelH, offsetLoc, scaleLoc);

        // H. Winner Banner when Match is Over
        if (matchOver) {
            glUniform3f(colorLoc, 1.0f, 1.0f, 0.2f); // Retro Gold

            string winBanner = (winner == 1) ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!";
            float bannerPixelW = 0.018f;
            float bannerPixelH = 0.022f;
            float bannerW = GetStringWidth(winBanner, bannerPixelW);
            RenderString(winBanner, -bannerW / 2.0f, 0.12f, bannerPixelW, bannerPixelH, offsetLoc, scaleLoc);

            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // White
            string restartPrompt = "PRESS SPACE TO PLAY NEXT MATCH";
            float subPixelW = 0.009f;
            float subPixelH = 0.011f;
            float subW = GetStringWidth(restartPrompt, subPixelW);
            RenderString(restartPrompt, -subW / 2.0f, -0.05f, subPixelW, subPixelH, offsetLoc, scaleLoc);

            string resetPrompt = "PRESS R TO RESET ALL WINS";
            float rW = GetStringWidth(resetPrompt, subPixelW);
            RenderString(resetPrompt, -rW / 2.0f, -0.15f, subPixelW, subPixelH, offsetLoc, scaleLoc);
        }
        else if (isPaused) {
            // Retro Frozen / Paused Banner
            glUniform3f(colorLoc, 0.2f, 0.9f, 1.0f); // Ice Cyan

            string pauseBanner = "PAUSED";
            float pausePixelW = 0.022f;
            float pausePixelH = 0.028f;
            float pauseW = GetStringWidth(pauseBanner, pausePixelW);
            RenderString(pauseBanner, -pauseW / 2.0f, 0.08f, pausePixelW, pausePixelH, offsetLoc, scaleLoc);

            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // White
            string resumePrompt = "PRESS P TO RESUME";
            float subPixelW = 0.009f;
            float subPixelH = 0.011f;
            float subW = GetStringWidth(resumePrompt, subPixelW);
            RenderString(resumePrompt, -subW / 2.0f, -0.04f, subPixelW, subPixelH, offsetLoc, scaleLoc);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    bgTexture.Delete();
    bgVAO.Delete();
    bgVBO.Delete();
    bgEBO.Delete();
    bgShader.Delete();

    quadVAO.Delete();
    quadVBO.Delete();
    quadEBO.Delete();
    pongShader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
