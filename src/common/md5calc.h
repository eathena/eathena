// $Id: md5calc.h,v 1.1.1.1 2004/09/10 17:26:54 MagicalTux Exp $
#ifndef _MD5CALC_H_
#define _MD5CALC_H_

#include "base.h"

void MD5_String(const char * str, char * output);
void MD5_String2binary(const char * str, char * output);

#endif
