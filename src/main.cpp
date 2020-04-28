#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>

// Prototype Funcitons
void initJoyStick();
int joyStickDecode();
String gameMode2Str(int mode);
String joyStickAction2Str(int statusCode);
// Arduino Configurations
#define DEFAULT_BAUD_RATE 9600
#define PRINT_NEW_LINE(a) (Serial.println(a))
#define PRINT_IN_LINE(a) (Serial.print(F(a)))

// TFT Configurations
#define TFT_RST A4
#define TFT_RS A3
#define TFT_CS A5  // SS
#define TFT_SDI A2 // MOSI
#define TFT_CLK A1 // SCK
#define TFT_LED 0  // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 200

// Joy Stick Configurations
#define PIN_ANALOG_X 0
#define PIN_ANALOG_Y 1
#define PIN_A 2
#define PIN_B 3
#define PIN_C 4
#define PIN_D 5
#define PIN_E 6
#define PIN_F 7
#define PIN_K 8

// Joy Stick Actions To String
#define STABLE "STABLE"
#define UP "UP"
#define DOWN "DOWN"
#define LEFT "LEFT"
#define RIGHT "RIGHT"
#define PRESS_A "PRESS A"
#define PRESS_B "PRESS B"
#define PRESS_C "PRESS C"
#define PRESS_D "PRESS D"

#define START_SCREEN "Start Screen ."
#define PLAY_GAME "Play  Game."
#define GAME_OVER "Game Over."

//Joy Stick Actions
enum joyStick
{
  stable,
  up,
  down,
  left,
  right,
  pressA,
  pressB,
  pressC,
  pressD
};

enum gameMode
{
  startScreen,
  playGame,
  gameOver,
};

struct coordination
{
  uint8_t X;
  uint8_t Y;
};

struct dimension
{
  uint8_t width;
  uint8_t height;
};

struct rectangle
{
  struct coordination topLeft;
  struct coordination bottomRight;
  struct coordination center;
  struct dimension rectangleDimension;
  uint16_t color;
  uint8_t radius;
};

struct circle
{
  struct coordination center;
  uint8_t radius;
  uint16_t color;
};

struct player
{
  struct rectangle head;
  struct rectangle tail;
  boolean isHitBorder;
  boolean isHitItem;
  boolean isHitBarricade;
  uint32_t score;
  uint8_t direction;
  uint8_t lastDirection;
};
struct item
{
  struct circle body;
  boolean isSpawn;
  boolean isFirstDraw;
  uint8_t score;
};

struct engine
{
  uint8_t mode;
  uint8_t gridSize;
  uint16_t backgroundColor;
  boolean isDrawBorder;
  struct rectangle border;
  struct dimension gridNo;
};

struct Barricade
{
  struct rectangle body;
  boolean isDraw;
};

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

struct engine gameEngine;
struct player snake;
struct item food;
struct Barricade straightWall;

