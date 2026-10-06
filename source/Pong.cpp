#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

using namespace std;

#include "EBO.h"
#include "Texture.h"
#include "VAO.h"
#include "VBO.h"
#include "shaderClass.h"

// helper to locate assets whether running from project root or Debug/
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

// game constants and settings
const int Window_width= 800;
const int Window_height= 800;

const float Playfield_top= 0.92f;
const float Playfield_bottom= -0.92f;
const float Border_height= 0.02f;

const float Paddle_width= 0.032f;
const float Paddle_height= 0.25f;

// how fast paddles move per second
const float Paddle_speed= 1.75f;

const float Ball_size= 0.028f;
const float Ball_initial_speed= 1.15f;
const float Ball_max_speed= 2.40f;
const float Ball_speed_increment= 1.05f;

// score needed to win a match
const int Winning_score= 7;

// 5x3 pixel bitmap font definitions for drawing text
const uint8_t *GetCharBitmap(char c) {
  if (c >= 'a' && c <= 'z') {
    c= c - 'a' + 'A';
  }

  static const uint8_t BLANK[5]= {0, 0, 0, 0, 0};

  static const struct {
    char ch;
    uint8_t rows[5];
  } GLYPHS[]= {     // digits
                {'0', {0b111, 0b101, 0b101, 0b101, 0b111}},
                {'1', {0b010, 0b110, 0b010, 0b010, 0b111}},
                {'2', {0b111, 0b001, 0b111, 0b100, 0b111}},
                {'3', {0b111, 0b001, 0b111, 0b001, 0b111}},
                {'4', {0b101, 0b101, 0b111, 0b001, 0b001}},
                {'5', {0b111, 0b100, 0b111, 0b001, 0b111}},
                {'6', {0b111, 0b100, 0b111, 0b101, 0b111}},
                {'7', {0b111, 0b001, 0b001, 0b001, 0b001}},
                {'8', {0b111, 0b101, 0b111, 0b101, 0b111}},
                {'9', {0b111, 0b101, 0b111, 0b001, 0b111}},

                // alphabet
                {'A', {0b111, 0b101, 0b111, 0b101, 0b101}},
                {'B', {0b110, 0b101, 0b110, 0b101, 0b110}},
                {'C', {0b111, 0b100, 0b100, 0b100, 0b111}},
                {'D', {0b110, 0b101, 0b101, 0b101, 0b110}},
                {'E', {0b111, 0b100, 0b111, 0b100, 0b111}},
                {'F', {0b111, 0b100, 0b110, 0b100, 0b100}},
                {'G', {0b111, 0b100, 0b101, 0b101, 0b111}},
                {'H', {0b101, 0b101, 0b111, 0b101, 0b101}},
                {'I', {0b111, 0b010, 0b010, 0b010, 0b111}},
                {'J', {0b001, 0b001, 0b001, 0b101, 0b111}},
                {'K', {0b101, 0b110, 0b100, 0b110, 0b101}},
                {'L', {0b100, 0b100, 0b100, 0b100, 0b111}},
                {'M', {0b101, 0b111, 0b101, 0b101, 0b101}},
                {'N', {0b101, 0b111, 0b111, 0b101, 0b101}},
                {'O', {0b111, 0b101, 0b101, 0b101, 0b111}},
                {'P', {0b111, 0b101, 0b111, 0b100, 0b100}},
                {'Q', {0b111, 0b101, 0b101, 0b111, 0b001}},
                {'R', {0b111, 0b101, 0b110, 0b101, 0b101}},
                {'S', {0b111, 0b100, 0b111, 0b001, 0b111}},
                {'T', {0b111, 0b010, 0b010, 0b010, 0b010}},
                {'U', {0b101, 0b101, 0b101, 0b101, 0b111}},
                {'V', {0b101, 0b101, 0b101, 0b101, 0b010}},
                {'W', {0b101, 0b101, 0b101, 0b111, 0b101}},
                {'X', {0b101, 0b101, 0b010, 0b101, 0b101}},
                {'Y', {0b101, 0b101, 0b010, 0b010, 0b010}},
                {'Z', {0b111, 0b001, 0b010, 0b100, 0b111}},

                // symbols
                {':', {0b000, 0b010, 0b000, 0b010, 0b000}},
                {'-', {0b000, 0b000, 0b111, 0b000, 0b000}},
                {'!', {0b010, 0b010, 0b010, 0b000, 0b010}},
                {'/', {0b001, 0b001, 0b010, 0b100, 0b100}},
                {'.', {0b000, 0b000, 0b000, 0b000, 0b010}}};

  int totalGlyphs= sizeof(GLYPHS) / sizeof(GLYPHS[0]);
  for (int i= 0; i < totalGlyphs; i++) {
    if (GLYPHS[i].ch == c) {
      return GLYPHS[i].rows;
    }
  }
  return BLANK;
}

