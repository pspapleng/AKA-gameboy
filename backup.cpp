#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>
// Prototype Funcitons

void initJoyStick();
int joyStickDecode();
String gameMode2Str(int mode);
String joyStickAction2Str(int statusCode);
int readFromKeyboard();
void initGameEngine();
void initPlayer();
void initItem();
void initBarricade();
void drawBorder();
void initJoyStick();
void firstDrawItem();
void drawItem();
void drawBarricade();
void isHitBorder();
boolean isHitItem();
void isHitBarricade();
void isGameOver();
void reDrawBarricade();
void addTail();
void hitItemAction();
void deleteTail();
void updateDirectionReverse();
void updateSnakeDirection();
void updateSnakePosition();
void addHead();
// Arduino Configurations
#define CENTER_POSITION_CALCULATE(gridNO, gridSize, radius) (((gridNO / 2) * gridSize) - radius)
#define DEFAULT_BAUD_RATE 9600
#define DIMENSION_CALCULATE(start, end) (start - end)
#define GRID_NO_CALCULATE(total, size) (total / size)
#define PRINT_IN_LINE(a) (Serial.print(a))
#define PRINT_NEW_LINE(a) (Serial.println(a))
#define RADIUS_CALCULATE(radius) (radius / 2)
#define SQUARE_TOP_LEFT_CALCULATE(center, radius) (center - radius)
#define SQUARE_BOTTOM_RIGHT_CALCULATE(center, radius) (center + radius)
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

#define SCREEN_WIDTH 174
#define SCREEN_HEIGHT 218
#define START_POSITION -1
#define GRID_SIZE 5
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

enum startScreenStge
{
  ardunoboyLogo,
  snakeGameLogo,
  waitKeyPress
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

typedef struct player
{
  struct rectangle body;
  int direction;
  int nextDirection;
  struct player *next;
} player_t;

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
  boolean isHitBorder;
  boolean isHitItem;
  boolean isHitBarricade;
  uint32_t score;
  int lastDirection;
  struct rectangle border;
  struct dimension gridNo;
  int screenStage;
};

struct Barricade
{
  struct rectangle body;
  boolean isDraw;
};

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);
const long interval = 1000;
int blinkState = LOW;

int period = 500;

player_t *head = NULL;

struct Barricade straightWall;
struct engine gameEngine;
struct item food;

unsigned long previousMillis = 0;
unsigned long time_now = 0;

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
  initGameEngine();
  initPlayer();
  initItem();
  initBarricade();
  initJoyStick();
  randomSeed(millis());
  tft.setBackgroundColor(gameEngine.backgroundColor);
}