void setup()
{
  Serial.begin(DEFAULT_BAUD_RATE);
  Serial.begin(9600);
  PRINT_NEW_LINE();
  PRINT_NEW_LINE();
  PRINT_NEW_LINE();

  PRINT_NEW_LINE(" _____       _     _           ");
  PRINT_NEW_LINE("|  _  |___ _| |_ _| |_ ___ _ _ ");
  PRINT_NEW_LINE("|     |  _| . | | | . | . | | |");
  PRINT_NEW_LINE("|__|__|_| |___|___|___|___|_  |");
  PRINT_NEW_LINE("                          |___|");

  PRINT_NEW_LINE("======== OS BOOT ========");
  PRINT_NEW_LINE("[  ok  ] Serial Initialize.");

  tft.begin();
  PRINT_NEW_LINE("[  ok  ] TFT Initialize.");

  initJoyStick();
  PRINT_NEW_LINE("[  ok  ] JoyStick Pin Initialize.");

  //Init Player Parameter

  // Init Game Engine Parameter
  gameEngine.mode = playGame;
  gameEngine.border.topLeft.X = 0;
  gameEngine.border.topLeft.Y = 0;
  gameEngine.border.bottomRight.X = 174;
  gameEngine.border.bottomRight.Y = 218;
  gameEngine.border.rectangleDimension.width = gameEngine.border.bottomRight.X - gameEngine.border.topLeft.X;
  gameEngine.border.rectangleDimension.height = gameEngine.border.bottomRight.Y - gameEngine.border.topLeft.Y;
  gameEngine.border.color = COLOR_WHITE;
  gameEngine.isDrawBorder = false;
  gameEngine.backgroundColor = COLOR_BLACK;
  gameEngine.gridSize = 5;
  gameEngine.gridNo.width = gameEngine.border.rectangleDimension.width / gameEngine.gridSize;
  gameEngine.gridNo.height = gameEngine.border.rectangleDimension.height / gameEngine.gridSize;

  snake.head.radius = gameEngine.gridSize / 2;
  snake.head.center.X = ((gameEngine.gridNo.width / 2) * gameEngine.gridSize) - snake.head.radius;
  snake.head.center.Y = ((gameEngine.gridNo.height / 2) * gameEngine.gridSize) - snake.head.radius;
  snake.head.topLeft.X = snake.head.center.X - snake.head.radius;
  snake.head.topLeft.Y = snake.head.center.Y - snake.head.radius;
  snake.head.bottomRight.X = snake.head.center.X + snake.head.radius;
  snake.head.bottomRight.Y = snake.head.center.Y + snake.head.radius;
  snake.head.color = COLOR_WHITE;
  snake.tail.radius = gameEngine.gridSize / 2;
  snake.tail.center.X = ((gameEngine.gridNo.width / 2) * gameEngine.gridSize) - snake.tail.radius;
  snake.tail.center.Y = ((gameEngine.gridNo.height / 2) * gameEngine.gridSize) - snake.tail.radius;
  snake.tail.topLeft.X = snake.tail.center.X - snake.tail.radius;
  snake.tail.topLeft.Y = snake.tail.center.Y - snake.tail.radius;
  snake.tail.bottomRight.X = snake.tail.center.X + snake.tail.radius;
  snake.tail.bottomRight.Y = snake.tail.center.Y + snake.tail.radius;
  snake.tail.color = gameEngine.backgroundColor;
  snake.isHitBorder = false;
  snake.isHitItem = false;
  snake.score = 0;
  snake.direction = up;
  snake.lastDirection = up;

  food.body.center.X = -1;
  food.body.center.Y = -1;
  food.body.color = COLOR_RED;
  food.body.radius = 1;
  food.isSpawn = false;
  food.score = 0;
  food.isFirstDraw = false;

  straightWall.body.center.X = -1;
  straightWall.body.center.Y = -1;
  straightWall.body.radius = gameEngine.gridSize / 2;
  straightWall.body.topLeft.X = (straightWall.body.center.X - straightWall.body.radius);
  straightWall.body.topLeft.Y = (straightWall.body.center.Y - (straightWall.body.radius + (2 * gameEngine.gridSize)));
  straightWall.body.bottomRight.X = (straightWall.body.center.X + straightWall.body.radius);
  straightWall.body.bottomRight.Y = (straightWall.body.center.Y + (straightWall.body.radius + (2 * gameEngine.gridSize)));
  straightWall.body.rectangleDimension.width = straightWall.body.bottomRight.X - straightWall.body.topLeft.X;
  straightWall.body.rectangleDimension.height = straightWall.body.bottomRight.Y - straightWall.body.topLeft.Y;
  straightWall.body.color = COLOR_GREEN;
  straightWall.isDraw = false;

  randomSeed(millis());
  tft.setBackgroundColor(gameEngine.backgroundColor);
}

