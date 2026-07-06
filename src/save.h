#ifndef _SAVE
#define _SAVE

#define DATA_SIZE 10

extern unsigned int saveData[DATA_SIZE];

int saveLoad();
int saveWrite();
int saveInit();

#endif