void loop()
{
  switch (gameEngine.mode)
  {
  case startScreen:
  {
    switch (gameEngine.screenStage)
    {
    case ardunoboyLogo:
    {
      tft.setFont(Terminal12x16);
      tft.drawText(145, 55, "B", COLOR_YELLOW);
      tft.drawText(144, 80, "O", COLOR_GREEN);
      tft.drawText(25, 80, "A R D U I N", COLOR_TURQUOISE);
      tft.drawText(145, 105, "Y", COLOR_RED);

      gameEngine.screenStage = snakeGameLogo;
      delay(3000);
      break;
    }
    case snakeGameLogo:
    {
      tft.clear();
      //snakegame
      tft.drawText(80, 50, "NAKE");
      tft.drawText(80, 80, "GAME");
      tft.fillRectangle(55, 50, 75, 55, COLOR_GREENYELLOW);
      tft.fillRectangle(55, 50, 60, 70, COLOR_GREENYELLOW);
      tft.fillRectangle(55, 70, 75, 75, COLOR_GREENYELLOW);
      tft.fillRectangle(70, 70, 75, 90, COLOR_GREENYELLOW);
      tft.fillRectangle(55, 87, 75, 93, COLOR_GREENYELLOW);
      tft.fillCircle(47, 91, 3, COLOR_RED);
      tft.setFont(Terminal12x16);
      tft.drawRectangle(50, 140, 125, 170, COLOR_LIGHTGRAY);
      tft.drawText(65, 147, "PLAY", COLOR_TURQUOISE);

      gameEngine.screenStage = waitKeyPress;

      break;
    }
    case waitKeyPress:
    {

      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval)
      {

        previousMillis = currentMillis;

        if (blinkState)
        {
          tft.setFont(Terminal6x8);
          tft.drawText(47, 175, "press any key", COLOR_CYAN);
          tft.drawText(62, 185, "to start", COLOR_CYAN);
          blinkState = !blinkState;
        }
        else
        {
          tft.drawText(47, 175, "press any key", gameEngine.backgroundColor);
          tft.drawText(62, 185, "to start", gameEngine.backgroundColor);
          blinkState = !blinkState;
        }
      }

      int input = joyStickDecode();
      Serial.println(input);
      if (input != stable)
      {
        Serial.println(input);
        tft.clear();

        gameEngine.mode = playGame;
      }
      break;
    }

    default:
      break;
    }

    break;
  }
  case playGame:
  {
    head->nextDirection = joyStickDecode();

    if (head->nextDirection == stable)
    {
      head->nextDirection = gameEngine.lastDirection;
    }
    else
    {
      gameEngine.lastDirection = head->nextDirection;
    }

    if (!gameEngine.isDrawBorder)
    {
      drawBorder();
    }

    if (!food.isSpawn)
    {
      if (!food.isFirstDraw)
        firstDrawItem();
      drawItem();
    }

    if (!straightWall.isDraw)
      drawBarricade();

    if (head->nextDirection != stable && head->nextDirection != 99)
    {
      // if (millis() >= time_now + period)
      // {
      //     time_now += period;
      // PRINT_NEW_LINE("Next Direction : " + String(head->nextDirection));
      unsigned long currentMillis = millis();
      if (currentMillis - time_now >= period)
      {

        time_now = currentMillis;

        deleteTail();
        addHead();

        isHitBorder();
        gameEngine.isHitItem = isHitItem();
        isHitBarricade();

        if (gameEngine.isHitItem)
        {
          PRINT_NEW_LINE("IS Hit Item");
          reDrawBarricade();
          addTail();
          hitItemAction();
        }

        isGameOver();
      }
    }

    break;
  }
  case gameOver:
  {

    initGameEngine();
    gameEngine.mode = gameOver;

    initPlayer();
    initItem();
    initBarricade();

    randomSeed(millis());
    tft.setFont(Terminal12x16);
    tft.drawText(40, 40, "GAMEOVER", COLOR_RED);
    tft.setFont(Terminal6x8);
    tft.drawText(70, 75, "SCORE", COLOR_TOMATO);
    tft.setFont(Terminal12x16);
    tft.drawText(76, 90, String(gameEngine.score), COLOR_WHITE);
    tft.setFont(Terminal12x16);
    tft.drawRectangle(50, 140, 130, 165, COLOR_LIGHTGRAY);
    tft.drawText(68, 146, "EXIT", COLOR_RED);
    tft.setFont(Terminal6x8);
    tft.drawText(47, 175, "press any key", COLOR_SIENNA);
    tft.drawText(65, 185, "to exit", COLOR_SIENNA);

    int input = joyStickDecode();
    Serial.println(input);
    if (input != stable)
    {
      Serial.println(input);
      tft.clear();
      gameEngine.mode = startScreen;
      gameEngine.screenStage = ardunoboyLogo;
    }

    break;
  }
  default:
    break;
  }
}

