/**
 * ! Open With 
 * ? VSCODE : https://code.visualstudio.com/
 * ! Require Extension
 * ? Platform Io : https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide
 * ? Better Comment : https://marketplace.visualstudio.com/items?itemName=aaron-bond.better-comments
 * ! Require Library
 * ? M5Stack : https://platformio.org/lib/show/1851/M5Stack
 * ! Require Mood For Read Code
 * ? Lofi-hiphop : https://www.youtube.com/watch?v=5qap5aO4i9A
 * 
*/

// * Library Declaration Zone
#include <Arduino.h> //? Basic Library For Use Any Arduino Framework Function
#include <M5Stack.h> //? Basic Library For Use Any Module And Funtion Of M5Stack

//* Define Constant Variable Zone
#define SCREEN_WIDTH 320       //? Set Screen Width Of Device
#define SCREEN_HEIGHT 240      //? Set Screen Height Of Device
#define GRID_RECT_SIZE 8       //? Set Grid Size Of Game
#define BAUD_RATE 115200       //? Set Baud Rate
#define SNAKE_COLOR GREEN      //? Set Snake Color
#define WALL_COLOR BLUE        //? Set Wall Color
#define FOOD_COLOR RED         //? Set Food Color
#define BACKGROUND_COLOR BLACK //? Set Backgroud Color
#define FOOD_SCORE 10          //? Set Food Score
#define WALL_WIDTH 8           //? Set Wall Width
#define WALL_HEIGHT 48         //? Set Wall Height
#define INTERVAL 500
//* Macro Function Zone
//! Use Macro Function Because Reduce Memory Usage
/**
 * DECODE_DIRECTION_2_STR
 * ? Return String Of Input Direction Enum 
 * @param direction Current Direction 
*/
#define DECODE_DIRECTION_2_STR(direction) (inputDirectionStr[direction])
/**
 * DECODE_MODE_2_STR
 * ? Return String Of Game Mode Enum 
 * @param mode Current Game Mode 
*/
#define DECODE_MODE_2_STR(mode) (gameModeStr[mode])
/**
 * CALCULATE_CENTER
 * ? Return Int Of Center Of Grid Box
 * @param point row or column 
 * @param gridSize dimention Of Grid Box(width or height)
 * @param radius radius of Grid Box
*/
#define CALCULATE_CENTER(point, gridSize, radius) ((point * gridSize) + radius)
/**
 * CALCULATE_TOP_LEFT
 * ? Return Int Of Top Left Pos
 * @param center x or y position
 * @param radius radius of Grid Box
*/
#define CALCULATE_TOP_LEFT(center, radius) (center - radius)
/**
 * PRINT_NEW_LINE
 * ? Print New Line Use In Macro Because Can Delete Inside Function And Diable All Debug 
 * @param str string of text
*/
#define PRINT_NEW_LINE(str) (Serial.println(str))
/**
 * PRINT_IN_LINE
 * ? Print In Line Use In Macro Because Can Delete Inside Function And Diable All Debug 
 * @param str string of text
*/
#define PRINT_IN_LINE(str) (Serial.print(str))
/**
 * BOARD_GET_TOP_LEFT_X
 * ? Retun Int Top Left X Of grid Box Relate row and column
 * @param column column of Grid Box 
 * @param row row of Grid Box 
*/
#define BOARD_GET_TOP_LEFT_X(column, row) ((*(board + column) + row)->top_left_x)
/**
 * BOARD_GET_TOP_LEFT_Y
 * ? Retun Int Top Left Y Of grid Box Relate row and column
 * @param column column of Grid Box 
 * @param row row of Grid Box 
*/
#define BOARD_GET_TOP_LEFT_Y(column, row) ((*(board + column) + row)->top_left_y)
/**
 * CALCULATE_BOTTOM_RIGHT
 * ? Retun Int Top Left Y Of grid Box Relate row and column
 * @param center x or y position
 * @param radius radius of Grid Box
*/
#define CALCULATE_BOTTOM_RIGHT(center, radius) (center + radius)

//* Enum Declare Zone
/**
 *  inputDirection
 * ? Declare Direction in Enum For Use In Switch Case
 * @param stable dont Move
 * @param up move up
 * @param down move down
 * @param left move left
 * @param right move right
*/
enum inputDirection
{
    stable,
    up,
    down,
    left,
    right
};

