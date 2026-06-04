#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "fonts.h"
#include "rand.h"
#include "games.h"

//216x384

int selection = 0;
int selected[4] = {};
int selectionCount = 0;

int guesses[4][4] = {};
int guessCount = 0;

char answers[4][64] = {};

const char *categoryNames[4] = {};
const char *board[16] = {};
int boardWordCtg[16] = {};

bool solvedCategories[4] = {false, false, false, false};
int mostRecentSolved = -1;
bool lastIsOneAway = false;

bool revealAnswer = false;
bool canReset = false;

void sort(int *arr, int n) {
  int temp;

  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
          temp = arr[j];
          arr[j] = arr[j + 1];
          arr[j + 1] = temp;
      }
    }
  }
}

bool matchingArr(int *arr1, int n1, int *arr2, int n2) {
  if (n1 != n2) return false;

  sort(arr1, n1);
  sort(arr2, n2);

  int t = 0;
  for (int i = 0; i < n1; i++) {
    if (arr1[i] == arr2[i]) t++;
  }
  
  if (t == n1) return true;
  else return false;
}

void fillArea(unsigned x,unsigned y,unsigned w,unsigned h,unsigned short col){
    unsigned short*s=(unsigned short*)GetVRAMAddress();
    s+=(y*384)+x;
    while(h--){
        unsigned w2=w;
        while(w2--)
            *s++=col;
        s+=384-w;
    }
}

void roundedRect(unsigned x,unsigned y,unsigned w,unsigned h,unsigned short col,unsigned short bcol) {
  fillArea(x, y, w, h, col);

  unsigned short* vram = (unsigned short*)GetVRAMAddress();

  w -= 1;
  h -= 1;

  //Top left
  vram[(y * 384) + x] = bcol;
  vram[(y * 384) + x+1] = bcol;
  vram[((y+1) * 384) + x] = bcol;

  //Top right
  vram[(y * 384) + x+w] = bcol;
  vram[(y * 384) + x+w-1] = bcol;
  vram[((y+1) * 384) + x+w] = bcol;

  //Bottom left
  vram[((y+h) * 384) + x] = bcol;
  vram[((y+h) * 384) + x+1] = bcol;
  vram[((y+h-1) * 384) + x] = bcol;

  //Bottom right
  vram[((y+h) * 384) + x+w] = bcol;
  vram[((y+h) * 384) + x+w-1] = bcol;
  vram[((y+h-1) * 384) + x+w] = bcol;
}

void drawChar(int x, int y, char c, unsigned short color) {
    unsigned short* vram = (unsigned short*)GetVRAMAddress();
    
    for (int row = 0; row < 8; row++) {
        unsigned char row_bits = font8x8_basic[(int)c][row];
        for (int col = 0; col < 8; col++) {
            // CHANGED: Scan from LSB to MSB (Right to Left)
            if (row_bits & (0x01 << col)) { 
                
                // Bounds check
                if ((x + col) >= 0 && (x + col) < 384 && (y + row) >= 0 && (y + row) < 216) {
                    vram[(y + row) * 384 + (x + col)] = color;
                }
            }
        }
    }
}

void drawString(int x, int y, const char* str, unsigned short color) {
    while (*str) {
        drawChar(x, y, *str++, color);
        x += 8; // Move 8 pixels right for the next character
    }
}

void centerText(int x, int y, int w, int h, const char* str, unsigned short color) {
  int nx = x;
  int ny = y;
  int len = strlen(str);
  //nx
  //ny = (((len * 8) % w) == 0 ?  0 : ((len * 8) % w)/2) + y / 2;
  nx = x + (w / 2) - ((len * 8) / 2);
  ny = y + (h / 2) - 4;

  drawString(nx, ny, str, color);
}

void drawControls() {
  int x = 5, y = 150 - 24;
  PrintMiniMini(&x, &y, "Arrows", 0b00010010, TEXT_COLOR_BLACK, 0);
  PrintMiniMini(&x, &y, " to move cursor", 0b00000010, TEXT_COLOR_BLACK, 0);
  x = 5, y = 160 - 24;
  PrintMiniMini(&x, &y, "EXE", 0b00010010, TEXT_COLOR_BLACK, 0);
  PrintMiniMini(&x, &y, " to select/deselect", 0b00000010, TEXT_COLOR_BLACK, 0);
  x = 5, y = 170 - 24;
  PrintMiniMini(&x, &y, "F2", 0b00010010, TEXT_COLOR_BLACK, 0);
  PrintMiniMini(&x, &y, " to deselect all", 0b00000010, TEXT_COLOR_BLACK, 0);
  x = 5, y = 180 - 24;
  PrintMiniMini(&x, &y, "F5", 0b00010010, TEXT_COLOR_BLACK, 0);
  PrintMiniMini(&x, &y, " to submit answer", 0b00000010, TEXT_COLOR_BLACK, 0);
  x = 5, y = 190 - 24;
  PrintMiniMini(&x, &y, "AC", 0b00010010, TEXT_COLOR_BLACK, 0);
  PrintMiniMini(&x, &y, " to reset", 0b00000010, TEXT_COLOR_BLACK, 0);
  x = 5, y = 200 - 24;
  PrintMiniMini(&x, &y, "0", 0b00010010, TEXT_COLOR_BLACK, 0);
  PrintMiniMini(&x, &y, " to reveal an answer", 0b00000010, TEXT_COLOR_BLACK, 0);
}

