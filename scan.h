#ifndef SCAN_H
#define SCAN_H

#define MAXTOKENLEN 40

extern char tokenString[MAXTOKENLEN+1];

TokenType get_token(void);

#endif
