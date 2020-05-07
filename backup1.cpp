/* Lib Declaration Zone */
#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>

/*  Define Zone */
#define TFT_RST A4
#define TFT_RS A3
#define TFT_CS A5  // SS
#define TFT_SDI A2 // MOSI
#define TFT_CLK A1 // SCK
#define TFT_LED 0  // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 200
#define SCREEN_WIDTH 174
#define SCREEN_HEIGHT 218
#define GRID_RECT_SIZE 4
#define BAUD_RATE 9600
#define SNAKE_COLOR COLOR_GREEN
#define WALL_COLOR COLOR_BLUE
#define FOOD_COLOR COLOR_RED
#define BACKGROUND_COLOR COLOR_BLACK
#define BORDER_COLOR COLOR_WHITE
#define FOOD_SCORE 10
#define WALL_WIDTH 4
#define WALL_HEIGHT 24
/* Macro Function Zone*/
#define DECODE_DIRECTION_2_STR(direction) (inputDirectionStr[direction])
#define DECODE_MODE_2_STR(mode) (gameModeStr[mode])
#define CALCULATE_CENTER(point, gridSize, radius) ((point * gridSize) + radius)
#define CALCULATE_TOP_LEFT(center, radius) (center - radius)
#define PRINT_NEW_LINE(str) (Serial.println(str))
#define PRINT_IN_LINE(str) (Serial.print(str))
#define BOARD_GET_TOP_LEFT_X(column, row) ((*(board + column) + row)->top_left_x)
#define BOARD_GET_TOP_LEFT_Y(column, row) ((*(board + column) + row)->top_left_y)
#define CALCULATE_BOTTOM_RIGHT(center, radius) (center + radius)
/* Enum Declare Zone */
enum inputDirection
{
    stable,
    up,
    down,
    left,
    right
};

enum gameMode
{
    startScreen,
    playGame,
    gameOver
};

enum state
{
    arduinoBoyLogo,
    snakeLogo,
    waitKeyPress,
    score
};

/* Struct Declare Zone */
typedef struct
{
    int row;
    int column;
    int radius;
    int lastInput;
    int score;
    int mode;
    int state;
    int input;
    boolean isDrawBorder;
    boolean isHitBorder;
    boolean isHitFood;
    boolean isHitWall;
} GameEngine_t;

typedef struct
{
    int col;
    int row;
    boolean isDraw;
} Food_t;

typedef struct
{
    int col;
    int row;
    boolean isDraw;
} Wall_t;

typedef struct
{
    int x;
    int y;
    int top_left_x;
    int top_left_y;
} Board_t;

struct Node
{
    int row;
    int col;
    int direction;
    struct Node *next;
    struct Node *prev;
};

/* Variable Declare Zone */
String inputDirectionStr[] = {"Stable ", "Up ", "Down ", "Left ", "Right "};
String gameModeStr[] = {"Start Screen", "Play Game", "Game Over"};

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);
GameEngine_t gameEngine;
Board_t **board = NULL;
Food_t food;
Wall_t wall;
struct Node *head = NULL;
/* Prototype Function Zone */
void board_init();
void board_pre_cal_func(int column, int row);
void board_pre_cal();
void board_print_cell(int column, int row);
void board_double_for(int column, int row, void (*cell)(int column, int row));
void board_print();

int util_get_key_press();
Node *snake_get_last_node(struct Node **head_ref);
void snake_add_tail();
void snake_body_update_position();
void snake_first_draw();
void snake_hit_food_action();
void snake_hit_wall_action();
void snake_init(int new_row, int new_col, int new_direction);
void snake_tail_delete();
void snake_tail_update_direction(int next_direction);
void snake_hit_border_action();
void util_draw_rect(int column, int row, int width, int height, uint16_t color);
void util_draw_square(int column, int row, uint16_t color);
void util_set_input();

void game_draw_food();
void game_draw_wall();
boolean game_rect_collision_detect(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h);