void loop()
{
  switch (gameEngine.mode)
  {
  case startScreen:
  {
    break;
  }
  case playGame:
  {

    // Drawn Map Border
    if (!gameEngine.isDrawBorder)
    {
      tft.drawRectangle(gameEngine.border.topLeft.X,
                        gameEngine.border.topLeft.Y,
                        gameEngine.border.bottomRight.X,
                        gameEngine.border.bottomRight.Y,
                        gameEngine.border.color);

      gameEngine.isDrawBorder = !gameEngine.isDrawBorder;
    }

    // Spawn Food
    if (!food.isSpawn)
    {
      // Random Grid Positon
      if (!food.isFirstDraw)
      {

        randomSeed(millis());
        PRINT_NEW_LINE("RANDOM X Start : " + String(gameEngine.border.topLeft.X + 2) + "  End : " + String(gameEngine.gridNo.width - 2));
        PRINT_NEW_LINE("RANDOM X Start : " + String(gameEngine.border.topLeft.Y + 2) + "  End : " + String(gameEngine.gridNo.height - 2));

        food.body.center.X = int(random(gameEngine.border.topLeft.X + 2, gameEngine.gridNo.width - 2));
        food.body.center.Y = int(random(gameEngine.border.topLeft.Y + 2, gameEngine.gridNo.height - 2));

        // Set Food Center To Center Of Grid

        food.body.center.X = (food.body.center.X * gameEngine.gridSize) - (gameEngine.gridSize / 2);
        food.body.center.Y = (food.body.center.Y * gameEngine.gridSize) - (gameEngine.gridSize / 2);

        food.isFirstDraw = !food.isFirstDraw;
      }

      // Drawn Food
      tft.fillCircle(food.body.center.X, food.body.center.Y, food.body.radius, food.body.color);

      // Change Status Of Food Spawn
      food.isSpawn = !food.isSpawn;

      PRINT_NEW_LINE("Food Center   X : " + String(food.body.center.X) + "  Y : " + food.body.center.Y);
    }

    if (!straightWall.isDraw)
    {
      randomSeed(millis());

      straightWall.body.center.X = int(random(gameEngine.border.topLeft.X + 2, gameEngine.gridNo.width - 2));
      straightWall.body.center.Y = int(random(gameEngine.border.topLeft.Y + 2, gameEngine.gridNo.height - 2));

      straightWall.body.center.X = (straightWall.body.center.X * gameEngine.gridSize) - (straightWall.body.radius);
      straightWall.body.center.Y = (straightWall.body.center.Y * gameEngine.gridSize) - (straightWall.body.radius);

      straightWall.body.topLeft.X = (straightWall.body.center.X - straightWall.body.radius);
      straightWall.body.topLeft.Y = (straightWall.body.center.Y - (straightWall.body.radius + (2 * gameEngine.gridSize)));
      straightWall.body.bottomRight.X = (straightWall.body.center.X + straightWall.body.radius);
      straightWall.body.bottomRight.Y = (straightWall.body.center.Y + (straightWall.body.radius + (2 * gameEngine.gridSize)));

      PRINT_NEW_LINE("WALL Width : " + String(straightWall.body.rectangleDimension.width) +
                     "  Height : " + String(straightWall.body.rectangleDimension.height));

      tft.fillRectangle(straightWall.body.topLeft.X,
                        straightWall.body.topLeft.Y,
                        straightWall.body.bottomRight.X,
                        straightWall.body.bottomRight.Y,
                        straightWall.body.color);

      straightWall.isDraw = !straightWall.isDraw;
    }

    // Delete Tail
    tft.fillRectangle(snake.tail.topLeft.X,
                      snake.tail.topLeft.Y,
                      snake.tail.bottomRight.X,
                      snake.tail.bottomRight.Y,
                      snake.tail.color);

    // Delete Head
    tft.fillRectangle(snake.head.topLeft.X,
                      snake.head.topLeft.Y,
                      snake.head.bottomRight.X,
                      snake.head.bottomRight.Y,
                      snake.head.color);

    // Check Hit Border
    boolean hitLeftBorder = snake.head.topLeft.X <= gameEngine.border.topLeft.X;
    boolean hitTopBorder = snake.head.topLeft.Y <= gameEngine.border.topLeft.Y;
    boolean hitRightBorder = snake.head.bottomRight.X >= gameEngine.border.bottomRight.X;
    boolean hitBottomBorder = snake.head.bottomRight.Y >= gameEngine.border.bottomRight.Y;
    snake.isHitBorder = hitLeftBorder || hitTopBorder || hitRightBorder || hitBottomBorder;

    // Check Hit Item
    int tempX = food.body.center.X;
    int tempY = food.body.center.Y;

    // which edge is closest?
    if (food.body.center.X < snake.head.center.X)
      tempX = snake.head.center.X; // test left edge
    else if (food.body.center.X > snake.head.center.X + snake.head.rectangleDimension.width)
      tempX = snake.head.center.X + snake.head.rectangleDimension.width; // right edge
    if (food.body.center.Y < snake.head.center.Y)
      tempY = snake.head.center.Y; // top edge
    else if (food.body.center.Y > snake.head.center.Y + snake.head.rectangleDimension.height)
      tempY = snake.head.center.Y + snake.head.rectangleDimension.height; // bottom edge

    // get distance from closest edges
    float distX = food.body.center.X - tempX;
    float distY = food.body.center.Y - tempY;
    float distance = sqrt((distX * distX) + (distY * distY));

    // if the distance is less than the radius, collision!
    if (distance <= food.body.radius)
    {
      snake.isHitItem = true;
    }

    // Check Hit Walls

    boolean collisionLeft = snake.head.topLeft.X + snake.head.rectangleDimension.width >= straightWall.body.topLeft.X;
    boolean collisionRight = snake.head.topLeft.X <= straightWall.body.topLeft.X + straightWall.body.rectangleDimension.width;
    boolean collisionBottom = snake.head.topLeft.Y + snake.head.rectangleDimension.height >= straightWall.body.topLeft.Y;
    boolean collisionTop = snake.head.topLeft.Y <= straightWall.body.topLeft.Y + straightWall.body.rectangleDimension.height;

    snake.isHitBarricade = collisionLeft && collisionRight && collisionTop && collisionBottom;
    PRINT_NEW_LINE("Hit Walls Left : " + String(collisionLeft) +
                   "  Right : " + String(collisionRight) +
                   "  Top : " + String(collisionTop) +
                   "  Bottom : " + String(collisionBottom));
    if (snake.isHitBarricade || snake.isHitBorder)
    {
      gameEngine.mode = gameOver;
    }

    if (snake.isHitItem)
    {
      snake.score += food.score;

      food.body.center.X = straightWall.body.center.X;
      food.body.center.Y = straightWall.body.center.Y;

      tft.fillRectangle(straightWall.body.topLeft.X,
                        straightWall.body.topLeft.Y,
                        straightWall.body.bottomRight.X,
                        straightWall.body.bottomRight.Y,
                        gameEngine.backgroundColor);

      // Update Tail Position
      switch (snake.direction)
      {
      case up:
      {
        snake.tail.center.Y += gameEngine.gridSize;
        break;
      }
      case down:
      {
        snake.tail.center.Y -= gameEngine.gridSize;
        break;
      }
      case left:
      {
        snake.tail.center.X += gameEngine.gridSize;
        break;
      }
      
      default:
        break;
      }

      snake.isHitItem = !snake.isHitItem;
      straightWall.isDraw = !straightWall.isDraw;
      food.isSpawn = !food.isSpawn;
    }

    // Update Tail Center Position
    // snake.tail.center.X = snake.head.center.X;
    // snake.tail.center.Y = snake.head.center.Y;

    // Update Snake Direction
    snake.direction = joyStickDecode();
    if (snake.direction == stable)
    {
      snake.direction = snake.lastDirection;
    }
    else
    {
      snake.lastDirection = snake.direction;
    }

    // Update Snake Center Position
    switch (snake.direction)
    {
    case up:
      snake.head.center.Y -= gameEngine.gridSize;
      break;
    case down:
      snake.head.center.Y += gameEngine.gridSize;
      break;
    case left:
      snake.head.center.X -= gameEngine.gridSize;
      break;
    case right:
      snake.head.center.X += gameEngine.gridSize;
      break;
    default:
      break;
    }

    // Update Snake Head Body Position
    snake.head.topLeft.X = snake.head.center.X - snake.head.radius;
    snake.head.topLeft.Y = snake.head.center.Y - snake.head.radius;
    snake.head.bottomRight.X = snake.head.center.X + snake.head.radius;
    snake.head.bottomRight.Y = snake.head.center.Y + snake.head.radius;

    // Update Snake Tail Body Position
    snake.tail.topLeft.X = snake.tail.center.X - snake.tail.radius;
    snake.tail.topLeft.Y = snake.tail.center.Y - snake.tail.radius;
    snake.tail.bottomRight.X = snake.tail.center.X + snake.tail.radius;
    snake.tail.bottomRight.Y = snake.tail.center.Y + snake.tail.radius;
    /*
    PRINT_NEW_LINE("[ " + gameMode2Str(gameEngine.mode) + " ] Move Direction : " +
                   String(joyStickAction2Str(snake.direction)) +
                   " | Player Center   X : " +
                   String(snake.head.center.X) +
                   " Y : " + String(snake.head.center.Y) +
                   " | Item Center  X : " + String(food.body.center.X) +
                   " Y : " + String(food.body.center.Y) +
                   " | Hit Item : " + String(snake.isHitItem));
*/
    break;
  }
  case gameOver:
  {
    tft.clear();
    break;
  }
  default:
    break;
  }
  delay(100);
}

