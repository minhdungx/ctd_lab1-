#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum {
    ERR_ENDOFCOMMENT,          // ok
    ERR_IDENTTOOLONG,          // ok
    ERR_INVALIDCHARCONSTANT,   // ok
    ERR_INVALIDSYMBOL,         // ok
    ERR_NUMBERTOOLONG          // ok
} ErrorCode;

#define ERM_ENDOFCOMMENT        "End of comment expected!"
#define ERM_IDENTTOOLONG        "Identification too long!"
#define ERM_INVALIDCHARCONSTANT "Invalid const char!"
#define ERM_INVALIDSYMBOL       "Invalid symbol!"
#define ERM_NUMBERTOOLONG       "Value of integer number exceeds the range!"

void error(ErrorCode err, int lineNo, int colNo);

#endif
