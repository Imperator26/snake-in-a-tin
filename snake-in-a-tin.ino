#include <PCD8544.h>

/*
  Use a 10 kOhm resistor for each connection unless you're using a 3.3V board.
  
  RST - Digital Pin 6
  CE - Digital Pin 7
  DC - Digital Pin 5
  DIN - Digital Pin 4
  CLK - Digital Pin 3
  VCC - +3.3V Pin
  LIGHT - 10uF capacitor to Ground Pin
  GND - Ground Pin
*/

static PCD8544 lcd;

// Screen dimensions:
// Width = 84 px
// Height = 48 px
static const byte LCD_WIDTH = 84;
static const byte LCD_HEIGHT = 48;

static const byte welcome[4][15] = {
  {0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xb0, 0xbc, 0x62, 0x2a, 0x22, 0x2a, 0x64, 0xc0, 0x40},
  {0xf8, 0x18, 0x14, 0x1f, 0x1b, 0x3a, 0x3e, 0x35, 0x34, 0x14, 0x14, 0x14, 0x18, 0xf8, 0x00},
  {0xff, 0xe6, 0x9c, 0x68, 0x68, 0x98, 0x68, 0x68, 0x98, 0x68, 0x68, 0x9c, 0xe6, 0xff, 0x00},
  {0x1f, 0x30, 0x61, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x61, 0x30, 0x1f, 0x00}
};

const int NORTH_BUTTON_PIN = 8;
const int SOUTH_BUTTON_PIN = 9;
const int EAST_BUTTON_PIN = 10;
const int WEST_BUTTON_PIN = 11;

static byte schema[LCD_WIDTH*LCD_HEIGHT/8] = {0x00};

unsigned long lastRefresh;

unsigned int score = 0;
//unsigned int high_score = 0;

unsigned int trail[2][31];
unsigned int snakeLength; // Can't go higher than 30
char headGo = 'N';

unsigned int berry[2];

bool northStatus = 0; 
bool southStatus = 0; 
bool eastStatus = 0; 
bool westStatus = 0; 

unsigned int i;

//unsigned int generateBerry(unsigned int trail[2][31], int snakeLength);

void setup() {
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);

  pinMode(NORTH_BUTTON_PIN, INPUT);
  pinMode(SOUTH_BUTTON_PIN, INPUT);
  pinMode(EAST_BUTTON_PIN, INPUT);
  pinMode(WEST_BUTTON_PIN, INPUT);
  
  // Print welcome screen
  lcd.setCursor(15,0);
  lcd.print("Tin Snake");
  /*for(i = 0; i < 4; i++) {
    lcd.setCursor(30, i+1);
    lcd.drawBitmap(welcome[i], 15, 1);
  }*/
  lcd.setCursor(0,5);
  lcd.print("by F.A.Corazza");
  delay(7000);//2000

  snakeLength = 2;
  
  trail[0][0] = random(22, 62);
  trail[1][0] = random(12, 36);
  trail[0][1] = trail[0][0]-1;
  trail[1][1] = trail[1][0];

  generateBerry(trail, snakeLength);
  
  lastRefresh = millis();
  lcd.clear();
}


void loop() {
  northStatus = digitalRead(NORTH_BUTTON_PIN);
  southStatus = digitalRead(SOUTH_BUTTON_PIN);
  eastStatus = digitalRead(EAST_BUTTON_PIN);
  westStatus = digitalRead(WEST_BUTTON_PIN);

  if (northStatus == HIGH) headGo = 'N';
  else if (southStatus == HIGH) headGo = 'S';
  else if (eastStatus == HIGH) headGo = 'E';
  else if (westStatus == HIGH) headGo = 'W';

  
  if(millis() - lastRefresh >= 1000 - 10*score) {
    lastRefresh = millis();
    updateScreen(headGo);
  }
}

void updateScreen(char headGo){
  // Check if the snake ate the berry. Yes, this snake is herbivorous
  if(trail[0][0] == berry[0] and trail[1][0] == berry[1]) {
    generateBerry(trail, snakeLength);
    score++;
    snakeLength++;
  }
  
  // Move snake
  for(i = snakeLength; i > 0; i--) {
    trail[0][i] = trail[0][i-1];
    trail[1][i] = trail[1][i-1];
  }
  
  // Move the head of the snake
  if(headGo == 'N') {
    trail[0][0] = trail[0][1];
    trail[1][0] = trail[1][1] - 1;
  } else if(headGo == 'E') {
    trail[0][0] = trail[0][1] + 1;
    trail[1][0] = trail[1][1];
  } else if(headGo == 'S') {
    trail[0][0] = trail[0][1];
    trail[1][0] = trail[1][1] + 1;
  } else if(headGo == 'W') {
    trail[0][0] = trail[0][1] - 1;
    trail[1][0] = trail[1][1];
  }

  // Check for legal move (offscreen, bit itself)
  if(trail[0][0] < 0 or trail[0][0] > 83 or trail[1][0] < 1 or trail[1][0] > 48) endGame();

  for(i = 1; i < snakeLength; i++) {
    if(trail[0][0] == trail[0][i] and trail[1][0] == trail[1][i]) {
      endGame();
      break;
    }
  }
  
  drawTrail(trail, LCD_WIDTH, LCD_HEIGHT, berry);
}

void drawTrail(unsigned int trail[2][31], unsigned int width, unsigned int height, unsigned int berry[2]) {
  unsigned int row = 0;
  unsigned int decimal = 0;

  // Initialize schema
  for(i = 0; i < width*height/8; i++) {
    schema[i] = 0;
  }

  // Generate trail
  for(i = 0; i < snakeLength; i++) {
    row = (int) (trail[1][i]-1) / 8;
    decimal = (int) (trail[1][i]-1) - row*8;
    schema[trail[0][i] + row*width] += 0.5 + pow(2, decimal);
  }

  // Generate berry
  row = (int) (berry[1]-1) / 8;
  decimal = (int) (berry[1]-1) - row*8;
  schema[berry[0] + row*width] += 0.5 + pow(2, decimal);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.drawBitmap(schema, width, height/8);
}

void generateBerry(unsigned int trail[2][31], unsigned int snakeLength) {
  berry[0] = random(0,84);
  berry[1] = random(0,48);
  
  for(i = 0; i < snakeLength; i++) {
    if(trail[0][i] == berry[0]) {
      berry[0] = random(0,84);
      i = -1;//
    }
  }

  for(i = 0; i < snakeLength; i++) {
    if(trail[1][i] == berry[0]) {
      berry[1] = random(0,48);
      i = -1;//
    }
  }
}

void endGame() {
  lcd.clear();
  
  lcd.setCursor(15,1);
  lcd.print("GAME OVER!");
  lcd.setCursor(40,2);
  lcd.print(score);

  delay(1000);
  
  lcd.setCursor(0,4);
  lcd.print("Press >");
  lcd.setCursor(0,5);
  lcd.print("to Play Again");

  // Reset game
  score = 0;
  snakeLength = 2;
  
  trail[0][0] = random(22, 62);
  trail[1][0] = random(12, 36);
  trail[0][1] = trail[0][0]-1;
  trail[1][1] = trail[1][0];

  generateBerry(trail, snakeLength);

  // Wait for response
  while(eastStatus == LOW) {
    eastStatus = digitalRead(EAST_BUTTON_PIN);
  }
  
  lastRefresh = millis();
  lcd.clear();
}