void initGameEngine()
{
  gameEngine.mode = startScreen;
  gameEngine.screenStage = ardunoboyLogo;
  gameEngine.border.topLeft.X = 0;
  gameEngine.border.topLeft.Y = 0;
  gameEngine.border.bottomRight.X = SCREEN_WIDTH;
  gameEngine.border.bottomRight.Y = SCREEN_HEIGHT;
  gameEngine.border.rectangleDimension.width = DIMENSION_CALCULATE(gameEngine.border.bottomRight.X, gameEngine.border.topLeft.X);
  gameEngine.border.rectangleDimension.height = DIMENSION_CALCULATE(gameEngine.border.bottomRight.Y, gameEngine.border.topLeft.Y);
  gameEngine.border.color = COLOR_WHITE;
  gameEngine.isDrawBorder = false;
  gameEngine.backgroundColor = COLOR_BLACK;
  gameEngine.gridSize = GRID_SIZE;
  gameEngine.gridNo.width = GRID_NO_CALCULATE(gameEngine.border.rectangleDimension.width, gameEngine.gridSize);
  gameEngine.gridNo.height = GRID_NO_CALCULATE(gameEngine.border.rectangleDimension.height, gameEngine.gridSize);
}

void initPlayer()
{
  head = (player_t *)malloc(sizeof(player_t));
  head->body.radius = RADIUS_CALCULATE(gameEngine.gridSize);
  head->body.center.X = CENTER_POSITION_CALCULATE(gameEngine.gridNo.width, gameEngine.gridSize, head->body.radius);
  head->body.center.Y = CENTER_POSITION_CALCULATE(gameEngine.gridNo.height, gameEngine.gridSize, head->body.radius);
  head->body.topLeft.X = SQUARE_TOP_LEFT_CALCULATE(head->body.center.X, head->body.radius);
  head->body.topLeft.Y = SQUARE_TOP_LEFT_CALCULATE(head->body.center.Y, head->body.radius);
  head->body.bottomRight.X = SQUARE_BOTTOM_RIGHT_CALCULATE(head->body.center.X, head->body.radius);
  head->body.bottomRight.Y = SQUARE_BOTTOM_RIGHT_CALCULATE(head->body.center.Y, head->body.radius);
  head->body.color = COLOR_WHITE;
  head->direction = up;
  head->nextDirection = up;
  head->next = (player_t *)malloc(sizeof(player_t));
  head->next = NULL;
}

void initItem()
{
  food.body.center.X = START_POSITION;
  food.body.center.Y = START_POSITION;
  food.body.color = COLOR_RED;
  food.body.radius = 1;
  food.isSpawn = false;
  food.score = 10;
  food.isFirstDraw = false;
}

void initBarricade()
{
  straightWall.body.center.X = START_POSITION;
  straightWall.body.center.Y = START_POSITION;
  straightWall.body.radius = gameEngine.gridSize / 2;
  straightWall.body.topLeft.X = (straightWall.body.center.X - straightWall.body.radius);
  straightWall.body.topLeft.Y = (straightWall.body.center.Y - (straightWall.body.radius + (2 * gameEngine.gridSize)));
  straightWall.body.bottomRight.X = (straightWall.body.center.X + straightWall.body.radius);
  straightWall.body.bottomRight.Y = (straightWall.body.center.Y + (straightWall.body.radius + (2 * gameEngine.gridSize)));
  straightWall.body.rectangleDimension.width = straightWall.body.bottomRight.X - straightWall.body.topLeft.X;
  straightWall.body.rectangleDimension.height = straightWall.body.bottomRight.Y - straightWall.body.topLeft.Y;
  straightWall.body.color = COLOR_GREEN;
  straightWall.isDraw = false;
}

void drawBorder()
{
  tft.drawRectangle(gameEngine.border.topLeft.X,
                    gameEngine.border.topLeft.Y,
                    gameEngine.border.bottomRight.X,
                    gameEngine.border.bottomRight.Y,
                    gameEngine.border.color);

  gameEngine.isDrawBorder = !gameEngine.isDrawBorder;
}

