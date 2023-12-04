/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "charcode.h"
#include "error.h"
#include "reader.h"
#include "token.h"

extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];
int state;
int ln, cn;
char str[MAX_IDENT_LEN];
char str_number[100];
int i = 0;
int col;
char c;
int isIdentTooLong = 0;
int isNumberTooLong = 0;
int preColNo = 0;

void log_current_state() {
    char c_ = (char) currentChar;
    printf("[ln %d, col %d] charcode: '%c' - %d\tstate: %d\n", lineNo, colNo, c_, currentChar,
           state);
}

Token *getToken() {
    Token *token;
    switch (state) {
        case 0:
            if (currentChar == EOF)
                state = 1;
            else
                switch (charCodes[currentChar]) {
                    case CHAR_SPACE:
                        state = 2;
                        break;
                    case CHAR_LETTER:
                        ln = lineNo;
                        cn = colNo;
                        state = 3;
                        break;
                    case CHAR_DIGIT:
                        state = 7;
                        break;
                    case CHAR_PLUS:
                        state = 12;
                        break;
                    case CHAR_MINUS:
                        state = 10;
                        break;
                    case CHAR_TIMES:
                        state = 11;
                        break;
                    case CHAR_SLASH:
                        state = 12;
                        break;
                    case CHAR_LT:
                        state = 13;
                        break;
                    case CHAR_GT:
                        state = 16;
                        break;
                    case CHAR_EQ:
                        state = 19;
                        break;
                    case CHAR_EXCLAIMATION:
                        state = 20;
                        break;
                    case CHAR_COMMA:
                        state = 23;
                        break;
                    case CHAR_PERIOD:
                        state = 24;
                        break;
                    case CHAR_SEMICOLON:
                        state = 27;
                        break;
                    case CHAR_COLON:
                        state = 28;
                        break;
                    case CHAR_SINGLEQUOTE:
                        state = 31;
                        break;
                    case CHAR_LPAR:
                        state = 35;
                        break;
                    case CHAR_RPAR:
                        state = 42;
                        break;
                    default:
                        state = 43;
                }
            return getToken();
        case 1:
            return makeToken(TK_EOF, lineNo, colNo);
        case 2:
            do {
                readChar();
            } while (charCodes[currentChar] == CHAR_SPACE && currentChar != EOF);
            state = 0;
            return getToken();
        case 3:
            // TODO Recognize Identifiers and keywords
            str[i] = currentChar;
            i++;
            readChar();
            while (currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER ||
                                          charCodes[currentChar] == CHAR_DIGIT)) {
                if (i < MAX_IDENT_LEN) {
                    str[i] = currentChar;
                    i++;
                } else {
                    isIdentTooLong = 1;
                }
                readChar();
            }
            state = 4;
            return getToken();
        case 4:
            Token *t = makeToken(TK_NONE, lineNo, colNo);
            t->tokenType = checkKeyword(str);
            state = t->tokenType == TK_NONE ? 5 : 6;
            return getToken();
        case 5:
            col = colNo - i;
            i = 0;
            if (isIdentTooLong || strlen(str) > MAX_IDENT_LEN || strlen(str) == 0) {
                error(ERR_IDENTTOOLONG, lineNo, col);
                isIdentTooLong = 0;
                state = 0;
                memset(str, 0, sizeof(str));
                return getToken();
            } else {
                Token *res = makeToken(TK_IDENT, lineNo, col);
                strcpy(res->string, str);
                memset(str, 0, sizeof(str));
                return res;
            }
        case 6:
            TokenType tp = checkKeyword(str);
            col = colNo - i;
            i = 0;
            memset(str, 0, sizeof(str));
            int line = lineNo;
            if (currentChar == 10) {
                line = lineNo - 1;
                col = 1;
            }
            return makeToken(tp, line, col);
        case 7:
            int isZeroOnLeft = 0;
            if (currentChar != '0') {
                str_number[i] = currentChar;
                isZeroOnLeft = 1;
                i++;
            }
            readChar();
            while (charCodes[currentChar] == CHAR_DIGIT) {
                if (currentChar != '0' || isZeroOnLeft) {
                    isZeroOnLeft = 1;
                    str_number[i] = currentChar;
                    i++;
                }
                readChar();
            }
            state = 8;
            return getToken();
        case 8:
            // convert str_number to int
            long res = 0;
            for (int j = 0; j < i; j++) {
                res = res * 10 + (str_number[j] - '0');
            }
            if (res > INT_MAX) {
                error(ERR_NUMBERTOOLONG, lineNo, colNo - i);
                state = 0;
                memset(str_number, 0, sizeof(str_number));
                i = 0;
                return getToken();
            } else {
                Token *tk_number = makeToken(TK_NUMBER, lineNo, colNo - i);
                tk_number->value = (int) res;
                strcpy(tk_number->string, str_number);
                memset(str_number, 0, sizeof(str_number));
                i = 0;
                return tk_number;
            }
        case 9:
            readChar();
            return makeToken(SB_PLUS, lineNo, colNo - 1);
        case 10:
            readChar();
            return makeToken(SB_MINUS, lineNo, colNo);
        case 11:
            readChar();
            return makeToken(SB_TIMES, lineNo, colNo);
        case 12:
            readChar();
            return makeToken(SB_SLASH, lineNo, colNo);
        case 13:
            readChar();
            if (charCodes[currentChar] == CHAR_EQ)
                state = 14;
            else
                state = 15;
            return getToken();
        case 14:
            readChar();
            return makeToken(SB_LE, lineNo, colNo - 1);
        case 15:
            return makeToken(SB_LT, lineNo, colNo - 1);
        case 16:
            readChar();
            state = charCodes[currentChar] == CHAR_EQ ? 17 : 18;
            return getToken();
        case 17:
            readChar();
            return makeToken(SB_GE, lineNo, colNo - 1);
        case 18:
            return makeToken(SB_GT, lineNo, colNo - 1);
        case 19:
            readChar();
            return makeToken(SB_EQ, lineNo, colNo);
        case 20:
            readChar();
            state = charCodes[currentChar] == CHAR_EQ ? 21 : 22;
            return getToken();
        case 21:
            readChar();
            return makeToken(SB_NEQ, lineNo, colNo - 1);
        case 22:
            token = makeToken(TK_NONE, lineNo, colNo - 1);
            error(ERR_INVALIDSYMBOL, token->lineNo, token->colNo);
            return token;
        case 23:
            readChar();
            return makeToken(SB_COMMA, lineNo, colNo);
        case 24:
            readChar();
            state = charCodes[currentChar] == CHAR_RPAR ? 25 : 26;
            return getToken();
        case 25:
            readChar();
            return makeToken(SB_RSEL, lineNo, colNo - 1);
        case 26:
            return makeToken(SB_PERIOD, lineNo, colNo - 1);
        case 27:
            int col_ = colNo;
            readChar();
            if (currentChar == 10) {   // newline, should be previous line
                return makeToken(SB_SEMICOLON, lineNo - 1, col_);
            } else {
                return makeToken(SB_SEMICOLON, lineNo, colNo - 1);
            }
        case 28:
            readChar();
            state = charCodes[currentChar] == CHAR_EQ ? 29 : 30;
            return getToken();
        case 29:
            readChar();
            return makeToken(SB_ASSIGN, lineNo, colNo - 1);
        case 30:
            return makeToken(SB_COLON, lineNo, colNo - 1);
        case 31:
            preColNo = colNo;
            readChar();
            if (currentChar == EOF)
                state = 34;
            else if (isprint(currentChar))
                state = 32;
            else
                state = 34;
            return getToken();
        case 32:
            c = currentChar;
            preColNo = colNo;
            readChar();
            if (charCodes[currentChar] == CHAR_SINGLEQUOTE)
                state = 33;
            else
                state = 34;
            return getToken();
        case 33:
            token = makeToken(TK_CHAR, lineNo, colNo - 1);
            token->string[0] = c;
            token->string[1] = '\0';
            readChar();
            return token;
        case 34:
            if (currentChar == 10) {
                error(ERR_INVALIDCHARCONSTANT, lineNo - 1, preColNo - 1);
            } else {
                error(ERR_INVALIDCHARCONSTANT, lineNo, colNo - 2);
            }
            state = 0;
            return getToken();
        case 35:   // tokens begin with lpar, skip comments
            ln = lineNo;
            cn = colNo;
            readChar();
            if (currentChar == EOF)
                state = 41;
            else
                switch (charCodes[currentChar]) {
                    case CHAR_PERIOD:
                        state = 36;
                        break;
                    case CHAR_TIMES:
                        state = 37;
                        break;
                    default:
                        state = 41;
                }
            return getToken();

        case 36:
            readChar();
            return makeToken(SB_LSEL, lineNo, colNo);
        case 37:
            readChar();
            while (charCodes[currentChar] != CHAR_TIMES && currentChar != EOF) {
                state = 37;
                readChar();
            }
            if (currentChar == EOF)
                state = 40;
            else
                state = 38;

            return getToken();
        case 38:
            readChar();
            while (charCodes[currentChar] == CHAR_TIMES) {
                state = 38;
                readChar();
            }
            if (currentChar == EOF) {
                state = 40;
            } else if (charCodes[currentChar] == CHAR_RPAR) {
                state = 39;
            } else {
                state = 37;
            }
            return getToken();
        case 39:
            readChar();
            state = 0;
            return getToken();
        case 40:
            error(ERR_ENDOFCOMMENT, lineNo, colNo);
            state = 0;
            return getToken();
        case 41:
            return makeToken(SB_LPAR, ln, cn);
        case 42:
            readChar();
            return makeToken(SB_RPAR, lineNo, colNo);
        case 43:
            token = makeToken(TK_NONE, lineNo, colNo);
            error(ERR_INVALIDSYMBOL, lineNo, colNo);
            readChar();
            return token;
    }
}