/**
 *  gameMode
 * ? Declare Game Mode In Enum For Use In Switch
 * @param startScreen in start screen 
 * @param playGame in play game
 * @param gameOver in game over and show score
*/
enum gameMode
{
    startScreen,
    playGame,
    gameOver
};

/**
 *  state
 * ? Declare State Use in Start Screen And Game Over Screen
 * @param arduinoBoyLogo in start screen 
 * @param snakeLogo in play game
 * @param waitKeyPress in game over and show score
 * @param score 
*/
enum state
{
    arduinoBoyLogo,
    snakeLogo,
    waitKeyPress,
    score
};

// * Declare Typedef Struct Zone
/**
 * GameEngine_t
 * ? Declare Typedef For Make New Variable And Store Game Engine Data
 * @param row       total row in game board
 * @param column    total column in game board
 * @param radius    radius of each grid box
 * @param lastInput store last input from any input device and use in continuous move in play game
 * @param score     store score
 * @param mode      store game mode
 * @param state     store state in start screen and game over screen
 * @param input     store current input from any device
 * @param isDrawBorder store draw border state for draw once 
 * @param isDrawSnake  store draw snake state for draw once 
*/
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
    boolean isDrawSnake;
} GameEngine_t;

/**
 * Food_t
 * ? Declare Typedef For Make New Variable And Store Food Data
 * @param col store current column  in grid system
 * @param row store current row in grid system of food
 * @param isDraw store draw state for draw once
*/
typedef struct
{
    int col;
    int row;
    boolean isDraw;
} Food_t;

/**
 * Food_t
 * ? Declare Typedef For Make New Variable And Store Food Data
 * @param col store current column  in grid system of wall
 * @param row store current row in grid system of wall
 * @param isDraw store draw state for draw once
*/
typedef struct
{
    int col;
    int row;
    boolean isDraw;
} Wall_t;

/**
 * Board_t
 * ? Declare Typedef For Make New Variable And Store Food Data
 * @param x center x of each grid box 
 * @param y center y of each grid box
 * @param top_left_x top left x position of each grid box 
 * @param top_left_y top left y position of each grid box 
*/
typedef struct
{
    int x;
    int y;
    int top_left_x;
    int top_left_y;
} Board_t;

/**
 * Node
 * ?  Node Of Each Snake Body In Double Linklist
 * @param row store current row of snake body 
 * @param col store current column of snake body
 * @param direction store current direction of body
 * @param next pointer to next snake body
 * @param prev pointer to prev snake body 
*/
struct Node
{
    int row;
    int col;
    int direction;
    struct Node *next;
    struct Node *prev;
};

//* Variable Declare Zone
String inputDirectionStr[] = {"Stable ", "Up ", "Down ", "Left ", "Right "};
String gameModeStr[] = {"Start Screen", "Play Game", "Game Over"};

GameEngine_t gameEngine;          //Declare Game Engine Variable
Board_t **board = NULL;           //Declare Board Variable As Double Pointer For Dynamic 2 D Array Allocation
Food_t food;                      //Delacre Food Variable
Wall_t wall;                      //Decalre Wall Variable
struct Node *head = NULL;         // Declare Node As head of snake in double linklist
int toggleState = LOW;            //Declare toggle Use In Blink Print
unsigned long previousMillis = 0; //Use In Millis Loop

//* Prototype Function Zone
boolean game_rect_collision_detect(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h);
int util_get_key_press();
Node *snake_get_last_node();
void board_double_for(int column, int row, void (*cell)(int column, int row));
void board_init();
void board_pre_cal_func(int column, int row);
void board_pre_cal();
void board_print_cell(int column, int row);
void board_print();
void game_draw_food();
void game_draw_score();
void game_draw_wall();
void game_game_over_screen();
void game_init_param();
void game_start_screen_aka_logo();
void game_start_screen_snake_logo();
void game_start_screen_wait_key_press(int column, int row, int size, String text);
void snake_add_tail();
void snake_body_update_position();
void snake_first_draw();
void snake_hit_border_action();
void snake_hit_food_action();
void snake_hit_wall_action();
void snake_init(int new_row, int new_col, int new_direction);
void snake_tail_delete();
void snake_tail_update_direction(int next_direction);
void util_draw_rect(int column, int row, int width, int height, uint16_t color);
void util_draw_square(int column, int row, uint16_t color);
void util_lcd_print(int column, int row, int size, String text);
void util_set_input();
void snake_body_init();