void setup()
{
    /* Default Setting */
    Serial.begin(BAUD_RATE);
    Serial.println("HEllo");
    gameEngine.row = SCREEN_HEIGHT / GRID_RECT_SIZE;
    gameEngine.column = SCREEN_WIDTH / GRID_RECT_SIZE;
    gameEngine.radius = GRID_RECT_SIZE / 2;
    gameEngine.isHitBorder = false;
    gameEngine.isHitFood = false;
    gameEngine.isHitWall = false;
    gameEngine.lastInput = up;
    gameEngine.mode = playGame;
    gameEngine.state = arduinoBoyLogo;
    gameEngine.isDrawBorder = false;
    food.col = -1;
    food.row = -1;
    food.isDraw = false;
    wall.isDraw = false;

    board_init();
    board_pre_cal();
    board_print();

    snake_init(gameEngine.row / 2, gameEngine.column / 2, up);
    snake_add_tail();
    snake_add_tail();
    snake_first_draw();
    tft.begin();

    // tft.setBackgroundColor(COLOR_BLACK);
}

void loop()
{
    PRINT_NEW_LINE("Row " + String(gameEngine.row));
    PRINT_NEW_LINE("Col " + String(gameEngine.column));

    gameEngine.input = util_get_key_press();

    switch (gameEngine.mode)
    {
    case startScreen:
    {
        switch (gameEngine.state)
        {
        case arduinoBoyLogo:
            break;
        case snakeLogo:
            break;
        case waitKeyPress:
            break;
        default:
            break;
        }
        break;
    }
    case playGame:
    {
        util_set_input();
        if (!food.isDraw)
        {
            game_draw_food();
        }
        if (!wall.isDraw)
        {
            game_draw_wall();
        }
        if (!gameEngine.isDrawBorder)
        {
            tft.drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BORDER_COLOR);
        }
        snake_tail_delete();
        snake_tail_update_direction(gameEngine.input);
        snake_body_update_position();
        util_draw_square(head->col, head->row, SNAKE_COLOR);
        snake_hit_food_action();
        snake_hit_wall_action();
        snake_hit_border_action();

        delay(300);
        break;
    }
    case gameOver:
    {
        switch (gameEngine.state)
        {
        case score:
            break;
        case waitKeyPress:
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void board_init()
{
    int column = gameEngine.column;
    int row = gameEngine.row;
    int radius = gameEngine.radius;

    board = (Board_t **)malloc(column * sizeof(Board_t *));
    for (int col = 0; col < column; ++col)
        board[col] = (Board_t *)malloc(row * sizeof(Board_t));

    for (int col = 0; col < column; col++)
    {
        for (int row_f = 0; row_f < row; row_f++)
        {
            int center_X = CALCULATE_CENTER(col, GRID_RECT_SIZE, radius);
            int center_Y = CALCULATE_CENTER(row_f, GRID_RECT_SIZE, radius);
            (*(board + col) + row_f)->top_left_x = CALCULATE_TOP_LEFT(center_X, radius);
            (*(board + col) + row_f)->top_left_y = CALCULATE_TOP_LEFT(center_Y, radius);
            int topLeft_X = BOARD_GET_TOP_LEFT_X(col, row_f);
            int topLeft_Y = BOARD_GET_TOP_LEFT_Y(col, row_f);
            char buffer[1024];
            sprintf(buffer, "Index [ %2d, %2d ] TopLeft X : %3d  Y : %3d", row_f, col, topLeft_X, topLeft_Y);
            PRINT_NEW_LINE(buffer);
        }
    }
}

void board_pre_cal()
{
    int row = gameEngine.row;
    int column = gameEngine.column;
    //board_double_for(column, row, board_pre_cal_func);
    return;
}
void board_print_cell(int column, int row)
{
    char buffer[1024];
    int topLeft_X = BOARD_GET_TOP_LEFT_X(column, row);
    int topLeft_Y = BOARD_GET_TOP_LEFT_Y(column, row);
    sprintf(buffer, "Index [ %2d, %2d ] TopLeft X : %3d  Y : %3d", row, column, topLeft_X, topLeft_Y);
    PRINT_NEW_LINE((buffer));
    return;
}

void board_double_for(int column, int row, void (*cell)(int column, int row))
{
    for (int col = 0; col < column; col++)
    {
        for (int row_f = 0; row_f < row; row_f++)
        {
            cell(col, row_f);
        }
    }
    return;
}

void board_print()
{
    int column = gameEngine.column;
    int row = gameEngine.row;
    board_double_for(column, row, board_print_cell);
    return;
}

void board_pre_cal_func(int column, int row)
{
    int radius = gameEngine.radius;
    (*(board + column) + row)->x = CALCULATE_CENTER(column, GRID_RECT_SIZE, radius);
    (*(board + column) + row)->y = CALCULATE_CENTER(row, GRID_RECT_SIZE, radius);
    int center_X = (*(board + column) + row)->x;
    int center_Y = (*(board + column) + row)->y;
    (*(board + column) + row)->top_left_x = CALCULATE_TOP_LEFT(center_X, radius);
    (*(board + column) + row)->top_left_y = CALCULATE_TOP_LEFT(center_Y, radius);
    PRINT_NEW_LINE("In Pre Cal");
    return;
}

void snake_init(int new_row, int new_col, int new_direction)
{
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    struct Node *last = head;

    new_node->row = new_row;
    new_node->col = new_col;
    new_node->direction = new_direction;
    new_node->next = NULL;

    if (head == NULL)
    {
        new_node->prev = NULL;
        head = new_node;
        return;
    }
    while (last->next != NULL)
        last = last->next;
    last->next = new_node;
    new_node->prev = last;

    return;
}

void snake_add_tail()
{
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    struct Node *last = head;
    struct Node *last_tail = snake_get_last_node(&last);

    new_node->row = last_tail->row;
    new_node->col = last_tail->col;
    switch (last_tail->direction)
    {
    case up:
        new_node->row += 1;
        break;
    case down:
        new_node->row -= 1;
        break;
    case left:
        new_node->col += 1;
        break;
    case right:
        new_node->col -= 1;
        break;
    default:
        break;
    }

    new_node->direction = last_tail->direction;

    new_node->next = NULL;
    if (head == NULL)
    {
        new_node->prev = NULL;
        head = new_node;
        return;
    }
    while (last->next != NULL)
        last = last->next;
    last->next = new_node;
    new_node->prev = last;

    return;
}

Node *snake_get_last_node(struct Node **head_ref)
{
    struct Node *tail = *head_ref;

    while (tail->next != NULL)
    {

        tail = tail->next;
    }

    return tail;
}

void snake_first_draw()
{
    struct Node *tail = head;

    while (tail != NULL)
    {
        int column = tail->col;
        int row = tail->row;
        util_draw_square(column, row, SNAKE_COLOR);
        tail = tail->next;
    }
}

void util_draw_square(int column, int row, uint16_t color)
{
    int topleft_X = BOARD_GET_TOP_LEFT_X(column, row);
    int topleft_Y = BOARD_GET_TOP_LEFT_Y(column, row);
    int bottom_right_X = CALCULATE_BOTTOM_RIGHT(topleft_X, GRID_RECT_SIZE);
    int bottom_right_Y = CALCULATE_BOTTOM_RIGHT(topleft_Y, GRID_RECT_SIZE);
    tft.fillRectangle(topleft_X, topleft_Y, bottom_right_X, bottom_right_Y, color);
    /* add bottom right calculate here*/
    // M5.Lcd.fillRect(topleft_X, topleft_Y, GRID_RECT_SIZE, GRID_RECT_SIZE, color);
}

int util_get_key_press()
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

void util_set_input()
{
    if (gameEngine.input == stable)
    {
        gameEngine.input = gameEngine.lastInput;
    }
    else
    {
        gameEngine.lastInput = gameEngine.input;
    }
}

void game_draw_food()
{
    if (food.col == -1 && food.row == -1)
    {
        randomSeed(millis());
        food.col = random(10, gameEngine.column - 10);
        food.row = random(10, gameEngine.row - 10);
    }

    util_draw_square(food.col, food.row, FOOD_COLOR);
    food.isDraw = !food.isDraw;
}

void game_draw_wall()
{
    wall.col = random(2, gameEngine.column);
    wall.row = random(2, gameEngine.row);
    util_draw_rect(wall.col, wall.row, 4, 24, WALL_COLOR);
    wall.isDraw = !wall.isDraw;
}

void util_draw_rect(int column, int row, int width, int height, uint16_t color)
{
    int topleft_X = BOARD_GET_TOP_LEFT_X(column, row);
    int topleft_Y = BOARD_GET_TOP_LEFT_Y(column, row);
    int bottom_right_X = CALCULATE_BOTTOM_RIGHT(topleft_X, width);
    int bottom_right_Y = CALCULATE_BOTTOM_RIGHT(topleft_Y, height);
    /* add bottom right calculate here*/
    tft.fillRectangle(topleft_X, topleft_Y, bottom_right_X, bottom_right_Y, color);
    // M5.Lcd.fillRect(topleft_X, topleft_Y, width, height, color);
}

void snake_tail_delete()
{
    struct Node *head_temp = head;
    struct Node *temp = snake_get_last_node(&head_temp);
    int row = temp->row;
    int column = temp->col;
    util_draw_square(column, row, BACKGROUND_COLOR);
}

void snake_tail_update_direction(int next_direction)
{
    struct Node *tail = head;
    while (tail->next != NULL)
    {
        tail = tail->next;
    }
    while (tail != head)
    {
        tail->direction = tail->prev->direction;

        tail = tail->prev;
    }
    tail->direction = next_direction;
}

void snake_body_update_position()
{
    struct Node *cur_node = head;

    while (cur_node != NULL)
    {
        int direction = cur_node->direction;
        switch (direction)
        {
        case up:
            cur_node->row -= 1;
            break;
        case down:
            cur_node->row += 1;
            break;
        case left:
            cur_node->col -= 1;
            break;
        case right:
            cur_node->col += 1;
            break;
        default:
            break;
        }
        cur_node = cur_node->next;
    }
}
void snake_hit_food_action()
{
    int rect_2_topleft_X = BOARD_GET_TOP_LEFT_X(food.col, food.row);
    int rect_2_topleft_Y = BOARD_GET_TOP_LEFT_Y(food.col, food.row);
    int rect_1_topleft_X = BOARD_GET_TOP_LEFT_X(head->col, head->row);
    int rect_1_topleft_Y = BOARD_GET_TOP_LEFT_Y(head->col, head->row);

    if (game_rect_collision_detect(rect_1_topleft_X, rect_1_topleft_Y, GRID_RECT_SIZE, GRID_RECT_SIZE, rect_2_topleft_X, rect_2_topleft_Y, GRID_RECT_SIZE, GRID_RECT_SIZE))
    {
        PRINT_NEW_LINE("HIT Food");
        snake_add_tail();
        gameEngine.score += FOOD_SCORE;
        food.col = wall.col;
        food.row = wall.row;
        util_draw_rect(wall.col, wall.row, WALL_WIDTH, WALL_HEIGHT, BACKGROUND_COLOR);
        wall.isDraw = !wall.isDraw;
        food.isDraw = !food.isDraw;
    }
}

boolean game_rect_collision_detect(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h)
{

    // are the sides of one rectangle touching the other?

    if (r1x + r1w >= r2x && // r1 right edge past r2 left
        r1x <= r2x + r2w && // r1 left edge past r2 right
        r1y + r1h >= r2y && // r1 top edge past r2 bottom
        r1y <= r2y + r2h)
    { // r1 bottom edge past r2 top
        return true;
    }
    return false;
}

void snake_hit_wall_action()
{
    int rect_2topleft_X = BOARD_GET_TOP_LEFT_X(wall.col, wall.row);
    int rect_2topleft_Y = BOARD_GET_TOP_LEFT_Y(wall.col, wall.row);
    int rect_1_topleft_X = BOARD_GET_TOP_LEFT_X(head->col, head->row);
    int rect_1_topleft_Y = BOARD_GET_TOP_LEFT_Y(head->col, head->row);

    if (game_rect_collision_detect(rect_1_topleft_X, rect_1_topleft_Y, GRID_RECT_SIZE, GRID_RECT_SIZE, rect_2topleft_X, rect_2topleft_Y, WALL_WIDTH, WALL_HEIGHT))
    {
        PRINT_NEW_LINE("Game Over");
        gameEngine.mode = gameOver;
        wall.isDraw = !wall.isDraw;
    }
}

void snake_hit_border_action()
{
    int head_top_left_X = BOARD_GET_TOP_LEFT_X(head->col, head->row);
    int head_top_left_Y = BOARD_GET_TOP_LEFT_Y(head->col, head->row);
    int head_bottom_right_X = CALCULATE_BOTTOM_RIGHT(head_top_left_X, gameEngine.radius);
    int head_bottom_right_Y = CALCULATE_BOTTOM_RIGHT(head_top_left_Y, gameEngine.radius);
    boolean hitLeftBorder = head_top_left_X <= 0;
    boolean hitTopBorder = head_top_left_Y <= 0;
    boolean hitRightBorder = head_bottom_right_X >= SCREEN_WIDTH;
    boolean hitBottomBorder = head_bottom_right_Y >= SCREEN_HEIGHT;

    if (hitLeftBorder || hitTopBorder || hitRightBorder || hitBottomBorder)
    {
        PRINT_NEW_LINE("HIT Border");
        gameEngine.mode = gameOver;
    }
}