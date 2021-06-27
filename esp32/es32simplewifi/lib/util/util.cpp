#include <util.h>

char * stringToCharArray(String str) {
    const int len = str.length();
    char *ret = (char*)malloc((len+1)*sizeof(char));
    str.toCharArray(ret, len);
    return ret;
}