void firstDrawItem()
{
  randomSeed(millis());

  food.body.center.X = int(random(gameEngine.border.topLeft.X + 2, gameEngine.gridNo.width - 2));
  food.body.center.Y = int(random(gameEngine.border.topLeft.Y + 2, gameEngine.gridNo.height - 2));

  food.body.center.X = (food.body.center.X * gameEngine.gridSize) - (gameEngine.gridSize / 2);
  food.body.center.Y = (food.body.center.Y * gameEngine.gridSize) - (gameEngine.gridSize / 2);

  food.isFirstDraw = !food.isFirstDraw;
}

void drawItem()
{
  tft.fillCircle(food.body.center.X, food.body.center.Y, food.body.radius, food.body.color);

  food.isSpawn = !food.isSpawn;
}

void drawBarricade()
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

  tft.fillRectangle(straightWall.body.topLeft.X,
                    straightWall.body.topLeft.Y,
                    straightWall.body.bottomRight.X,
                    straightWall.body.bottomRight.Y,
                    straightWall.body.color);

  straightWall.isDraw = !straightWall.isDraw;
}

void isHitBorder()
{
  boolean hitLeftBorder = head->body.topLeft.X <= gameEngine.border.topLeft.X;
  boolean hitTopBorder = head->body.topLeft.Y <= gameEngine.border.topLeft.Y;
  boolean hitRightBorder = head->body.bottomRight.X >= gameEngine.border.bottomRight.X;
  boolean hitBottomBorder = head->body.bottomRight.Y >= gameEngine.border.bottomRight.Y;
  gameEngine.isHitBorder = hitLeftBorder || hitTopBorder || hitRightBorder || hitBottomBorder;
}

boolean isHitItem()
{
  int cx = food.body.center.X;
  int cy = food.body.center.Y;
  int radius = food.body.radius;
  int rw = head->body.rectangleDimension.width;
  int rh = head->body.rectangleDimension.height;
  int rx = head->body.center.X;
  int ry = head->body.center.Y;
  // PRINT_NEW_LINE("Circle X : " + String(cx) +
  //                " Y : " + String(cy) +
  //                "Rectangle X  : " + String(rx) +
  //                " Y  : " + String(ry));
  int distX = abs(cx - rx - (rw / 2));
  int distY = abs(cy - ry - (rh / 2));

  if (distX > ((rw / 2) + radius))
  {
    return false;
  }
  if (distY > ((rh / 2) + radius))
  {
    return false;
  }

  if (distX <= (rw / 2))
  {
    return true;
  }
  if (distY <= (rh / 2))
  {
    return true;
  }

  int dx = distX - (rw / 2);
  int dy = distY - (rh / 2);

  return ((dx * dx) + (dy * dy) <= (radius * radius));
  // return cx == rx && cy == ry;
}
void isHitBarricade()
{
  boolean collisionLeft = head->body.topLeft.X + head->body.rectangleDimension.width >= straightWall.body.topLeft.X;
  boolean collisionRight = head->body.topLeft.X <= straightWall.body.topLeft.X + straightWall.body.rectangleDimension.width;
  boolean collisionBottom = head->body.topLeft.Y + head->body.rectangleDimension.height >= straightWall.body.topLeft.Y;
  boolean collisionTop = head->body.topLeft.Y <= straightWall.body.topLeft.Y + straightWall.body.rectangleDimension.height;

  gameEngine.isHitBarricade = collisionLeft && collisionRight && collisionTop && collisionBottom;
}

void isGameOver()
{
  if (gameEngine.isHitBarricade || gameEngine.isHitBorder)
  {
    tft.clear();
    tft.setBackgroundColor(COLOR_BLACK);
    gameEngine.mode = gameOver;
  }
}

