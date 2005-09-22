#ifndef _J_STR_LIB_H_
#define _J_STR_LIB_H_
#include "base.h"

// String function library.
// code by Jioh L. Jung (ziozzang@4wish.net)
// This code is under license "BSD"
char* jstrescape (char* pt);
char* jstrescapecpy (char* pt, const char* spt);
size_t jmemescapecpy (char* pt, const char* spt, int size);
char* trim(char* str);
char* checktrim(char* str, bool removeall=false);

#endif