void drawGrid() {
  int w = 90;
  int h = 30;

  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      int index = x+(y*4);
      const char *word = board[index];
      int nx = 5 + (5*x) + (w*x);
      int ny = 5 + (5*y) + (h*y);

      int cardColor = (color_t)0xf711;
      int textColor = (color_t)0x0000;

      int cardCatagory = boardWordCtg[index];

      for (int i = 0; i < selectionCount; i++) {
        if (selected[i] == index) {
          cardColor = (color_t)0xa514;
        }
      }

      if (solvedCategories[cardCatagory] == true) {
        switch(cardCatagory) {
          case 0:
            cardColor = (color_t)0x9e0b; //Green
            break;
          case 1:
            cardColor = (color_t)0xefe7; //Yellow
            break;
          case 2:
            cardColor = (color_t)0x953d; //Blue
            break;
          case 3:
            cardColor = (color_t)0xb377; //Purple
            break;
        }

        textColor = (color_t)0x0000;
      }

      if (selection == index) {
        if (cardColor == (color_t)0xa514) cardColor = (color_t)0x5acb;
        else if (cardColor == (color_t)0x9e0b) cardColor = (color_t)0x6d21;
        else if (cardColor == (color_t)0xefe7) cardColor = (color_t)0xee20;
        else if (cardColor == (color_t)0x953d) cardColor = (color_t)0x4b5c;
        else if (cardColor == (color_t)0xb377) cardColor = (color_t)0x9074;
        else cardColor = (color_t)0x0000;
        textColor = (color_t)0xffff;
      }

      roundedRect(nx, ny, w, h, cardColor, (color_t)0xffff);
      centerText(nx, ny, w, h, word, textColor);
    }
  }
}

void drawUI() {
  int deselectX = (64 * 2) - 32 - (4*8);
  int deselectY = 200;
  int deselectColor = selectionCount > 0 ? (color_t)0x0000 : (color_t)0xb596;

  roundedRect(deselectX - 6, deselectY - 6, 64 + (6*2), 8 + (6*2), deselectColor, (color_t)0xffff);
  roundedRect(deselectX - 5, deselectY - 5, 62 + (6*2), 6 + (6*2), (color_t)0xffff, deselectColor);
  drawString(deselectX, deselectY, "Deselect", deselectColor);

  int submitX = (64 * 5) - 32 - (3*8);
  int submitY = 200;

  int submitTextColor = (color_t)0xa554;
  int submitOutlineColor = (color_t)0xa554;
  int submitBoxColor = (color_t)0xffff;
  if (selectionCount == 4) {
    submitTextColor = (color_t)0xffff;
    submitOutlineColor = (color_t)0x0000;
    submitBoxColor = (color_t)0x0000;
  }

  roundedRect(submitX - 6, submitY - 6, 48 + (6*2), 8 + (6*2), submitOutlineColor, (color_t)0xffff);
  roundedRect(submitX - 5, submitY - 5, 46 + (6*2), 6 + (6*2), submitBoxColor, submitOutlineColor);
  drawString(submitX, submitY, "Submit", submitTextColor);

  char testString[32] = "Mistakes Remaining: ";
  int index = 20; 

  for (int i = 0; i < 4 - guessCount; i++) {
    testString[index] = '*';
    index++;
  }

  testString[index] = '\0';

  drawString(384 / 2 - (24*8 / 2), 170, testString, (color_t)0x0000);

  switch (mostRecentSolved) {
    case 0:
      fillArea(5, 149, 10, 10, (color_t)0x9e0b);
      drawString(18, 150, categoryNames[0], (color_t)0x0000);
      break;
    case 1:
      fillArea(5, 149, 10, 10, (color_t)0xefe7);
      drawString(18, 150, categoryNames[1], (color_t)0x0000);
      break;
    case 2:
      fillArea(5, 149, 10, 10, (color_t)0x953d);
      drawString(18, 150, categoryNames[2], (color_t)0x0000);
      break;
    case 3:
      fillArea(5, 149, 10, 10, (color_t)0xb377);
      drawString(18, 150, categoryNames[3], (color_t)0x0000);
      break;
  }
  
  if (lastIsOneAway == true) drawString(379 - 8*9, 150, "One Away!", (color_t)0x0000);
}

