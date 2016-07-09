
int data[8][8] = { \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}, \
  {0, 0, 0, 0, 0, 0, 0, 0}  \
};
int LED[4][3] = {
  {1, 1, 1},
  {1, 1, 0},
  {1, 0, 0},
  {0, 0, 0}
};

//pin numebrs
//Pin connected to ST_CP of 74HC595
int latchPin = 10 ;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;
int rotPin = A0;
int led1 = 19;
int led2 = 20;
int led3 = 21;
int button = 22;

//varaibles 
int life = 0x0F;
int ballX = 4;
int ballY = 6;
int ballVolX = 1;
int ballVolY = 1;
boolean ballOutX = false;
boolean ballOutY = false;
long currentTime = 0;
int boardPosition = 4;

//constants
const int ADJUST_ROT = 100;
const int ORIGIN_ROT = 100;
const int ballSpeed = 80;
const int MATRIXSIZE = 8;
const int BOARDSIZE = 3;

//function signatures
void shift(int myDataPin, int myClockPin, int data[8][8], int col);
void changePosition();
int clamp(int Max, int Min, int num);
void checkLED();
void ballPosition();

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(rotPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(button, INPUT);
  //just to show that the game is starting, flash though all the LEDs
  for (int i = 0; i < 4; i ++ ) {
    digitalWrite(led1, LED[i][0]);
    digitalWrite(led2, LED[i][1]);
    digitalWrite(led3, LED[i][2]);
    delay(500);
  }
}
void loop() {

  //update the speed of ball. ball spped is the time
  if (millis() - currentTime > ballSpeed)
    ballPosition();
  currentTime = millis();

  int rowBit = 0x80;
  for (int i = 0; i < MATRIXSIZE; i ++) {
    changePosition();
    checkLED();
    int colBit = 0x01;
    for (int j = 0; j < MATRIXSIZE; j ++) {
      delay(1);
      digitalWrite(latchPin, LOW);
      shift(dataPin, clockPin, data, j);
      shiftOut(dataPin, clockPin, LSBFIRST, ~colBit);
      digitalWrite(latchPin, HIGH);
      colBit = colBit << 1;
    }
    rowBit = rowBit >> 1;
  }
  //stop the game when life goes to zero
  if (life == 0) {
    int temp = LOW;
    while (temp == LOW) {
      temp = digitalRead(button);
      delay(100);
    }
    life = 0x0F;
  }

}
// this function reads the analog input from potenitalmeter.
void changePosition() {
  int temp = analogRead(rotPin);
  temp = clamp( ORIGIN_ROT + ADJUST_ROT, ORIGIN_ROT - ADJUST_ROT, temp);
  temp = map(temp, ORIGIN_ROT - ADJUST_ROT, ORIGIN_ROT + ADJUST_ROT, 0, 5);
  for (int i = boardPosition ; i < boardPosition + BOARDSIZE; i ++) {
    data[MATRIXSIZE - 1][i] = 0;
  }
  for (int i = temp; i < temp + BOARDSIZE; i ++) {
    data[MATRIXSIZE - 1][i] = 1;
  }
  boardPosition = temp;
}
//this function displays the life as LEDs
void checkLED() {
  digitalWrite(led1, life & 1 );
  digitalWrite(led2, life & 1 << 1 );
  digitalWrite(led3, life & 1 << 2 );
}
// just a helper function
int clamp(int Max, int Min, int num) {
  if (num > Max) {
    return Max;
  } else if (num < Min) {
    return Min;
  } else {
    return num;
  }
}

// modified from shiftout function so that it can dsplay an 2D array
void shift(int myDataPin, int myClockPin, int data[8][8], int col) {
  // This shifts 8 bits out MSB first,
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i = 0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights.
  for (i = 0; i < MATRIXSIZE; i++)  {
    digitalWrite(myClockPin, 0);
    if ( (data[i][col] << i) & (1 << i) ) {
      pinState = 1;
    }
    else {
      pinState = 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}
//edit the data array 
void ballPosition() {
  //clear out the last location of ball
  data[ballY][ballX] = 0;

  //if the ball is at the side of the board, make sure the ball's turing back
  if (ballX == 0 || ballX == MATRIXSIZE - 1 ) {
    ballOutX = true;
  } else {
    ballOutX = false;
  }
  if (ballY == 0 || ballY == MATRIXSIZE - 1 ) {
    ballOutY = true;
  } else {
    ballOutY = false;
  }
  //turn the ball's direction around
  if (ballOutX)ballVolX *= -1;
  if (ballOutY)ballVolY *= -1;
  //update the ball's location
  ballX += ballVolX;
  ballY += ballVolY;

  //if the ball hits the board, turn the ball around again.
  if (data[ballY][ballX] == 1) {
    ballY -= ballVolY;
    ballVolY *= -1;
    ballY += ballVolY;
  }
  data[ballY][ballX] = 1;
  //if the ball fall -- reach the end of the board, life decrement
  if (ballY == MATRIXSIZE - 1 ) {
    life = life >> 1;
  }
}