// vertices for our full-screen background quad
GLfloat bgVertices[]= {
    -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    1.0f,  -1.0f, 1.0f, 0.0f, // bottom-right
    1.0f,  1.0f,  1.0f, 1.0f, // top-right
    -1.0f, 1.0f,  0.0f, 1.0f  // top-left
};

// basic unit quad vertices for rendering things in the game
GLfloat quadVertices[]= {
    0.0f, 0.0f, // bottom-left
    1.0f, 0.0f, // bottom-right
    1.0f, 1.0f, // top-right
    0.0f, 1.0f  // top-left
};

// two triangles that form our rectangle
GLuint quadIndices[]= {
    0, 1, 2, // first triangle
    2, 3, 0  // second triangle
};

// shader uniform locations
GLuint offsetLoc;
GLuint scaleLoc;
GLuint colorLoc;

// quick helpers to draw colored rectangles and text
void SetColor(float r, float g, float b) {
  glUniform3f(colorLoc, r, g, b); 
}

void DrawRect(float x, float y, float w, float h) {
  glUniform2f(scaleLoc, w, h);
  glUniform2f(offsetLoc, x, y);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DrawChar(char c, float startX, float startY, float pixelW, float pixelH) {
  const uint8_t *rows= GetCharBitmap(c);
  glUniform2f(scaleLoc, pixelW * 0.95f, pixelH * 0.95f);

  for (int row= 0; row < 5; row++) {
    float py= startY + (4 - row) * pixelH;
    for (int col= 0; col < 3; col++) {
      bool isPixelOn= (rows[row] & (1 << (2 - col))) != 0;
      if (isPixelOn) {
        glUniform2f(offsetLoc, startX + col * pixelW, py);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
    }
  }
}

void DrawString(const string &text, float startX, float startY, float pixelW, float pixelH) {
  float curX= startX;
  float charAdvance= 4.0f * pixelW;
  for (int i= 0; i < text.length(); i++) {
    DrawChar(text[i], curX, startY, pixelW, pixelH);
    curX += charAdvance;
  }
}

float GetStringWidth(const string &text, float pixelW) {
  if (text.empty()) {
    return 0.0f;
  }
  return (float)text.length() * (4.0f * pixelW) - pixelW;
}

// game objects and state
struct Paddle {
  float x;
  float y= 0.0f;
  int score= 0;
  int wins= 0;

  void Move(float amount) {
    y += amount;
    float halfH= Paddle_height / 2.0f;
    y= clamp(y, Playfield_bottom + halfH, Playfield_top - halfH);
  }
};

struct Ball {
  float x= 0.0f;
  float y= 0.0f;
  float vx= 0.0f;
  float vy= 0.0f;
  float speed= Ball_initial_speed;

  // reset ball position and give it a push
  void Reset(int serveDirection) {
    x= 0.0f;
    y= 0.0f;
    speed= Ball_initial_speed;

    // random serve angle between -35 and 35 degrees
    float angle= ((float)(rand() % 70) - 35.0f) * (3.14159265f / 180.0f);
    vx= serveDirection * speed * cos(angle);
    vy= speed * sin(angle);
  }
};

struct GameState {
  bool isPaused= false;
  bool matchOver= false;
  int winner= 0;             // 1 for player 1, 2 for player 2
  float serveTimer= 1.0f;    // slight delay before serving
  int nextServeDirection= 1; // 1 = toward p2, -1 = toward p1
};

// reset scores, paddle positions, and start a fresh round
void ResetRound(Paddle &p1, Paddle &p2, Ball &ball, GameState &state) {
  p1.score= 0;
  p2.score= 0;
  p1.y= 0.0f;
  p2.y= 0.0f;
  state.matchOver= false;
  state.winner= 0;
  state.isPaused= false;
  ball.Reset(state.nextServeDirection);
  state.serveTimer= 0.8f;
}

// handle ball collision and angle bounce off the paddle
void CheckPaddleCollision(Ball &ball, const Paddle &paddle, int direction) {
  float halfH= Paddle_height / 2.0f;
  float halfB= Ball_size / 2.0f;

  // only collide if the ball is actually moving towards the paddle
  if ((direction < 0 && ball.vx >= 0.0f) || (direction > 0 && ball.vx <= 0.0f)) {
    return;
  }

  float paddleLeft= paddle.x;
  float paddleRight= paddle.x + Paddle_width;
  float paddleTop= paddle.y + halfH;
  float paddleBot= paddle.y - halfH;

  // check if the bounding boxes overlap
  bool hitX= (ball.x + halfB >= paddleLeft) && (ball.x - halfB <= paddleRight);
  bool hitY= (ball.y + halfB >= paddleBot) && (ball.y - halfB <= paddleTop);

  if (hitX && hitY) {
    // push the ball outside the paddle so it doesn't get stuck
    if (direction < 0) {
      ball.x= paddleRight + halfB;
    } 
    else {
      ball.x= paddleLeft - halfB;
    }

    // speed up the ball slightly on every hit
    ball.speed= min(ball.speed * Ball_speed_increment, Ball_max_speed);

    // calculate bounce angle based on where the ball hit the paddle
    float relativeHit= clamp((ball.y - paddle.y) / halfH, -1.0f, 1.0f);
    float bounceAngle= relativeHit * (55.0f * 3.14159265f / 180.0f);

    ball.vx= -direction * ball.speed * cos(bounceAngle);
    ball.vy= ball.speed * sin(bounceAngle);
  }
}

// handle when someone scores and check for a winner
void HandleGoalScored(int scoringPlayer, Paddle &scorer, Ball &ball, GameState &state) {
  scorer.score++;

  if (scorer.score >= Winning_score) {
    scorer.wins++;
    state.matchOver= true;
    state.winner= scoringPlayer;
  } 
  else {
    // next serve aims toward whoever just conceded
    if (scoringPlayer == 1) {
      state.nextServeDirection= -1;
    } 
    else {
      state.nextServeDirection= 1;
    }
    ball.Reset(state.nextServeDirection);
    state.serveTimer= 0.8f;
  }
}

// main entry point
int main() {

  // initialize glfw and opengl
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window= glfwCreateWindow(Window_width, Window_height, "Ping Pong 2D", NULL, NULL);
  if (window == NULL) {
    cout << "Failed to create GLFW window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  gladLoadGL();
  glViewport(0, 0, Window_width, Window_height);

  // load our shaders and background image
  Shader bgShader("assets/shaders/default.vert", "assets/shaders/default.frag");
  Shader pongShader("assets/shaders/pong.vert", "assets/shaders/pong.frag");

  // set up fullscreen background quad
  VAO bgVAO;
  bgVAO.Bind();
  VBO bgVBO(bgVertices, sizeof(bgVertices));
  EBO bgEBO(quadIndices, sizeof(quadIndices));
  bgVAO.LinkAttrib(bgVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void *)0);
  bgVAO.LinkAttrib(bgVBO, 1, 2, GL_FLOAT, 4 * sizeof(float),(void *)(2 * sizeof(float)));
  bgVAO.Unbind();
  bgVBO.Unbind();
  bgEBO.Unbind();

  Texture bgTexture("background.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
  bgTexture.texUnit(bgShader, "tex0", 0);

  // set up unit quad for game elements
  VAO quadVAO;
  quadVAO.Bind();
  VBO quadVBO(quadVertices, sizeof(quadVertices));
  EBO quadEBO(quadIndices, sizeof(quadIndices));
  quadVAO.LinkAttrib(quadVBO, 0, 2, GL_FLOAT, 2 * sizeof(float), (void *)0);
  quadVAO.Unbind();
  quadVBO.Unbind();
  quadEBO.Unbind();

  // cache shader uniform locations
  pongShader.Activate();
  offsetLoc= glGetUniformLocation(pongShader.ID, "offset");
  scaleLoc= glGetUniformLocation(pongShader.ID, "scale");
  colorLoc= glGetUniformLocation(pongShader.ID, "color");

  // initialize game state and entities
  srand((unsigned int)time(NULL));

  Paddle p1{-0.92f};
  Paddle p2{0.92f - Paddle_width};
  Ball ball;
  GameState state;

  ResetRound(p1, p2, ball, state);

  bool pKeyWasPressed= false;
  float lastFrameTime= (float)glfwGetTime();

  // main game loop
  while (!glfwWindowShouldClose(window)) {
    // calculate capped delta time
    float currentFrameTime= (float)glfwGetTime();
    float deltaTime= min(currentFrameTime - lastFrameTime, 0.05f);
    lastFrameTime= currentFrameTime;

    // 1. input handling
    bool pKeyPressed= (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
    if (pKeyPressed && !pKeyWasPressed) {
      state.isPaused= !state.isPaused;
    }
    pKeyWasPressed= pKeyPressed;

    if (!state.matchOver && !state.isPaused) {
      // player 1 movement controls
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        p1.Move(Paddle_speed * deltaTime);
      }
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        p1.Move(-Paddle_speed * deltaTime);
      }

      // player 2 movement controls
      if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        p2.Move(Paddle_speed * deltaTime);
      }
      if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        p2.Move(-Paddle_speed * deltaTime);
      }
    } 
    else if (state.matchOver) {
      // restart match on space or enter
      if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        ResetRound(p1, p2, ball, state);
      }
    }

    // full game reset on r key
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
      p1.wins= 0;
      p2.wins= 0;
      ResetRound(p1, p2, ball, state);
    }

    // 2. physics & ball simulation
    if (!state.matchOver && !state.isPaused) {
      if (state.serveTimer > 0.0f) {
        state.serveTimer -= deltaTime;
      } 
      else {
        // move ball
        ball.x += ball.vx * deltaTime;
        ball.y += ball.vy * deltaTime;

        float halfB= Ball_size / 2.0f;

        // bounce off top and bottom boundaries
        if (ball.y + halfB >= Playfield_top) {
          ball.y= Playfield_top - halfB;
          ball.vy= -abs(ball.vy);
        } 
        else if (ball.y - halfB <= Playfield_bottom) {
          ball.y= Playfield_bottom + halfB;
          ball.vy= abs(ball.vy);
        }

        // check paddle collisions
        CheckPaddleCollision(ball, p1, -1);
        CheckPaddleCollision(ball, p2, 1);

        // check scoring conditions
        if (ball.x < -1.08f) {
          HandleGoalScored(2, p2, ball, state);
        } 
        else if (ball.x > 1.08f) {
          HandleGoalScored(1, p1, ball, state);
        }
      }
    }

    // update window title status
    string title= "Ping Pong 2D | P1: " + to_string(p1.score) + " (Wins: " + to_string(p1.wins) + ")" + "  -  P2: " + to_string(p2.score) + " (Wins: " + to_string(p2.wins) + ")";
    if (state.isPaused) {
      title += " [PAUSED - PRESS P TO RESUME]";
    }
    else if (state.matchOver) {
      title += " [PRESS SPACE]";
    }
    glfwSetWindowTitle(window, title.c_str());

    // 3. clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 4. draw background
    bgShader.Activate();
    bgTexture.Bind();
    bgVAO.Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // 5. draw game elements
    pongShader.Activate();
    quadVAO.Bind();
    SetColor(1.0f, 1.0f, 1.0f); // set white color for borders and paddles

    // draw field borders
    DrawRect(-1.0f, Playfield_top, 2.0f, Border_height);
    DrawRect(-1.0f, Playfield_bottom - Border_height, 2.0f, Border_height);

    // draw center dashed line
    for (float y= Playfield_bottom + 0.01f; y < Playfield_top - 0.02f; y += (0.045f + 0.035f)) {
      DrawRect(-0.012f / 2.0f, y, 0.012f, 0.045f);
    }

    // draw paddles
    float halfPaddleH= Paddle_height / 2.0f;
    DrawRect(p1.x, p1.y - halfPaddleH, Paddle_width, Paddle_height);
    DrawRect(p2.x, p2.y - halfPaddleH, Paddle_width, Paddle_height);

    // draw ball
    DrawRect(ball.x - Ball_size / 2.0f, ball.y - Ball_size / 2.0f, Ball_size, Ball_size);

    // draw round scores
    DrawString(to_string(p1.score), -0.25f - GetStringWidth(to_string(p1.score), 0.022f), 0.62f,0.022f, 0.026f);
    DrawString(to_string(p2.score), 0.25f, 0.62f, 0.022f, 0.026f);

    // draw win counters and tally boxes
    DrawString("WINS: " + to_string(p1.wins), -0.85f, 0.82f, 0.010f, 0.012f);
    int p1BoxCount= min(p1.wins, 10);
    for (int i= 0; i < p1BoxCount; i++) {
      DrawRect(-0.85f + i * 0.026f, 0.77f, 0.018f, 0.018f);
    }

    DrawString("WINS: " + to_string(p2.wins), 0.85f - GetStringWidth("WINS: " + to_string(p2.wins), 0.010f), 0.82f, 0.010f, 0.012f);
    int p2BoxCount= min(p2.wins, 10);
    for (int i= 0; i < p2BoxCount; i++) {
      DrawRect((0.85f - 0.018f) - i * 0.026f, 0.77f, 0.018f, 0.018f);
    }

    // draw keybind hints
    SetColor(0.65f, 0.65f, 0.65f);
    DrawString("P1: W / S  |  P: PAUSE", -0.85f, -0.88f, 0.007f, 0.009f);

    DrawString("P2: UP / DN", 0.85f - GetStringWidth("P2: UP / DN", 0.007f), -0.88f, 0.007f, 0.009f);

    // draw match winner or pause overlay
    if (state.matchOver) {
      SetColor(1.0f, 1.0f, 0.2f); // added a golden color for winner message
      if (state.winner == 1) {
        DrawString("PLAYER 1 WINS!",-GetStringWidth("PLAYER 1 WINS!", 0.018f) / 2.0f, 0.12f,0.018f, 0.022f);
      } 
      else {
        DrawString("PLAYER 2 WINS!",-GetStringWidth("PLAYER 2 WINS!", 0.018f) / 2.0f, 0.12f,0.018f, 0.022f);
      }

      SetColor(1.0f, 1.0f, 1.0f);
      DrawString("PRESS SPACE TO PLAY NEXT MATCH",-GetStringWidth("PRESS SPACE TO PLAY NEXT MATCH", 0.009f) / 2.0f,-0.05f, 0.009f, 0.011f);

      DrawString("PRESS R TO RESET ALL WINS",-GetStringWidth("PRESS R TO RESET ALL WINS", 0.009f) / 2.0f,-0.15f, 0.009f, 0.011f);
    } 
    else if (state.isPaused) {
      SetColor(0.2f, 0.9f, 1.0f); // cyan color for pause message
      DrawString("PAUSED", -GetStringWidth("PAUSED", 0.022f) / 2.0f, 0.08f,0.022f, 0.028f);

      SetColor(1.0f, 1.0f, 1.0f);
      DrawString("PRESS P TO RESUME", -GetStringWidth("PRESS P TO RESUME", 0.009f) / 2.0f, -0.04f,0.009f, 0.011f);
    }

    // swap buffers and poll window events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // clean up opengl resources on exit
  bgVAO.Delete();
  bgVBO.Delete();
  bgEBO.Delete();
  bgTexture.Delete();
  bgShader.Delete();
  quadVAO.Delete();
  quadVBO.Delete();
  quadEBO.Delete();
  pongShader.Delete();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