void drawDebug() {
  drawString(5, 150, "Selected IDs: ", (color_t)0x0000);
  
  int printX = 110;
  for (int i = 0; i < selectionCount; i++) {
    char numStr[4];
    
    // Convert array integer to a string format safely
    // e.g. index 5 becomes "5 "
    numStr[0] = (selected[i] / 10) + '0';
    numStr[1] = (selected[i] % 10) + '0';
    numStr[2] = ' '; // spacer
    numStr[3] = '\0';
    
    // Skip leading zero for cleaner printing if index < 10
    if (numStr[0] == '0') {
      drawString(printX, 150, &numStr[1], (color_t)0x0000);
      printX += 16;
    } else {
      drawString(printX, 150, numStr, (color_t)0x0000);

      printX += 24;
    }
  }
}

void drawResetScreen() {
  color_t* VRAM = (color_t*)GetVRAMAddress();  
  for(int j=0; j<LCD_HEIGHT_PX; j++) { 
    for(int i=0; i<LCD_WIDTH_PX;  i++) { 
      *(VRAM) = (color_t)((((int)(0x0000 & 0xf81f) * 18 + (int)(*VRAM & 0xf81f) * (32-18) + 0x8010) >> 5) & 0xf81f) | 
      (color_t)((((int)(0x0000 & 0x07e0) * 18 + (int)(*VRAM & 0x07e0) * (32-18) + 0x0400) >> 6) & 0x07e0);
      VRAM++;
    }  
  }

  DrawFrame((color_t)((((int)(0x0000 & 0xf81f) * 18 + (int)(0xffff & 0xf81f) * (32-18) + 0x8010) >> 5) & 0xf81f) | 
      			    (color_t)((((int)(0x0000 & 0x07e0) * 18 + (int)(0xffff & 0x07e0) * (32-18) + 0x0400) >> 6) & 0x07e0));

  fillArea(LCD_WIDTH_PX/2-150, LCD_HEIGHT_PX/2-35, 300, 80, 0x0020);

  if (canReset) {
    int c = 68, d = 72;
    PrintMini(&c, &d, "Press AC again to restart", 0x02, 0xffffffff, 0, 0, COLOR_WHITE, (color_t)0x1082, 1, 0);
    c = 118, d += 25;
    PrintMiniMini(&c, &d, "Press any key to cancel", 0b00000010, TEXT_COLOR_WHITE, 0);
  } else {
    int c = LCD_HEIGHT_PX / 2 - 29*4, d = 72;
    int midX = 0;
    PrintMini(&midX, &d, "Press 1-4 to reveal an answer", 0x02, 0xffffffff, 0, 0, COLOR_WHITE, (color_t)0x1082, 0, 0);
    midX = LCD_WIDTH_PX / 2 - midX / 2;

    PrintMini(&midX, &d, "Press 1-4 to reveal an answer", 0x02, 0xffffffff, 0, 0, COLOR_WHITE, (color_t)0x1082, 1, 0);
    c = 118, d += 25;
    PrintMiniMini(&c, &d, "Press any key to cancel", 0b00000010, TEXT_COLOR_WHITE, 0);
  }
  
}

void guess() {
  if (selectionCount != 4 || guessCount >= 4) return;

  int arr1[4];
  for (int i = 0; i < 4; i++) arr1[i] = selected[i];

  for (int i = 0; i < guessCount; i++) {
    int arr2[4];
    for (int j = 0; j < 4; j++) arr2[j] = guesses[i][j];

    if (matchingArr(arr1, 4, arr2, 4) == true) {
      return;
      break;
    }
  }

  int matchCategory = boardWordCtg[selected[0]];
  int correctAnswers = 4;

  for (int i = 0; i < 4; i++) {
    if (boardWordCtg[selected[i]] != matchCategory) {
      correctAnswers--;
    }
  }

  if (correctAnswers == 4) {
    solvedCategories[matchCategory] = true;
    mostRecentSolved = matchCategory;
    lastIsOneAway = false;
  } else {
    if (correctAnswers == 3) lastIsOneAway = true;
    else lastIsOneAway = false;
    for (int i = 0; i < 4; i++) guesses[guessCount][i] = selected[i];
    guessCount++;
  }

  memset(selected, -1, sizeof(selected));
  selectionCount = 0;
}