void printToken(Token *token) {
    printf("%d-%d:", token->lineNo, token->colNo);

    switch (token->tokenType) {
        case TK_NONE:
            printf("TK_NONE\n");
            break;
        case TK_IDENT:
            printf("TK_IDENT(%s)\n", token->string);
            break;
        case TK_NUMBER:
            printf("TK_NUMBER(%s)\n", token->string);
            break;
        case TK_CHAR:
            printf("TK_CHAR(\'%s\')\n", token->string);
            break;
        case TK_EOF:
            printf("TK_EOF\n");
            break;

        case KW_PROGRAM:
            printf("KW_PROGRAM\n");
            break;
        case KW_CONST:
            printf("KW_CONST\n");
            break;
        case KW_TYPE:
            printf("KW_TYPE\n");
            break;
        case KW_VAR:
            printf("KW_VAR\n");
            break;
        case KW_INTEGER:
            printf("KW_INTEGER\n");
            break;
        case KW_CHAR:
            printf("KW_CHAR\n");
            break;
        case KW_ARRAY:
            printf("KW_ARRAY\n");
            break;
        case KW_OF:
            printf("KW_OF\n");
            break;
        case KW_FUNCTION:
            printf("KW_FUNCTION\n");
            break;
        case KW_PROCEDURE:
            printf("KW_PROCEDURE\n");
            break;
        case KW_BEGIN:
            printf("KW_BEGIN\n");
            break;
        case KW_END:
            printf("KW_END\n");
            break;
        case KW_CALL:
            printf("KW_CALL\n");
            break;
        case KW_IF:
            printf("KW_IF\n");
            break;
        case KW_THEN:
            printf("KW_THEN\n");
            break;
        case KW_ELSE:
            printf("KW_ELSE\n");
            break;
        case KW_WHILE:
            printf("KW_WHILE\n");
            break;
        case KW_DO:
            printf("KW_DO\n");
            break;
        case KW_FOR:
            printf("KW_FOR\n");
            break;
        case KW_TO:
            printf("KW_TO\n");
            break;
        case SB_SEMICOLON:
            printf("SB_SEMICOLON\n");
            break;
        case SB_COLON:
            printf("SB_COLON\n");
            break;
        case SB_PERIOD:
            printf("SB_PERIOD\n");
            break;
        case SB_COMMA:
            printf("SB_COMMA\n");
            break;
        case SB_ASSIGN:
            printf("SB_ASSIGN\n");
            break;
        case SB_EQ:
            printf("SB_EQ\n");
            break;
        case SB_NEQ:
            printf("SB_NEQ\n");
            break;
        case SB_LT:
            printf("SB_LT\n");
            break;
        case SB_LE:
            printf("SB_LE\n");
            break;
        case SB_GT:
            printf("SB_GT\n");
            break;
        case SB_GE:
            printf("SB_GE\n");
            break;
        case SB_PLUS:
            printf("SB_PLUS\n");
            break;
        case SB_MINUS:
            printf("SB_MINUS\n");
            break;
        case SB_TIMES:
            printf("SB_TIMES\n");
            break;
        case SB_SLASH:
            printf("SB_SLASH\n");
            break;
        case SB_LPAR:
            printf("SB_LPAR\n");
            break;
        case SB_RPAR:
            printf("SB_RPAR\n");
            break;
        case SB_LSEL:
            printf("SB_LSEL\n");
            break;
        case SB_RSEL:
            printf("SB_RSEL\n");
            break;
    }
}

int scan(char *fileName) {
    Token *token;
    if (openInputStream(fileName) == IO_ERROR)
        return IO_ERROR;

    token = getToken();

    while (token->tokenType != TK_EOF) {
        printToken(token);
        free(token);
        state = 0;
        token = getToken();
    }

    free(token);
    closeInputStream();
    return IO_SUCCESS;
}

int main() {
    if (scan("example1.kpl") == IO_ERROR) {
        printf("Can\'t read input file!\n");
    }
    return 0;
}
