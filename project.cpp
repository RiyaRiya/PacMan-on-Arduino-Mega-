#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

// standard U of A library settings, assuming Atmel Mega SPI pins

#define SD_CS 5
#define TFT_CS 6
#define TFT_DC 7
#define TFT_RST 8
#define JOY_SEL 9
#define JOY_VERT_ANALOG 0
#define JOY_HORIZ_ANALOG 1
#define TFT_WIDTH 128
#define TFT_HEIGHT 160
#define JOY_DEADZONE 64 // Only care about joystick movement if
// position is JOY_CENTRE +/- JOY_DEADZONE
#define JOY_CENTRE 512
#define MILLIS_PER_FRAME 50 

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

uint8_t g_joyX = TFT_WIDTH/2+2; // X-position of cursor in pixels offset by 2 to not start at maze position
uint8_t g_joyY = TFT_HEIGHT/2-8; // Y-position of cursor in pixels offset by 8 to not start at maze position
uint8_t g_cursorX = 0; // Drawn cursor position
uint8_t g_cursorY = 0;
uint8_t cursor_x [6] = {0,0,0,0,0,0}; //6 ny 6 pixel cursor
uint8_t cursor_y [6] = {0,0,0,0,0,0};
int update = 1;
const int ghost_speed= 0.00001;
const uint16_t radius = 6; // measure of dimensions of ghost and pacman
const uint16_t ORANGE = tft.Color565(0xff,0x80,0x00);
int score = 0; //to keep track of score
int playing = 1; // flag to check for collision between pacman and ghost
int state =0; // variable to change state between playing mode and game over mode
int reprint =1; // flag to avoid reprint of game over screen if hoystick hasnt been moved
int selection = 4; // Options "Yes" and "No" printed starting on the 4th (8 pixel tall) line on LCD
int old_selection= 4;
uint8_t high_score[20]; // because only 20 entries on LED screen
uint8_t score_index =0; //the latest number of entry


const int redghost_y = 56; // starting position const as ghost moves horizontally
int redghost_x = 0;
int red_direction = 0;
uint8_t redghostx [6] = {0,0,0,0,0,0};
uint8_t redghosty [6] = {0,0,0,0,0,0};

const int blueghost_y = 72; //starting position of the ghost
int blueghost_x = 0;
int blue_direction = 0;
uint8_t blueghostx [6] = {0,0,0,0,0,0};
uint8_t blueghosty [6] = {0,0,0,0,0,0};

const int magentaghost_x = 16; // starting position const as ghost moves vertically
int magentaghost_y = 0;
int magenta_direction = 0;
uint8_t magentaghostx [6] = {0,0,0,0,0,0};
uint8_t magentaghosty [6] = {0,0,0,0,0,0};

const int greenghost_x = 104; //starting position of the ghost
int greenghost_y = 0;
int green_direction = 0;
uint8_t greenghostx [6] = {0,0,0,0,0,0};
uint8_t greenghosty [6] = {0,0,0,0,0,0};

