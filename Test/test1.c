
#include "../src/tiny_regex.h"
#include "string.h"

int main(void)
{
    char buf[] = "(1(2(3)?4(5(\\+(7(\\d)\\w([^b-e])|a([1-6avc])c)d(e)f)\\s+g)h)i)*";
    //char buf[] = "(-(-)())?";
    printf("start\r\n");
    
    tregex_complie(buf, strlen(buf));

    printf("end\r\n");
    return 0;
}