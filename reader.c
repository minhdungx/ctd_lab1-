/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include "reader.h"

#include <stdio.h>

FILE* inputStream;
int lineNo, colNo;
int currentChar;

int readChar(void) {
    currentChar = getc(inputStream);
    colNo++;
    if (currentChar == '\n') {
        lineNo++;
        colNo = 0;
    }
    return currentChar;
}

int openInputStream(char* filename) {
    inputStream = fopen(filename, "rt");
    if (inputStream == NULL) {
        printf("Khong thay");
        return IO_ERROR;
    } else
        printf("Read file %s successfully! 🌱\n", filename);

    lineNo = 1;
    colNo = 0;
    readChar();
    return IO_SUCCESS;
}

void closeInputStream() { fclose(inputStream); }