void reDrawBarricade()
{
  PRINT_IN_LINE("Food Location X : " + String(food.body.center.X) +
                " Y : " + String(food.body.center.Y));
  food.body.center.X = straightWall.body.center.X;
  food.body.center.Y = straightWall.body.center.Y;
  tft.fillRectangle(straightWall.body.topLeft.X,
                    straightWall.body.topLeft.Y,
                    straightWall.body.bottomRight.X,
                    straightWall.body.bottomRight.Y,
                    gameEngine.backgroundColor);
  tft.fillCircle(food.body.center.X, food.body.center.Y, food.body.radius, food.body.color);
  PRINT_NEW_LINE(" ==> X " + String(food.body.center.X) +
                 " Y : " + String(food.body.center.Y));
}

void addTail()
{
  player_t *newTail = head;
  while (newTail->next != NULL)
  {
    newTail = newTail->next;
  }

  newTail->next = (player_t *)malloc(sizeof(player_t));
  newTail->next->body.center.X = newTail->body.center.X;
  newTail->next->body.center.Y = newTail->body.center.Y;
  switch (newTail->direction)
  {
  case up:
  {
    newTail->next->body.center.Y += gameEngine.gridSize;
    break;
  };
  case down:
  {
    newTail->next->body.center.Y -= +gameEngine.gridSize;
    break;
  };
  case left:
  {
    newTail->next->body.center.X -= gameEngine.gridSize;
    break;
  };
  case right:
  {
    newTail->next->body.center.X += gameEngine.gridSize;
    break;
  };
  default:
    break;
  }
  newTail->next->body.radius = newTail->body.radius;
  newTail->next->body.topLeft.X = newTail->next->body.center.X - newTail->next->body.radius;
  newTail->next->body.topLeft.Y = newTail->next->body.center.Y - newTail->next->body.radius;
  newTail->next->body.bottomRight.X = newTail->next->body.center.X + newTail->next->body.radius;
  newTail->next->body.bottomRight.Y = newTail->next->body.center.Y + newTail->next->body.radius;
  newTail->next->nextDirection = newTail->direction;
  newTail->next->direction = newTail->direction;
  newTail->next->body.color = newTail->body.color;
  newTail->next->next = NULL;
}

void hitItemAction()
{
  gameEngine.score += food.score;
  gameEngine.isHitItem = !gameEngine.isHitItem;
  straightWall.isDraw = !straightWall.isDraw;
  // food.isSpawn = !food.isSpawn;
}

