/*
 * strlib
 * string library for Eve programming language
 * Read LICENCE for licencing informations
 */

#include <stdlib.h>
#include <stdio.h>

int length(char* str)
{
  int i=0;
  if(str==NULL)
    return -1;
  for(;str[i] != '\0'; i++);
  return i;
}

char * cpystr(char * str)
{
  char * tmpstr = (char*)malloc(sizeof(char));
  int i = 0;
  for(; str[i] != '\0'; i++)
  {
    tmpstr = (char*)realloc(tmpstr, (i+1)*sizeof(char));
    tmpstr[i] = str[i];
  }
  
  tmpstr[i] = '\0';
  return tmpstr;
}

char * addstr(char * str1, char * str2)
{
  char * tmpstr = (char*)malloc(sizeof(char));
  int i = 0, j=0;
  for(; str1[i] != '\0'; i++)
  {
    tmpstr = (char*)realloc(tmpstr, (i+1)*sizeof(char));
    tmpstr[i] = str1[i];
  }

  for(; str2[j] != '\0'; j++)
  {
    tmpstr = (char*)realloc(tmpstr, (i+j+1)*sizeof(char));
    tmpstr[i+j] = str2[j];
  }
  
  tmpstr[i+j] = '\0';
  return tmpstr;
}  

char * repstr (char *str, int count) {
    if (count == 0) return NULL;
    char *ret = malloc (length (str) * count + count);
    if (ret == NULL) return NULL;
    strcpy (ret, str);
    while (--count > 0)
        strcat (ret, str);
    return ret;
}
