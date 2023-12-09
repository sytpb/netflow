

#ifndef INT__H
#define INT__H

int WINAPI cjIniReadInt(char* cSection,char* cKey,int iDefault,char* cIniFn);
int WINAPI cjIniReadString(char* cSection,char* cKey,char* cDefault,char* cReturnString,int iSize,char* cIniFn);
void WINAPI cjIniWriteString(char* cSection,char* cKey,char* cVal,char* cIniFn);
void WINAPI cjIniWriteInt(char* cSection,char* cKey,int iVal,char* cIniFn);


#endif