void newBoard() {
  ConnectionGame newGame = allGames[rand() % allGamesCount];

  int wordIndex = 0;
  for (int ctg = 0; ctg < 4; ctg++) {
    categoryNames[ctg] = newGame.categories[ctg].categoryName;
    for (int word = 0; word < 4; word++) {
      board[wordIndex] = newGame.categories[ctg].words[word];
      boardWordCtg[wordIndex] = ctg;
      wordIndex++;
    }
  }

  for (int i = 15; i > 0; i--) {
    int j = rand() % (i + 1);

    // Swap the words
    const char* tempWord = board[i];
    board[i] = board[j];
    board[j] = tempWord;

    // Swap the category tags along with them so they stay glued to their words
    int tempCat = boardWordCtg[i];
    boardWordCtg[i] = boardWordCtg[j];
    boardWordCtg[j] = tempCat;
  }
}

void reset() {
  canReset = false;

  memset(selected, -1, sizeof(selected));
  selectionCount = 0;
  for (int i = 0; i < guessCount; i++) {
    memset(guesses[i], -1, sizeof(selected));
  }
  guessCount = 0;

  for (int i = 0; i < 4; i++) {
    solvedCategories[i] = false;
  }

  mostRecentSolved = -1;
  lastIsOneAway = false;

  newBoard();
}

int main() {
  int key;

  // Clear VRAM
  Bdisp_AllClr_VRAM();
  //Enable colours
  Bdisp_EnableColor(1);
  //Stop top bar from appearing when keys are pressed
  EnableDisplayHeader(0, 0);

  //Set random seed
  srand(RTC_GetTicks());
  newBoard();
 
  drawGrid();
  drawControls();
  //drawUI();
  //drawDebug();
  

  // Add-ins should NOT exit by returning from main, but call GetKey in a loop
  // instead. This mirrors the behavior of included apps as you can exit with
  // the MENU key or power off
  while (1) {
    // GetKey also presents the contents of VRAM to the screen
    GetKey(&key);
    Bdisp_AllClr_VRAM();

    switch (key) {
      case KEY_CTRL_LEFT:
        selection--;
        break;
      case KEY_CTRL_RIGHT:
        selection++;
        break;
      case KEY_CTRL_UP:
        selection -= 4;
        break;
      case KEY_CTRL_DOWN:
        selection += 4;
        break;
      
      case KEY_CTRL_EXE: {
        int index = -1;

        for (int i = 0; i < selectionCount; i++) {
          if (selection == selected[i]) {
            index = i;
            break;
          }
        }

        if (index > -1) {
          volatile int* arr = selected; 
          for (int i = index; i < selectionCount - 1; i++) arr[i] = arr[i + 1];
          selectionCount--;
        } else if (selectionCount < 4) {
          selected[selectionCount] = selection;
          selectionCount++;
        }
        break;
      }

      case KEY_CTRL_F2:
        memset(selected, -1, sizeof(selected));
        selectionCount = 0;
        break;
      case KEY_CTRL_F5:
        guess();
        break;
      
      case KEY_CTRL_AC:
        if (canReset == true) reset();
        else canReset = true;
        break;

      case KEY_CHAR_0:
        revealAnswer = !revealAnswer;
        break;
      case KEY_CHAR_1:
        if (revealAnswer == true) {
          solvedCategories[0] = true;
          //solvedCategories[0] = solvedCategories[0] == true ? false : true;
          mostRecentSolved = 0;
        }
        break;
      case KEY_CHAR_2:
        if (revealAnswer == true) {
          solvedCategories[1] = true;
          //solvedCategories[1] = solvedCategories[1] == true ? false : true;
          mostRecentSolved = 1;
        }
        break;
      case KEY_CHAR_3:
        if (revealAnswer == true) {
          solvedCategories[2] = true;
          //solvedCategories[2] = solvedCategories[2] == true ? false : true;
          mostRecentSolved = 2;
        }
        break;
      case KEY_CHAR_4:
        if (revealAnswer == true) {
          solvedCategories[3] = true;
          //solvedCategories[3] = solvedCategories[3] == true ? false : true;
          mostRecentSolved = 3;
        }
        break;
    }

    if (key != KEY_CTRL_AC && canReset == true) canReset = false;
    if (key != KEY_CHAR_0) revealAnswer = false;

    if (selection < 0) selection += 16;
    else selection = selection % 16;

    drawGrid();
    drawUI();
    if (canReset == true || revealAnswer == true) drawResetScreen();
    else DrawFrame((color_t)0xffff);
    //drawDebug();
  }

  return 0;
}