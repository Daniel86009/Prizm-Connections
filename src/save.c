#include <fxcg/file.h>
#include <string.h>
#include "save.h"

unsigned char filename[] = "\\\\fls0\\connections.sav";

static void narrow_to_wide(unsigned short *dest, const char *src) {
  while (*src) *dest++ = (unsigned short)(unsigned char)*src++;
  *dest = 0;
}

#define FILE_SIZE (DATA_SIZE * sizeof(unsigned int))

//Total, Won, Lost, Perfect, Current Streak, Highest Streak, Green, Yellow, Blue, Purple
unsigned int saveData[DATA_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int saveLoad() {
    unsigned short wFilename[64];
    narrow_to_wide(wFilename, (const char *)filename);

    int file = Bfile_OpenFile_OS(wFilename, 1, 0);

    if (file >= 0) {
        Bfile_ReadFile_OS(file, saveData, FILE_SIZE, 0);
        Bfile_CloseFile_OS(file);
    }

    return file;
}

int saveWrite() {
    unsigned short wFilename[64];
    narrow_to_wide(wFilename, (const char *)filename);

    int file = Bfile_OpenFile_OS(wFilename, 2, 0);

    if (file >= 0) {
        Bfile_WriteFile_OS(file, saveData, FILE_SIZE);
        Bfile_CloseFile_OS(file);
    }

    return file;
}

int saveInit() {
    unsigned short wFilename[64];
    narrow_to_wide(wFilename, (const char *)filename);

    unsigned int size = FILE_SIZE;

    int result = Bfile_OpenFile_OS(wFilename, 1, 0);

    if (result < 0) {
        Bfile_CreateEntry_OS(wFilename, CREATEMODE_FILE, &size);

        int file = Bfile_OpenFile_OS(wFilename, 2, 0);

        if (file >= 0) {
            Bfile_WriteFile_OS(file, saveData, FILE_SIZE);
            Bfile_CloseFile_OS(file);
        }
    } else {
        Bfile_CloseFile_OS(result);
    }

    return result;
}