//to save memory, each cell entry draws a 8 by 8 rectange to create a maze pattern
const uint8_t maze [][16] = {{1,1,0,1,0,0,1,0,1,0,0,0,1,0,1,1},
{1,1,0,1,0,0,1,0,1,0,0,0,1,0,1,1},
{1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1},
{1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1},
{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
{0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1},
{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0},
{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0},
{0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0},
{0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,0},
{1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1},
{1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1},
{1,1,0,1,0,0,1,0,1,0,0,0,1,0,1,1},
{1,1,0,1,0,0,1,0,1,0,0,0,1,0,1,1}};

//to save memory, each cell entry draws a  food pixel on the op left corner of 8 by 8 block
uint8_t food [] [16] = {{0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0},
{0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0},
{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
{1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1},
{1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1},
{1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0},
{1,1,0,1,1,1,1,0,1,1,1,1,1,0,1,1},
{1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1},
{1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1},
{1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1},
{1,1,0,0,0,1,1,0,1,1,1,0,0,0,1,1},
{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
{0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0},
{0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0}};

uint8_t temp_food [20][16]; //to redraw the food without the food pixel that was eaten

void scanJoystick(); //Scans the joystick for any movement. Updates g_joyX, g_joyY.
void updateScreen_Pacman(); //Refreshes the display. Draws the cursor at (g_joyX,g_joyY).

void draw_Maze (){
  /*In the maze cell array, if the value of the cell is 1, and 8x8 white rectangle was drawn in the corresponding space of the on the LCD screen*/
  for(int i =0; i <16; ++i){
    for (int j=0; j <20; ++j){
      if (maze[j][i] == 1){
        tft.drawRect (i*8, j*8, 8,8,WHITE);
        tft.fillRect (i*8, j*8,8,8,WHITE);
      }
    }
  }


  /*In the food cell array, if the value of the cell is 1, an orange pixel was drawn in the corresponding spot on the LCD screen. A new cell array was created for scoring*/
  for(int i =0; i <16; ++i){
    for (int j=0; j <20; ++j){
      temp_food[j][i]= food[j][i];
      if (temp_food[j][i] == 1){
        tft.drawPixel (i*8,j*8, ORANGE);
      }
    }
  }
}

/*Redrawing the background over their earlier position and drawing ghosts in a new position as they move */
void Red_Ghost_Movement_Redraw(){
  tft.drawRect (redghost_x, redghost_y, radius, radius, BLACK);
  tft.fillRect (redghost_x, redghost_y, radius, radius, BLACK);
}

void Red_Ghost_Movement (){
  tft.drawRect (redghost_x, redghost_y, radius, radius, RED);
  tft.fillRect (redghost_x, redghost_y, radius, radius, RED);
  delay (ghost_speed);
}

void Blue_Ghost_Movement_Redraw (){
  tft.drawRect (blueghost_x, blueghost_y, radius, radius, BLACK);
  tft.fillRect (blueghost_x, blueghost_y, radius, radius, BLACK);
}

void Blue_Ghost_Movement (){
  tft.drawRect (blueghost_x, blueghost_y, radius, radius, BLUE);
  tft.fillRect (blueghost_x, blueghost_y, radius, radius, BLUE);
  delay (ghost_speed);
}

void Magenta_Ghost_Movement_Redraw () {
  tft.drawRect (magentaghost_x, magentaghost_y, radius, radius, BLACK);
  tft.fillRect (magentaghost_x, magentaghost_y, radius, radius, BLACK);
}

void Magenta_Ghost_Movement () {
  tft.drawRect(magentaghost_x,magentaghost_y,radius,radius, MAGENTA);
  tft.fillRect(magentaghost_x,magentaghost_y,radius,radius, MAGENTA);
  delay (ghost_speed);
}

void Green_Ghost_Movement_Redraw (){
  tft.drawRect (greenghost_x, greenghost_y, radius, radius, BLACK);
  tft.fillRect (greenghost_x, greenghost_y, radius, radius, BLACK);
}

void Green_Ghost_Movement (){
  tft.drawRect(greenghost_x,greenghost_y,radius,radius, GREEN);
  tft.fillRect(greenghost_x,greenghost_y,radius,radius, GREEN);
  delay (ghost_speed);
}


/*Collision check functions*/

int collision_checK_RED (){
  /*checks every pixel of the red ghost, and every pixel of the pacman to see if they overlap, if they do playing is set to 0 and the game is over */
  for (int i = 0; i < radius; ++i){
    int cursorxx = g_joyX + i;
    for (int j = 0; j < radius; ++j){
      int cursoryy = g_joyY + j;
      for (int k = 0; k < radius; ++k){
        int redghostxx = redghost_x + k;
        for (int l = 0; l < radius; ++l){
          int redghostyy = redghost_y + l;
          if (cursorxx == redghostxx && cursoryy == redghostyy){
            playing = 0;
            break;
          }
        }
      }
    }
  }
  return playing;
}

int collision_check_BLUE (){
  /*checks every pixel of the blue ghost, and every pixel of the pacman to see if they overlap, if they do the game is over */
  for (int i = 0; i < radius; ++i){
    int cursorxx = g_joyX + i;
    for (int j = 0; j < radius; ++j){
      int cursoryy = g_joyY + j;
      for (int k = 0; k < radius; ++k){
        int blueghostxx = blueghost_x + k;
        for (int l = 0; l < radius; ++l){
          int blueghostyy = blueghost_y + l;
          if (cursorxx == blueghostxx && cursoryy == blueghostyy){
            playing = 0;
            break;
          }
        }
      }
    }
  }
  return playing;
}

int collsion_check_MAGENTA (){
  /*checks every pixel of the magenta ghost, and every pixel of the pacman to see if they overlap, if they do the game is over */
  for (int i = 0; i < radius; ++i){
    int cursorxx = g_joyX + i;
    for (int j = 0; j < radius; ++j){
      int cursoryy = g_joyY + j;
      for (int k = 0; k < radius; ++k){
        int magtentaghostxx = magentaghost_x + k;
        for (int l = 0; l < radius; ++l){
          int magentaghostyy = magentaghost_y + l;
          if (cursorxx == magtentaghostxx && cursoryy == magentaghostyy){
            playing = 0;
            break;
          }
        }
      }
    }
  }
  return playing;
}

int collsion_check_GREEN (){
  /*checks every pixel of the green ghost, and every pixel of the pacman to see if they overlap, if they do the game is over */
  for (int i = 0; i < radius; ++i){
    int cursorxx = g_joyX + i;
    for (int j = 0; j < radius; ++j){
      int cursoryy = g_joyY + j;
      for (int k = 0; k < radius; ++k){
        int greenghostxx = greenghost_x + k;
        for (int l = 0; l < radius; ++l){
          int greenghostyy = greenghost_y + l;
          if (cursorxx == greenghostxx && cursoryy == greenghostyy){
            playing = 0;
            break;
          }
        }
      }
    }
  }
  return playing;
}


/*funstions to make ghosts move back and forth on the screen*/

void updateScreen_RedGhost(){
  if (redghost_x < (TFT_WIDTH-1)-radius && red_direction == 0){
    // the red ghost is moving to the right of the screen
    Red_Ghost_Movement_Redraw();
    redghost_x = constrain(redghost_x+1,0,(TFT_WIDTH-1)-radius);
    playing = collision_checK_RED ();
    Red_Ghost_Movement ();
    red_direction = 0;
  }
  else if (redghost_x == (TFT_WIDTH-1)-radius && red_direction == 0){
    //the red ghost has reached the right edge of the screen, and changes direction
    Red_Ghost_Movement_Redraw ();
    redghost_x = constrain(redghost_x-1,0,(TFT_WIDTH-1)-radius);
    playing = collision_checK_RED ();
    Red_Ghost_Movement ();
    red_direction = 1;
  }
  else if ( redghost_x !=0 && redghost_x < (TFT_WIDTH-1)-radius && red_direction == 1){
    //the red ghost is moving to the left on the screen
    Red_Ghost_Movement_Redraw();
    redghost_x = constrain(redghost_x-1,0,(TFT_WIDTH-1)-radius);
    playing = collision_checK_RED ();
    Red_Ghost_Movement ();
    red_direction = 1;
  }
  else if (redghost_x == 0 && red_direction == 1){
    //the red ghost has reached the left edge of the screen
    Red_Ghost_Movement_Redraw();
    redghost_x = constrain(redghost_x+1,0,(TFT_WIDTH-1)-radius);
    playing = collision_checK_RED ();
    Red_Ghost_Movement ();
    red_direction = 0;
  }
}

void updateScreen_BlueGhost (){
  if (blueghost_x < (TFT_WIDTH-1)-radius && blue_direction == 0){
    // the blue ghost is moving to the right of the screen
    Blue_Ghost_Movement_Redraw();
    blueghost_x = constrain(blueghost_x+1,0,(TFT_WIDTH-1)-radius);
    playing = collision_check_BLUE();
    Blue_Ghost_Movement ();
    blue_direction = 0;
  }
  else if (blueghost_x == (TFT_WIDTH-1)-radius && blue_direction == 0){
    //the blue ghost has reached the right edge of the screen, and changes direction
    Blue_Ghost_Movement_Redraw();
    blueghost_x = constrain(blueghost_x-1,0,(TFT_WIDTH-1)-radius);
    playing = collision_check_BLUE();
    Blue_Ghost_Movement ();
    blue_direction = 1;
  }
  else if (blueghost_x !=0 && blueghost_x < (TFT_WIDTH-1)-radius && blue_direction == 1){
    //the blue ghost is moving to the left on the screen
    Blue_Ghost_Movement_Redraw();
    blueghost_x = constrain(blueghost_x-1,0,(TFT_WIDTH-1)-radius);
    playing = collision_check_BLUE();
    Blue_Ghost_Movement ();
    blue_direction = 1;
  }
  else if (blueghost_x == 0 && blue_direction == 1){
    //the blue ghost has reached the left edge of the screen
    Blue_Ghost_Movement_Redraw();
    blueghost_x = constrain(blueghost_x+1,0,(TFT_WIDTH-1)-radius);
    playing = collision_check_BLUE ();
    Blue_Ghost_Movement ();
    blue_direction = 0;
  }
}

void updateScreen_MagentaGhost (){
  if (magentaghost_y < (TFT_HEIGHT-1)-radius && magenta_direction == 0){
    //the magenta ghost is moving down the screen
    Magenta_Ghost_Movement_Redraw ();
    magentaghost_y = constrain (magentaghost_y +1, 0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_MAGENTA ();
    Magenta_Ghost_Movement();
    magenta_direction = 0;
  }
  else if (magentaghost_y == (TFT_HEIGHT-1)-radius && magenta_direction == 0){
    //the magenta ghost has reached the bottom of the screen and is changing direction
    Magenta_Ghost_Movement_Redraw ();
    magentaghost_y = constrain (magentaghost_y-1, 0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_MAGENTA ();
    Magenta_Ghost_Movement ();
    magenta_direction = 1;
  }
  else if (magentaghost_y !=0 && magentaghost_y < (TFT_HEIGHT-1)-radius && magenta_direction == 1){
    //the magenta ghost is moving up the screen
    Magenta_Ghost_Movement_Redraw ();
    magentaghost_y = constrain (magentaghost_y-1,0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_MAGENTA ();
    Magenta_Ghost_Movement();
    magenta_direction = 1;
  }
  else if (magentaghost_y == 0 && magenta_direction == 1){
    //the magenta ghost has reached the top of the screen and is changing direction
    Magenta_Ghost_Movement_Redraw ();
    magentaghost_y = constrain (magentaghost_y +1, 0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_MAGENTA ();
    Magenta_Ghost_Movement ();
    magenta_direction = 0;
  }
}

void updateScreen_GreenGhost (){
  if (greenghost_y < (TFT_HEIGHT-1)-radius && green_direction == 0){
    //the green ghost is moving down the screen
    Green_Ghost_Movement_Redraw ();
    greenghost_y = constrain (greenghost_y +1, 0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_GREEN();
    Green_Ghost_Movement();
    green_direction = 0;
  }
  else if (greenghost_y == (TFT_HEIGHT-1)-radius && green_direction == 0){
    //the green ghost has reached the bottom of the screen and is changing direction
    Green_Ghost_Movement_Redraw ();
    greenghost_y = constrain (greenghost_y-1, 0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_GREEN ();
    Green_Ghost_Movement ();
    green_direction = 1;
  }
  else if (greenghost_y !=0 && greenghost_y < (TFT_HEIGHT-1)-radius && green_direction == 1){
    //the green  ghost is moving up the screen
    Green_Ghost_Movement_Redraw ();
    greenghost_y = constrain (greenghost_y-1,0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_GREEN ();
    Green_Ghost_Movement();
    green_direction = 1;
  }
  else if (greenghost_y == 0 && green_direction == 1){
    //the green ghost has reached the top of the screen and is changing direction
    Green_Ghost_Movement_Redraw ();
    greenghost_y = constrain (greenghost_y +1, 0, (TFT_HEIGHT-1)-radius);
    playing = collsion_check_GREEN ();
    Green_Ghost_Movement ();
    green_direction = 0;
  }
}

void update_screen_ghosts (){
  updateScreen_RedGhost ();
  updateScreen_BlueGhost ();
  updateScreen_MagentaGhost ();
  updateScreen_GreenGhost ();
}

/*High score table functions*/

/*Swap two scores of high_score array*/
void swap_rest(uint8_t *ptr_score1, uint8_t *ptr_score2) {
  uint8_t tmp = *ptr_score1;
  *ptr_score1 = *ptr_score2;
  *ptr_score2 = tmp;
}

/*Selection sort mechanism
In descending order at the moment*/
void ssort(uint8_t* high_score, int len) {
  for (int i = len-1; i >= 1; --i) {
    //find the min score
    int min_idx = 0;
    for (int j = 1; j <= i; ++j) {
      if(high_score[j] < high_score[min_idx]) {
        min_idx = j;
      }
    }
    // Swap it with the last element of the sub-array
    swap_rest(&high_score[i], &high_score[min_idx]);
  }
}

void update_highscore(){
  int len =20;
  int dont_update_score=0;
  int last_index = min(score_index,len);

  //to check if the score already exists on the table and dont update if it does (so no repetition)
  for(int i=0; i<last_index;i++){
    if(score/10 == high_score[i]) { //divide by 10 to store in the high score array so that less memory is used and can be done in uint8_t
      dont_update_score =1;
      break;
    }
  }

  if(dont_update_score!=1){
    if(score_index<len){
      high_score[score_index]=score/10; //assigning score to the highscore table
      //since we increment in 10 points, to increase storage efficiency and use 8 bit int to hold score
      ssort(high_score, len);
    }
    else if(score_index>=len && high_score[len-1]<score/10){ //if 20 scores already on list
      high_score[len-1]=score/10; //replacing the smallest score of the sorted high_score array with new score
      ssort(high_score,len);
    }
    score_index++;
  }
}

void print_highscore(){
  tft.fillScreen(tft.Color565(0x00, 0x00, 0x00));
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.print ("High scores - Top 19");
  tft.print("\n");
  if(score_index>=20){
    for(int i=0; i<20; i++){
      tft.print(high_score[i]*10);
      tft.print("\n");
    }
  }
  else{
    for(int i=0; i<score_index; i++){
      tft.print(high_score[i]*10);
      tft.print("\n");
    }
  }

  /* Press and replease the joystick when in the High score screen to switch back to game over screen*/
  while(digitalRead(JOY_SEL)==HIGH){
    // do nothing when joystick is not pressed
  };

  delay(1000); // to capture the delay when the user presses and releases the joystick
}


void setup(void)
{
  init();
  Serial.begin(9600);
  Serial.print ("hello!");
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735R chip, black tab
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS))
  {
    Serial.println("failed!");
    return;
  }
  Serial.println("OK!");
  tft.fillScreen(tft.Color565(0x00, 0x00, 0x00));
  draw_Maze ();
}

int main()
{
  init();
  setup();
  Serial.begin(9600);
  // Init TFT
  tft.initR(INITR_BLACKTAB);
  // Init joystick
  pinMode(JOY_SEL, INPUT);
  digitalWrite(JOY_SEL, HIGH); // enables pull-up resistor
  //select= digitalRead(JOY_SEL);
  Serial.println("initialized!");
  int prevTime = millis();
  while (true)
  {
    scanJoystick();

    // if in the playing state
    if(state == 0){
      update_screen_ghosts();
      if(update ==1) //to check if screen update is needed
      {
        updateScreen_Pacman();
      }
      // Want: delay until prevTime + 50
      int t = millis();
      if (t - prevTime < MILLIS_PER_FRAME)
      {
        delay(MILLIS_PER_FRAME - (t - prevTime));
      }
      prevTime = millis();
      if (playing == 0){
        state =1;
        reprint =1;
        old_selection=4;
        selection=4;
      }
    }

    //if switched to state 1 then print game over screen but dont reprint if joystick hasnt been moved
    if(state==1 && reprint ==1){
      tft.fillScreen(tft.Color565(0x00, 0x00, 0x00));
      tft.setCursor(0, 0);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

      tft.print ("GAME OVER");
      tft.print("\n");
      tft.print ("Score is ");
      tft.print (score);
      tft.print("\n");
      tft.print("\n");
      tft.print ("Play again?");
      tft.print("\n");

      tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
      tft.setCursor(0, 4*8);
      tft.print ("Yes");
      tft.print("\n");

      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.setCursor(0, 5*8);
      tft.print ("No, See High Score");
      tft.print("\n");

      reprint =0;// flag to not reprint if joystick hasnt been moved
    }
  }
  Serial.end();
  return 0;
}

// To enable the scrolling function in the Game over screen
void update_names() {
  // print old selection normally
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setCursor(0, old_selection*8);
  if(old_selection==4){
    tft.print("Yes");
    tft.print("\n");
  }
  else if(old_selection==5){
    tft.print("No, See High Score");
    tft.print("\n");
  }

  //highlight new selection
  tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
  tft.setCursor(0, selection*8);
  if(selection==4){
    tft.print("Yes");
    tft.print("\n");
  }
  else if(selection==5){
    tft.print("No, See High Score");
    tft.print("\n");
  }
}


void scanJoystick()
{
  int vert = analogRead(JOY_VERT_ANALOG);
  int horiz = analogRead(JOY_HORIZ_ANALOG);
  int select = digitalRead(JOY_SEL);

  //unpressed joystick in state 1
  if(state==1 && select == HIGH ){
    if (abs(vert - JOY_CENTRE) > JOY_DEADZONE){
      int delta = vert - JOY_CENTRE;
      old_selection = selection;
      if(delta>0){
        selection = constrain((selection+1),4,5); //to avoid wraparound of
        //selection and since options "yes" and "no" are printed on the 4th and 5th line on LCD
      }
      else {
        selection = constrain((selection-1),4,5);
      }

      update_names();
      select= digitalRead(JOY_SEL);
    }
  }

  //condition to go to state 0 or print highscore (options for replay and highscore)
  else if(state==1 && select == LOW ){
    //to update highscore everytime the game ends and the score is not 0
    if(score!=0){
      update_highscore();
    }
    score =0;
    if(selection == 5){ //Selection is "No, go to highscore"
    print_highscore();
    state=1;
    reprint=1; // so that the Game over screen is printed again
    selection=4; //so that selection is reset to 4 to give correct values when the game over screen is displayed again
    old_selection=4;
  }

  else if(selection ==4){ //Selection is "YES, play again"
  state=0;
  playing =1;
  select = digitalRead(JOY_SEL);
  tft.fillScreen(tft.Color565(0x00, 0x00, 0x00));
  draw_Maze ();

  //re-initialize to reset all values to what they should be at the start of the game
  g_joyX = TFT_WIDTH/2+2; // X-position of cursor in pixels
  g_joyY = TFT_HEIGHT/2-8; // Y-position of cursor in pixels
  g_cursorX=g_joyX;
  g_cursorY=g_joyY;
  tft.drawRect(g_cursorX, g_cursorY,radius, radius, YELLOW);
  tft.fillRect(g_cursorX, g_cursorY,radius, radius, YELLOW);
  redghost_x = 0;
  red_direction = 0;
  blueghost_x = 0;
  blue_direction = 0;
  magentaghost_y = 0;
  magenta_direction = 0;
  greenghost_y = 0;
  green_direction = 0;

  for (int i=0; i<6; i++){
    redghostx [i] = 0;
    redghosty [i] = 0;
    blueghostx [i] = 0;
    blueghosty [i] = 0;
    magentaghostx [i] = 0;
    magentaghosty [i] = 0;
    greenghostx [i] = 0;
    greenghosty [i] = 0;
  }
}
}

//if playing
else if(state == 0){
  // Update g_joyX
  if (abs(horiz - JOY_CENTRE) > JOY_DEADZONE)
  {
    int tmp_x = g_joyX;
    int delta = 1; //to move pixel by pixel
    if(horiz - JOY_CENTRE > 0){
      g_joyX = constrain(g_joyX + delta, 0, TFT_WIDTH-radius);
    }
    else if(horiz - JOY_CENTRE < 0){
      g_joyX = constrain(g_joyX - delta, 0, TFT_WIDTH-radius);
    }
    int i = constrain(abs((g_joyX + (radius)-1)/8),0,15);
    //the x-coordinate of the bottom row  of the pacman
    int j = constrain(abs ((g_joyY + (radius)-1)/8),0,19);
    //the y-coordinate of the most right column of the pacman
    int k = constrain(abs(g_joyX /8),0,15);
    //the x-coordinate of the top row  of the pacman
    int l = constrain(abs (g_joyY/8),0,19);
    //the y-coordinate of the most left column  of the pacman

    //checks if each corner of the pacman is not overlapping a wall
    if (maze [l][k] ==0 && maze [j][k]==0 && maze[j][i] == 0 && maze[l][i] == 0){
      update = 1;
      //checks if every pixel of the pacman is overlapping a food pixel
      // if so the score is increased by 10
      for (int t = 0; t < 16; ++t){ //since 16 8 by 8 columns on LCD
        for (int p = 0; p < 20; ++p){ //20 8 by 8 rows on LCD
          if (temp_food [p][t] == 1){
            int pixel_x = t*8;
            int pixel_y = p*8;
            for (int r = 0; r < radius; ++r){
              int check_x = g_joyX + r;
              for (int s = 0; s < radius; ++ s){
                int check_y = g_joyY + s;

                if (check_x == pixel_x && check_y == pixel_y){
                  score = score + 10;
                  temp_food [p][t] = 0;
                }
              }
            }
          }
          else {
            score = score;
          }
        }
      }
    }
    else {
      g_joyX = tmp_x;
      update = 0;
    }
  }

  // Update g_joyY
  if (abs(vert - JOY_CENTRE) > JOY_DEADZONE)
  {
    int tmp_y = g_joyY;
    int delta =1;
    // move the cursor the given number of pixels
    if(vert - JOY_CENTRE > 0){
      g_joyY = constrain(g_joyY + delta, 0, TFT_HEIGHT-radius);
    }
    else if(vert- JOY_CENTRE <0){
      g_joyY = constrain(g_joyY - delta, 0, TFT_HEIGHT-radius);
    }

    int i = constrain(abs((g_joyX + (radius)-1)/8),0,15);
    int j = constrain(abs ((g_joyY + (radius)-1)/8),0,19);
    int k = constrain(abs(g_joyX /8),0,15);
    int l = constrain(abs (g_joyY/8),0,19);

    if (maze[l][k] == 0 && maze [l][i] == 0 && maze[j][i] == 0 && maze [j][k] == 0){
      update = 1;
      for (int t = 0; t < 16; ++t){
        for (int p = 0; p < 20; ++p){
          if (temp_food [p][t] == 1){
            int pixel_x = t*8;
            int pixel_y = p*8;
            for (int r = 0; r < radius; ++r){
              int check_x = g_joyX + r;
              for (int s = 0; s < radius; ++s){
                int check_y = g_joyY + s;
                if (check_x == pixel_x && check_y == pixel_y){
                  score = score + 10;
                  temp_food [p][t] = 0;
                }
              }
            }
          }
          else {
            score = score;
          }
        }
      }
    }
    else {
      g_joyY = tmp_y;
      update = 0;
    }
  }
}
}

void updateScreen_Pacman()
{
  /*Redrawing only the pixels of the background that were being covered by the
  old pacman*/
  tft.drawRect(g_cursorX, g_cursorY, radius, radius, ST7735_BLACK);
  tft.fillRect(g_cursorX, g_cursorY, radius, radius, ST7735_BLACK);

  /*Constrains to limit the pacman movement to the area of the map*/
  g_cursorX = constrain(g_joyX,0,(TFT_WIDTH)-radius); // pacman is 6 pixels wide, g_joyx is top left pixel
  g_cursorY = constrain(g_joyY,0,(TFT_HEIGHT)-radius); //Upper and lower limit of constrains set same as above

  /*Draws a 6X6 pixel square as pacman*/
  tft.drawRect(g_cursorX, g_cursorY,radius, radius, YELLOW);
  tft.fillRect(g_cursorX, g_cursorY,radius, radius, YELLOW);

  update = 0;
}
