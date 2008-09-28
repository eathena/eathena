#include "../common/cbasetypes.h" // uint8
#include "../common/mmo.h" // struct fame_list

enum fame_type { FAME_SMITH = 1, FAME_CHEMIST = 2, FAME_TAEKWON = 3 };

void char_read_fame_list(void);
bool fame_list_update(enum fame_type type, int charid, int fame);
bool get_fame_list(enum fame_type type, struct fame_list** list, int* size);
int fame_list_tobuf(uint8* buf, enum fame_type type);
bool fame_config_read(const char* key, const char* value);