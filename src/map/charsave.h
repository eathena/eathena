#ifndef _CHARSAVE_H_
#define _CHARSAVE_H_

struct mmo_charstatus *charsave_loadchar(int charid);
int charsave_savechar(int charid, struct mmo_charstatus *c);

#endif