void deleteTail()
{
  player_t *tail = head;
  while (tail->next != NULL)
  {
    tail = tail->next;
  }
  tail->body.topLeft.X = tail->body.center.X - tail->body.radius;
  tail->body.topLeft.Y = tail->body.center.Y - tail->body.radius;
  tail->body.bottomRight.X = tail->body.center.X + tail->body.radius;
  tail->body.bottomRight.Y = tail->body.center.Y + tail->body.radius;
  // PRINT_NEW_LINE("Delete AT   X : " + String(tail->body.center.X) +
  //                " Y : " + String(tail->body.center.Y));

  tft.fillRectangle(tail->body.topLeft.X,
                    tail->body.topLeft.Y,
                    tail->body.bottomRight.X,
                    tail->body.bottomRight.Y,
                    COLOR_BLACK);

  tft.fillRectangle(tail->body.topLeft.X,
                    tail->body.topLeft.Y,
                    tail->body.bottomRight.X,
                    tail->body.bottomRight.Y,
                    COLOR_BLACK);
  tft.fillRectangle(tail->body.topLeft.X,
                    tail->body.topLeft.Y,
                    tail->body.bottomRight.X,
                    tail->body.bottomRight.Y,
                    COLOR_BLACK);
  tft.fillRectangle(tail->body.topLeft.X,
                    tail->body.topLeft.Y,
                    tail->body.bottomRight.X,
                    tail->body.bottomRight.Y,
                    COLOR_BLACK);
}
void updateDirectionReverse(player_t *head, int number, int nextDirection)
{
  // Base case
  if (head == NULL)
    return;
  number++;
  updateDirectionReverse(head->next, number, head->direction);
  head->direction = nextDirection;
}
void updateSnakeDirection()
{

  player_t *cur = head;
  int size = 0;
  while (cur != NULL)
  {
    size++;
    cur = cur->next;
  }
  // PRINT_NEW_LINE("Size : " + String(size));
  int *arr = (int *)malloc(size * sizeof(int));
  int nextData = head->nextDirection;
  // PRINT_NEW_LINE("Next Data : " + String(nextData));
  cur = NULL;
  cur = head;
  int index = 0;
  while (cur != NULL)
  {
    *(arr + index) = cur->direction;
    index += 1;
    cur = cur->next;
  }
  for (int k = size - 1; k > 0; --k)
  {
    *(arr + k) = *(arr + (k - 1));
  }
  *(arr) = nextData;

  cur = NULL;
  cur = head;
  index = 0;
  while (cur != NULL)
  {
    cur->direction = *(arr + index);
    index += 1;
    cur = cur->next;
  }
  /*
    PRINT_NEW_LINE("===== PRINT SNAKE =====");
    player_t *print = head;
    int number = 0;
    while (print != NULL)
    {
        PRINT_NEW_LINE("NO . " + String(number++) + " Direction :  " + String(print->direction));
        print = print->next;
    }
    PRINT_NEW_LINE("=======================");*/
}
void updateSnakePosition()
{

  player_t *current_pos = head;
  while (current_pos != NULL)
  {
    switch (current_pos->direction)
    {
    case up:
      current_pos->body.center.Y -= gameEngine.gridSize;
      break;
    case down:
      current_pos->body.center.Y += gameEngine.gridSize;
      break;
    case left:
      current_pos->body.center.X -= gameEngine.gridSize;
      break;
    case right:
      current_pos->body.center.X += gameEngine.gridSize;
      break;
    default:
      break;
    }
    current_pos->body.topLeft.X = current_pos->body.center.X - current_pos->body.radius;
    current_pos->body.topLeft.Y = current_pos->body.center.Y - current_pos->body.radius;
    current_pos->body.bottomRight.X = current_pos->body.center.X + current_pos->body.radius;
    current_pos->body.bottomRight.Y = current_pos->body.center.Y + current_pos->body.radius;

    current_pos = current_pos->next;
  }
}

void addHead()
{
  updateSnakeDirection();
  updateSnakePosition();
  tft.fillRectangle(head->body.topLeft.X,
                    head->body.topLeft.Y,
                    head->body.bottomRight.X,
                    head->body.bottomRight.Y,
                    head->body.color);
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

  int pinA = digitalRead(PIN_A);
  int pinB = digitalRead(PIN_B);
  int pinC = digitalRead(PIN_C);
  int pinD = digitalRead(PIN_D);
  int currentStatus;

  if (pinA == 0)
  {
    currentStatus = down;
  }
  if (pinB == 0)
  {
    currentStatus = left;
  }
  if (pinC == 0)
  {
    currentStatus = up;
  }
  if (pinD == 0)
  {
    currentStatus = right;
  }
  if (pinA == 1 && pinB == 1 && pinC == 1 && pinD == 1)
  {
    currentStatus = stable;
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

int readFromKeyboard()
{
  int currentStatus = stable;
  int incomingByte = 0;
  incomingByte = Serial.read();
  if (incomingByte > -1)
  {
    // PRINT_NEW_LINE("Read From Serial " + String(incomingByte));
    switch (incomingByte)
    {
    case 119:
      currentStatus = up;
      break;
    case 97:
      currentStatus = left;
      break;
    case 100:
      currentStatus = right;
      break;
    case 115:
      currentStatus = down;
      break;
    case 32:
      currentStatus = 99;
      break;
    default:
      currentStatus = stable;
      break;
    }
  }
  else
  {
    currentStatus = stable;
  }
  return currentStatus;
}