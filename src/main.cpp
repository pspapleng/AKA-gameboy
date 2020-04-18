#include <Arduino.h>

// Include application, user and local libraries
#include "SPI.h"
#include "TFT_22_ILI9225.h"

#define TFT_RST A4
#define TFT_RS A3
#define TFT_CS A5  // SS
#define TFT_SDI A2 // MOSI
#define TFT_CLK A1 // SCK
#define TFT_LED 0  // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 200

#define STABLE "STABLE"
#define UP "UP"
#define DOWN "DOWN"
#define LEFT "LEFT"
#define RIGHT "RIGHT"
#define PRESS_A "PRESS A"
#define PRESS_B "PRESS B"
#define PRESS_C "PRESS C"
#define PRESS_D "PRESS D"

#define PIN_ANALOG_X 0
#define PIN_ANALOG_Y 1
#define PIN_A 2
#define PIN_B 3
#define PIN_C 4
#define PIN_D 5
#define PIN_E 6
#define PIN_F 7
#define PIN_K 8
#define printNewLine(a) (Serial.println(a))
#define printInLine(a) (Serial.print(F(a)))

void initJoyStick();
String status2Code(int statusCode);
int joyStickDecode();
String mode2Str(int mode);

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
  score
};
struct coordination
{
  int x;
  int y;
};

struct gameArea
{
  struct coordination startWidth;
  struct coordination endWidth;
  struct coordination startHeight;
  struct coordination endHeight;
};

struct boxPosition
{
  struct coordination topLeft;
  struct coordination bottomRight;
};

struct snake
{
  struct coordination center;
  struct coordination tailCenter;
  struct boxPosition head;
  struct boxPosition tail;

  int radius;
  uint16_t color;
  int moveDirection;
  int lastMoveDirection;
  int OneMoveDistance;
};

struct border
{
  uint16_t color;
  struct coordination topLeft;
  struct coordination bottomRight;
};

struct gameController
{
  int mode;
  uint16_t backgroundColor;
  boolean isDrawnBorder;
  struct border mapBorder;
};

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

struct gameArea area = {};
struct gameController game = {};
struct snake player = {};

// Setup
void setup()
{

  Serial.begin(9600);
  printNewLine();
  printNewLine(" _____       _     _           ");
  printNewLine("|  _  |___ _| |_ _| |_ ___ _ _ ");
  printNewLine("|     |  _| . | | | . | . | | |");
  printNewLine("|__|__|_| |___|___|___|___|_  |");
  printNewLine("                          |___|");

  printNewLine("======== OS BOOT ========");
  printNewLine("[  ok  ] Serial Initialize.");

  tft.begin();
  printNewLine("[  ok  ] TFT Initialize.");

  initJoyStick();
  printNewLine("[  ok  ] JoyStick Pin Initialize.");

  //init game parameter
  game.mode = playGame;
  game.mapBorder.topLeft.x = 0;
  game.mapBorder.topLeft.x = 0;
  game.mapBorder.bottomRight.x = 174;
  game.mapBorder.bottomRight.y = 218;
  game.mapBorder.color = COLOR_WHITE;
  game.backgroundColor = COLOR_BLACK;
  game.isDrawnBorder = false;
  printNewLine("[  ok  ] Game Engine Initialize. ");

  player.center.x = game.mapBorder.bottomRight.x / 2;
  player.center.y = game.mapBorder.bottomRight.y / 2;
  player.tailCenter.x = game.mapBorder.bottomRight.x / 2;
  player.tailCenter.y = game.mapBorder.bottomRight.y / 2;

  player.radius = 2;

  player.head.topLeft.x = player.center.x - player.radius;
  player.head.topLeft.y = player.center.y - player.radius;
  player.head.bottomRight.x = player.center.x + player.radius;
  player.head.bottomRight.y = player.center.y + player.radius;

  player.tail.topLeft.x = player.tailCenter.x - player.radius;
  player.tail.topLeft.y = player.tailCenter.y - player.radius;
  player.tail.bottomRight.x = player.tailCenter.x + player.radius;
  player.tail.bottomRight.y = player.tailCenter.y + player.radius;

  player.color = COLOR_WHITE;

  player.moveDirection = up;
  player.lastMoveDirection = up;
  player.OneMoveDistance = 2 * player.radius - 1;

  printNewLine("[  ok  ] Player Parameter Initialize.");

  tft.setBackgroundColor(game.backgroundColor);
  printNewLine("[  ok  ] Set Back Ground To " + String(game.backgroundColor));
}

