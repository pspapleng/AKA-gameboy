#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>
#include "TrueRandom.h"
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
};

struct Barricade
{
    struct rectangle body;
    boolean isDraw;
};

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

struct engine gameEngine;
struct item food;
struct Barricade straightWall;
player_t *head = NULL;

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

    head = (player_t *)malloc(sizeof(player_t));
    head->body.radius = gameEngine.gridSize / 2;
    head->body.center.X = ((gameEngine.gridNo.width / 2) * gameEngine.gridSize) - head->body.radius;
    head->body.center.Y = ((gameEngine.gridNo.height / 2) * gameEngine.gridSize) - head->body.radius;
    head->body.topLeft.X = head->body.center.X - head->body.radius;
    head->body.topLeft.Y = head->body.center.Y - head->body.radius;
    head->body.bottomRight.X = head->body.center.X + head->body.radius;
    head->body.bottomRight.Y = head->body.center.Y + head->body.radius;
    head->body.color = COLOR_WHITE;
    head->direction = up;
    head->nextDirection = up;
    head->next = (player_t *)malloc(sizeof(player_t));
    head->next = NULL;

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
        /*
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
                // PRINT_NEW_LINE("RANDOM X Start : " + String(gameEngine.border.topLeft.X + 2) + "  End : " + String(gameEngine.gridNo.width - 2));
                // PRINT_NEW_LINE("RANDOM X Start : " + String(gameEngine.border.topLeft.Y + 2) + "  End : " + String(gameEngine.gridNo.height - 2));

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

            // PRINT_NEW_LINE("Food Center   X : " + String(food.body.center.X) + "  Y : " + food.body.center.Y);
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

            // PRINT_NEW_LINE("WALL Width : " + String(straightWall.body.rectangleDimension.width) +
            //                "  Height : " + String(straightWall.body.rectangleDimension.height));

            tft.fillRectangle(straightWall.body.topLeft.X,
                              straightWall.body.topLeft.Y,
                              straightWall.body.bottomRight.X,
                              straightWall.body.bottomRight.Y,
                              straightWall.body.color);

            straightWall.isDraw = !straightWall.isDraw;
        }

        // Check Hit Border
        boolean hitLeftBorder = head->body.topLeft.X <= gameEngine.border.topLeft.X;
        boolean hitTopBorder = head->body.topLeft.Y <= gameEngine.border.topLeft.Y;
        boolean hitRightBorder = head->body.bottomRight.X >= gameEngine.border.bottomRight.X;
        boolean hitBottomBorder = head->body.bottomRight.Y >= gameEngine.border.bottomRight.Y;
        gameEngine.isHitBorder = hitLeftBorder || hitTopBorder || hitRightBorder || hitBottomBorder;

        // Check Hit Item
        int tempX = food.body.center.X;
        int tempY = food.body.center.Y;

        // which edge is closest?
        if (food.body.center.X < head->body.center.X)
            tempX = head->body.center.X; // test left edge
        else if (food.body.center.X > head->body.center.X + head->body.rectangleDimension.width)
            tempX = head->body.center.X + head->body.rectangleDimension.width; // right edge
        if (food.body.center.Y < head->body.center.Y)
            tempY = head->body.center.Y; // top edge
        else if (food.body.center.Y > head->body.center.Y + head->body.rectangleDimension.height)
            tempY = head->body.center.Y + head->body.rectangleDimension.height; // bottom edge

        // get distance from closest edges
        float distX = food.body.center.X - tempX;
        float distY = food.body.center.Y - tempY;
        float distance = sqrt((distX * distX) + (distY * distY));

        // if the distance is less than the radius, collision!
        if (distance <= food.body.radius)
        {
            gameEngine.isHitItem = true;
        }

        // Check Hit Walls

        boolean collisionLeft = head->body.topLeft.X + head->body.rectangleDimension.width >= straightWall.body.topLeft.X;
        boolean collisionRight = head->body.topLeft.X <= straightWall.body.topLeft.X + straightWall.body.rectangleDimension.width;
        boolean collisionBottom = head->body.topLeft.Y + head->body.rectangleDimension.height >= straightWall.body.topLeft.Y;
        boolean collisionTop = head->body.topLeft.Y <= straightWall.body.topLeft.Y + straightWall.body.rectangleDimension.height;

        gameEngine.isHitBarricade = collisionLeft && collisionRight && collisionTop && collisionBottom;
        // PRINT_NEW_LINE("Hit Walls Left : " + String(collisionLeft) +
        //                "  Right : " + String(collisionRight) +
        //                "  Top : " + String(collisionTop) +
        //                "  Bottom : " + String(collisionBottom));

        if (
            gameEngine.isHitBarricade ||
            gameEngine.isHitBorder)
        {
            gameEngine.mode = gameOver;
        }

        if (gameEngine.isHitItem)
        {
            PRINT_NEW_LINE("HIT ITEM");


            gameEngine.score += food.score;

            food.body.center.X = straightWall.body.center.X;
            food.body.center.Y = straightWall.body.center.Y;

            tft.fillRectangle(straightWall.body.topLeft.X,
                              straightWall.body.topLeft.Y,
                              straightWall.body.bottomRight.X,
                              straightWall.body.bottomRight.Y,
                              gameEngine.backgroundColor);

            // Update Tail Position
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
                newTail->next->body.center.Y = newTail->body.center.Y - gameEngine.gridSize;
                break;
            };
            case down:
            {
                newTail->next->body.center.Y = newTail->body.center.Y + gameEngine.gridSize;
                break;
            };
            case left:
            {
                newTail->next->body.center.X = newTail->body.center.X + gameEngine.gridSize;
                break;
            };
            case right:
            {
                newTail->next->body.center.X = newTail->body.center.X - gameEngine.gridSize;
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

            // Serial.println("Current Tail X : " + String(head->body.center.X) +
            //                " Y : " + String(head->body.center.Y) +
            //                " New Tail Center X : " + String(newTail->next->body.center.X) +
            //                "  Y : " + String(newTail->next->body.center.Y) +
            //                " R : " + String(newTail->next->body.radius) +
            //                " Top Left X : " + String(newTail->next->body.topLeft.X) +
            //                " Y : " + String(newTail->next->body.topLeft.Y) +
            //                " Bottom X : " + String(newTail->next->body.bottomRight.X) +
            //                " Y : " + String(newTail->next->body.bottomRight.Y));

            gameEngine.isHitItem = !gameEngine.isHitItem;
            straightWall.isDraw = !straightWall.isDraw;
            food.isSpawn = !food.isSpawn;
        }

        player_t *tail = head;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }

        Serial.println(" New Tail Center X : " + String(tail->body.center.X) +
                       "  Y : " + String(tail->body.center.Y) +
                       " R : " + String(tail->body.radius) +
                       " Top Left X : " + String(tail->body.topLeft.X) +
                       " Y : " + String(tail->body.topLeft.Y) +
                       " Bottom X : " + String(tail->body.bottomRight.X) +
                       " Y : " + String(tail->body.bottomRight.Y));

        tft.fillRectangle(tail->body.topLeft.X,
                          tail->body.topLeft.Y,
                          tail->body.bottomRight.X,
                          tail->body.bottomRight.Y,
                          gameEngine.backgroundColor);
*/
        // Update Tail Center Position

        // Update Snake Direction
        int readFromJoy = joyStickDecode();
        if (readFromJoy == 99)
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
            player_t *print_snake = head;
            int no = 0;
            PRINT_NEW_LINE("========= New Tail Detail =========");
            while (print_snake != NULL)
            {
                PRINT_NEW_LINE("NO : " + String(no++) + " X : " + String(print_snake->body.center.X) +
                               " Y : " + String(print_snake->body.center.Y));

                print_snake = print_snake->next;
            }
            PRINT_NEW_LINE("====================");
        }
        if (readFromJoy != stable && readFromJoy != 99)
        {

            player_t *tail = head;
            int tail_no = 0;
            while (tail->next != NULL)
            {
                tail_no++;
                tail = tail->next;
            }
            PRINT_NEW_LINE("Last Tail tail_no : " + String(tail_no) + " X : " + String(tail->body.center.X) +
                           " Y : " + String(tail->body.center.Y) +
                           " Top Left X : " + String(tail->body.topLeft.X) +
                           " Y : " + String(tail->body.topLeft.Y) +
                           " Bottom X : " + String(tail->body.bottomRight.X) +
                           " Y : " + String(tail->body.bottomRight.Y));
            PRINT_NEW_LINE("==================");
            tft.fillRectangle(tail->body.topLeft.X,
                              tail->body.topLeft.Y,
                              tail->body.bottomRight.X,
                              tail->body.bottomRight.Y,
                              COLOR_BLACK);
            head->nextDirection = readFromJoy;

            PRINT_NEW_LINE(String(joyStickAction2Str(head->nextDirection)));
            player_t *current = head;
            current->direction = current->nextDirection;
            while (current->next != NULL)
            {
                current->next->nextDirection = current->direction;
                current = current->next;
            }

            player_t *current_pos = head;
            int no = 0;
            PRINT_NEW_LINE("========= Update POS Detail =========");

            while (current_pos != NULL)
            {
                Serial.print("NO : " + String(no++) + " X : " + String(current_pos->body.center.X) +
                             " Y : " + String(current_pos->body.center.Y) + " ==> ");
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
                PRINT_NEW_LINE(" X : " + String(current_pos->body.center.X) +
                               " Y : " + String(current_pos->body.center.Y));
                current_pos = current_pos->next;
            }
            PRINT_NEW_LINE("==================");

            // Update Snake Center Position

            tft.fillRectangle(head->body.topLeft.X,
                              head->body.topLeft.Y,
                              head->body.bottomRight.X,
                              head->body.bottomRight.Y,
                              head->body.color);
        }
        // if (head->nextDirection == stable)
        // {
        //     head->nextDirection = gameEngine.lastDirection;
        // }
        // else
        // {
        //     gameEngine.lastDirection = head->nextDirection;
        // }

        break;
    }
    case gameOver:
    {
        tft.clear();
        tft.setBackgroundColor(COLOR_BLACK);
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

        head->body.radius = gameEngine.gridSize / 2;
        head->body.center.X = ((gameEngine.gridNo.width / 2) * gameEngine.gridSize) - head->body.radius;
        head->body.center.Y = ((gameEngine.gridNo.height / 2) * gameEngine.gridSize) - head->body.radius;
        head->body.topLeft.X = head->body.center.X - head->body.radius;
        head->body.topLeft.Y = head->body.center.Y - head->body.radius;
        head->body.bottomRight.X = head->body.center.X + head->body.radius;
        head->body.bottomRight.Y = head->body.center.Y + head->body.radius;
        head->body.color = COLOR_WHITE;
        head->direction = up;
        head->nextDirection = up;
        head->next = (player_t *)malloc(sizeof(player_t));
        head->next = NULL;

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
        tft.clear();
        gameEngine.mode = playGame;

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

    // if (analogX <= 445 && analogX >= 230 && analogY == 0)
    // {
    //     currentStatus = up;
    // }
    // else if (analogX > 350 && analogX < 400 && analogY > 10)
    // {
    //     currentStatus = down;
    // }
    // else if (analogX > 400 && analogX < 750 && analogY == 1)
    // {
    //     currentStatus = left;
    // }
    // else if (analogX < 330 && analogX > -1 && analogY == 1)
    // {
    //     currentStatus = right;
    // }
    // else
    // {
    //     currentStatus = stable;
    // }
    // RINT_NEW_LINE("Analog X : " + String(analogX) + "  Y : " + String(analogY) + " Status : " + joyStickAction2Str(currentStatus));

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

    // if (pinA == 0)
    // {
    //     currentStatus = down;
    // }
    // if (pinB == 0)
    // {
    //     currentStatus = left;
    // }
    // if (pinC == 0)
    // {
    //     currentStatus = up;
    // }
    // if (pinD == 0)
    // {
    //     currentStatus = right;
    // }
    // if (pinA == 1 && pinB == 1 && pinC == 1 && pinD == 1)
    // {
    //     currentStatus = stable;
    // }

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