void setup()
{
    //* Default Setting
    M5.begin(); //Init M5Stack
    Serial.begin(BAUD_RATE);
    game_init_param(); // Init Game Parameter
    board_init();      //Init Grid Array
    board_pre_cal();   // Pre Calculate Center And Top Left OF Each Array
    //board_print();
    snake_body_init(); //Init Snake Parameter
}

void loop()
{
    gameEngine.input = util_get_key_press(); //Get Key Press
    /**
   * main game switch case chage by mode in game engine
   * @param startscreen render start screen logo and wait to start
   * @param playGame render game play and other alogorithm
   * @param gameOver render gameOver and score 
   */
    switch (gameEngine.mode)
    {
    case startScreen:
    {
        /**
    * start screen render switch case change by state in game engine
    * @param arduinoBoyLogo render arduino boy logo 
    * @param snakeLogo render snake logo
    * @param waitKeyPress render wait key press and wait key press
    */
        switch (gameEngine.state)
        {
        case arduinoBoyLogo:
        {
            game_start_screen_aka_logo();
            delay(3000);
            M5.Lcd.clearDisplay();
            gameEngine.state = snakeLogo; //change state to snake logo
            break;
        }
        case snakeLogo:
        {
            game_start_screen_snake_logo();
            gameEngine.state = waitKeyPress; //change state to wait key press
        }
        break;
        case waitKeyPress:
        {
            game_start_screen_wait_key_press(4, 22, 2, "press any key to start"); // render press any key to start
            if (gameEngine.input != stable)
            {
                PRINT_NEW_LINE(" Change");
                M5.Lcd.clear();
                gameEngine.mode = playGame; // change state to play game
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
        util_set_input();            // change input compare and last input
        if (!gameEngine.isDrawSnake) //check snake is draw in game start
        {
            snake_first_draw();                               //draw snake once
            gameEngine.isDrawSnake = !gameEngine.isDrawSnake; //change is draw to true
        }

        if (!food.isDraw) //check food is draw for draw once
        {
            game_draw_food();
            food.isDraw = !food.isDraw; //change is draw to true
        }

        if (!wall.isDraw) //check wall is draw for draw once
        {
            game_draw_wall();
            wall.isDraw = !wall.isDraw; //change wall is draw to true
        }

        if (!gameEngine.isDrawBorder) // check border drawed
        {
            M5.Lcd.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
        }

        snake_tail_delete();                                 //Find Tail Position And Delete This Position
        snake_tail_update_direction(gameEngine.input);       //Update Every Snake Body Direction Update Last To Head
        snake_body_update_position();                        //Update Snake Body By Current Direction
        util_draw_square(head->col, head->row, SNAKE_COLOR); //Draw Snake Head
        snake_hit_food_action();                             //Check Hit Food And Take Action
        game_draw_score();                                   //Draw Score
        snake_hit_wall_action();                             //Check Hit Wall And Take Action
        snake_hit_border_action();                           //Check Hit Border And Take Action

        delay(150);
        break;
    }
    case gameOver:
    {
        /**
    * start screen render switch case change by state in game engine
    * @param score render score screen
    * @param waitKeyPress render wait key press and wait key press
    */
        switch (gameEngine.state)
        {
        case score:
        {
            game_game_over_screen();         //Draw Game Over Screen
            gameEngine.state = waitKeyPress; //Change state to wait key press
            break;
        }
        case waitKeyPress:
        {
            game_start_screen_wait_key_press(4, 22, 2, "press any key to exit");
            if (gameEngine.input != stable)
            {
                M5.Lcd.clear();
                game_init_param(); //Reset Game Prameter to Default
                snake_body_init(); //Reset Snanke Body
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

/**
 * board_init
 * ? Initialize board array
*/
void board_init()
{
    /**
   * Allocation Dynamic Array
   * ========================== Column < Allocation First >  
   * ++++++++++++++++++++++++++ Row 
   * --------------------------> Allocation Row Start From First Collumn
  */
    int column = gameEngine.column;
    int row = gameEngine.row;
    board = (Board_t **)malloc(column * sizeof(Board_t *)); // Allocation board to Double Pointer
    for (int col = 0; col < column; ++col)
        board[col] = (Board_t *)malloc(row * sizeof(Board_t)); // Allocation Each Board(column) to board pointer
    return;
}

/**
 * board_pre_cal
 * ? Pre Calculate Center And Top Left
*/
void board_pre_cal()
{
    int row = gameEngine.row;
    int column = gameEngine.column;
    board_double_for(column, row, board_pre_cal_func); // Use Double For Function And Pass Pre Calculte Function
    return;
}

/**
 * board_print_cell
 * ? Print All Grid Box In Grid System
*/
void board_print_cell(int column, int row)
{
    char buffer[1024];
    int topLeft_X = BOARD_GET_TOP_LEFT_X(column, row);
    int topLeft_Y = BOARD_GET_TOP_LEFT_Y(column, row);
    sprintf(buffer, "Index [ %2d, %2d ] TopLeft X : %3d  Y : %3d", row, column, topLeft_X, topLeft_Y);
    PRINT_NEW_LINE(buffer);
    return;
}

/**
 * board_double_for
 * ? Pass Row Column And Function To Double Loop
 * ? This Funciton Will Func cell in double loop and pass parmeter to function
 * @param column total column 
 * @param row total row 
 * @param cell pass functino that have arg is (int column, int row) 
 *             <void (*cell)(int column, int row)>  Format of Function can pass
 */
void board_double_for(int column, int row, void (*cell)(int column, int row))
{
    for (int col = 0; col < column; col++)
    {
        for (int row_f = 0; row_f < row; row_f++)
        {
            cell(col, row_f); // Call Function and Pass Arg
        }
    }
    return;
}

/**
 * board_print
 * ? Board Print Function
 */
void board_print()
{
    int column = gameEngine.column;
    int row = gameEngine.row;
    board_double_for(column, row, board_print_cell);
    return;
}

/**
 * board_pre_cal_func
 * ? Board Pre Calculate Function
 */
void board_pre_cal_func(int column, int row)
{
    int radius = gameEngine.radius;                                                  //Get Radius From Game Engine
    (*(board + column) + row)->x = CALCULATE_CENTER(column, GRID_RECT_SIZE, radius); // Set Center X In Board Strut
    (*(board + column) + row)->y = CALCULATE_CENTER(row, GRID_RECT_SIZE, radius);    //Set Center Y In Board Strut
    int center_X = (*(board + column) + row)->x;
    int center_Y = (*(board + column) + row)->y;
    (*(board + column) + row)->top_left_x = CALCULATE_TOP_LEFT(center_X, radius); //Calculate Top Left X
    (*(board + column) + row)->top_left_y = CALCULATE_TOP_LEFT(center_Y, radius); //Calculte Top Left Y
    return;
}

/**
 * snake_init 
 * ? Init Snake First Head 
 * @param new_row start row of snake head
 * @param new_column start column of snake head
 * @param new_direction start direction of snke head
*/
void snake_init(int new_row, int new_col, int new_direction)
{

    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node)); //Alloction New Node
    struct Node *last = head;

    new_node->row = new_row;
    new_node->col = new_col;
    new_node->direction = new_direction;
    new_node->next = NULL; //Set Next Node To Null

    if (head == NULL) //Check Node IS Null
    {
        new_node->prev = NULL;
        head = new_node;
        return;
    }
    while (last->next != NULL) //Travel Util Last Node
        last = last->next;
    last->next = new_node; // Set Last Node To New Node
    new_node->prev = last; // Set Prev Of New Node To Last Node

    return;
}

/**
 * snake_init 
 * ? Add New Tail After Last Node
*/
void snake_add_tail()
{
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    struct Node *last = head;
    struct Node *last_tail = snake_get_last_node();

    new_node->row = last_tail->row;
    new_node->col = last_tail->col;
    switch (last_tail->direction) //Update Position Of New Node
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

Node *snake_get_last_node()
{
    struct Node *tail = head;

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
    /* add bottom right calculate here*/
    M5.Lcd.fillRect(topleft_X, topleft_Y, GRID_RECT_SIZE, GRID_RECT_SIZE, color);
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
}

void game_draw_wall()
{
    wall.col = random(2, gameEngine.column);
    wall.row = random(2, gameEngine.row);
    util_draw_rect(wall.col, wall.row, WALL_WIDTH, WALL_HEIGHT, WALL_COLOR);
}

void util_draw_rect(int column, int row, int width, int height, uint16_t color)
{
    int topleft_X = BOARD_GET_TOP_LEFT_X(column, row);
    int topleft_Y = BOARD_GET_TOP_LEFT_Y(column, row);
    /* add bottom right calculate here*/
    M5.Lcd.fillRect(topleft_X, topleft_Y, width, height, color);
}

void snake_tail_delete()
{
    struct Node *head_temp = head;
    struct Node *temp = snake_get_last_node();
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

    if (r1x + (r1w + -1) >= r2x && // r1 right edge past r2 left
        r1x <= r2x + (r2w - 1) &&  // r1 left edge past r2 right
        r1y + (r1h - 1) >= r2y &&  // r1 top edge past r2 bottom
        r1y <= r2y + (r2h - 1))
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
        gameEngine.state = score;
        M5.Lcd.clear();
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
        gameEngine.state = score;
        M5.Lcd.clear();
        gameEngine.mode = gameOver;
    }
}

void util_lcd_print(int column, int row, int size, String text, uint16_t color = WHITE)
{
    M5.Lcd.setTextColor(color);
    M5.Lcd.setCursor(BOARD_GET_TOP_LEFT_X(column, row), BOARD_GET_TOP_LEFT_Y(column, row));
    M5.Lcd.setTextSize(size);
    M5.Lcd.print(text);
}

void game_start_screen_aka_logo()
{
    util_lcd_print(8, 6, 2, "( also known as )", RED);
    util_lcd_print(13, 10, 20, "AKA", RED);
    util_lcd_print(8, 20, 3, "' gameboy '", WHITE);
}

void game_start_screen_snake_logo()
{
    M5.Lcd.clear();
    util_lcd_print(15, 6, 20, "s", GREEN);
    M5.Lcd.fillCircle(67 + 40 + 16, 92, 5, RED);
    util_lcd_print(20, 8, 2, "NAKE", WHITE);
    util_lcd_print(20, 10, 2, "GAME", WHITE);
}

void game_start_screen_wait_key_press(int column, int row, int size, String text)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= INTERVAL)
    {
        previousMillis = currentMillis;
        if (toggleState == LOW)
        {
            util_lcd_print(column, row, size, text, WHITE);
            toggleState = HIGH;
        }
        else
        {
            util_lcd_print(column, row, size, text, BACKGROUND_COLOR);
            toggleState = LOW;
        }
    }
}

void game_draw_score()
{
    M5.Lcd.setCursor(BOARD_GET_TOP_LEFT_X(35, 2), BOARD_GET_TOP_LEFT_Y(35, 2));
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.fillRect(BOARD_GET_TOP_LEFT_X(35, 2), BOARD_GET_TOP_LEFT_Y(35, 2), 10, 12, BLACK);
    M5.Lcd.print(String(gameEngine.score));
}

void game_game_over_screen()
{
    util_lcd_print(6, 5, 4, "GAME OVER", RED);
    util_lcd_print(11, 10, 2, "YOUR SCORE", ILI9341_DARKGREY);
    util_lcd_print(15, 14, 6, String(gameEngine.score), WHITE);
}

void game_init_param()
{
    food.col = -1;
    food.isDraw = false;
    food.row = -1;
    gameEngine.column = SCREEN_WIDTH / GRID_RECT_SIZE;
    gameEngine.isDrawBorder = false;
    gameEngine.isDrawSnake = false;
    gameEngine.lastInput = up;
    gameEngine.mode = startScreen;
    gameEngine.radius = GRID_RECT_SIZE / 2;
    gameEngine.row = SCREEN_HEIGHT / GRID_RECT_SIZE;
    gameEngine.state = arduinoBoyLogo;
    wall.isDraw = false;
}

void snake_body_init()
{
    head = NULL;
    snake_init(gameEngine.row / 2, gameEngine.column / 2, up);
    snake_add_tail();
    snake_add_tail();
}