// Loop
void loop()
{
  switch (game.mode)
  {
  case startScreen:
    break;
  case playGame:
  {

    if (!game.isDrawnBorder)
    {
      //drawn game border
      tft.drawRectangle(game.mapBorder.topLeft.x, game.mapBorder.topLeft.y, game.mapBorder.bottomRight.x, game.mapBorder.bottomRight.y, game.mapBorder.color);
      game.isDrawnBorder = !game.isDrawnBorder;
    }

    //Delete Tail Postion
    tft.fillRectangle(player.tail.topLeft.x, player.tail.topLeft.y, player.tail.bottomRight.x, player.tail.bottomRight.y, game.backgroundColor);

    //Add Head Postion
    tft.fillRectangle(player.head.topLeft.x, player.head.topLeft.y, player.head.bottomRight.x, player.head.bottomRight.y, player.color);


    player.moveDirection = joyStickDecode();
    if (player.moveDirection == stable)
    {
      player.moveDirection = player.lastMoveDirection;
    }
    else
    {
      player.lastMoveDirection = player.moveDirection;
    }

    // Update Tail Postion
    player.tailCenter.x = player.center.x;
    player.tailCenter.y = player.center.y;

    //Update Snake Direction
    switch (player.moveDirection)
    {
    case up:
      player.center.y -= player.OneMoveDistance;
      break;
    case down:
      player.center.y += player.OneMoveDistance;
      break;
    case left:
      player.center.x -= player.OneMoveDistance;
      break;
    case right:
      player.center.x += player.OneMoveDistance;
      break;
    default:
      break;
    }

    //Update Tail Position
    player.tail.topLeft.x = player.tailCenter.x - player.radius;
    player.tail.topLeft.y = player.tailCenter.y - player.radius;
    player.tail.bottomRight.x = player.tailCenter.x + player.radius;
    player.tail.bottomRight.y = player.tailCenter.y + player.radius;

    //Update Head Position;
    player.head.topLeft.x = player.center.x - player.radius;
    player.head.topLeft.y = player.center.y - player.radius;
    player.head.bottomRight.x = player.center.x + player.radius;
    player.head.bottomRight.y = player.center.y + player.radius;


    //GameOver
    boolean hitLeft = player.head.topLeft.x < game.mapBorder.topLeft.x;
    boolean hitTop = player.head.topLeft.y < game.mapBorder.topLeft.y;
    boolean hitRight = player.head.bottomRight.x > game.mapBorder.bottomRight.x;
    boolean hitBottom = player.head.bottomRight.y > game.mapBorder.bottomRight.y;
    if (hitLeft || hitRight || hitTop || hitBottom)
    {
      game.mode = gameOver;
    }

    //Debug Zone
    printNewLine("[ " + mode2Str(game.mode) + " ] Move Direction : " + String(status2Code(player.moveDirection)) + " | Player Center   X : " + String(player.center.x) + " Y : " + String(player.center.y));
    break;
  }
  case gameOver:
  {
    printNewLine("[ " + mode2Str(game.mode) + " ] ");
    tft.clear();
    tft.setBackgroundColor(COLOR_WHITE);
    break;
  }
  case score:
    break;
  default:
    printNewLine("[  default ] ");
    break;
  }
}





String mode2Str(int mode)
{
  String str = "";
  switch (mode)
  {
  case startScreen:
    str = "Start Screen.";
    break;
  case playGame:
    str = "Play Game.";
    break;
  case gameOver:
    str = "Game Over.";
    break;
  case score:
    str = "Score Board";
    break;
  default:
    str = "Out Of Case";
    break;
  }
  return str;
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

String status2Code(int statusCode)
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
  int pinE = digitalRead(PIN_E);
  int pinF = digitalRead(PIN_F);
  int pinK = digitalRead(PIN_K);

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