void initJoyStick()
{
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_C, INPUT);
  pinMode(PIN_D, INPUT);
  pinMode(PIN_E, INPUT);
  pinMode(PIN_F, INPUT);
  pinMode(PIN_K, INPUT);

  digitalWrite(PIN_A, HIGH);
  digitalWrite(PIN_B, HIGH);
  digitalWrite(PIN_C, HIGH);
  digitalWrite(PIN_D, HIGH);
  digitalWrite(PIN_E, HIGH);
  digitalWrite(PIN_F, HIGH);
  digitalWrite(PIN_K, HIGH);
}

String joyStickAction2Str(int statusCode)
{
  String text = "";
  switch (statusCode)
  {
  case 0:
    text = STABLE;
    break;
  case 1:
    text = UP;
    break;
  case 2:
    text = DOWN;
    break;
  case 3:
    text = LEFT;
    break;
  case 4:
    text = RIGHT;
    break;
  case 5:
    text = PRESS_A;
    break;
  case 6:
    text = PRESS_B;
    break;
  case 7:
    text = PRESS_C;
    break;
  case 8:
    text = PRESS_D;
    break;
  default:
    text = "Other";
    break;
  }
  return text;
}

int joyStickDecode()
{

  int analogX = analogRead(PIN_ANALOG_X);
  int analogY = analogRead(PIN_ANALOG_Y);
  int pinA = digitalRead(PIN_A);
  int pinB = digitalRead(PIN_B);
  int pinC = digitalRead(PIN_C);
  int pinD = digitalRead(PIN_D);
  int currentStatus;

  if (analogX > 350 && analogY < 360 && analogY == 1)
  {
    currentStatus = stable;
  }
  if (analogX > 340 && analogX < 400 && analogY == 0)
  {
    currentStatus = up;
  }
  if (analogX > 350 && analogX < 400 && analogY > 10)
  {
    currentStatus = down;
  }
  if (analogX > 400 && analogX < 750 && analogY == 1)
  {
    currentStatus = left;
  }
  if (analogX < 330 && analogX > -1 && analogY == 1)
  {
    currentStatus = right;
  }
  if (pinA == 0)
  {
    currentStatus = pressA;
  }
  if (pinB == 0)
  {
    currentStatus = pressB;
  }
  if (pinC == 0)
  {
    currentStatus = pressC;
  }
  if (pinD == 0)
  {
    currentStatus = pressD;
  }

  return currentStatus;
}

String gameMode2Str(int mode)
{
  String str = "";
  switch (mode)
  {
  case startScreen:
    str = START_SCREEN;
    break;
  case playGame:
    str = PLAY_GAME;
    break;
  case gameOver:
    str = GAME_OVER;
    break;
  default:
    str = "Out Of Case";
    break;
  }
  return str;
}