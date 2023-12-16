
#include "../src/tiny_regex.h"
#include "string.h"
#include "stdio.h"

int main(void)
{
    char buf[] = "(1(2(3)?4(5(\\+(7(\\d)\\w([^b-e])|a([1-6avc])c)d(e)f)\\s+g)h)i)*";
    //char buf[] = "(-(-)())?";
    printf("start\r\n");

    char temp[20] = { 0 };
    tr_res_t test = tregex_match_str("-abc-d!123!@#$0", 0, "-adc|!22|!\\W*", 0);
    memcpy(temp, test.Data, test.Size);
    printf("match : %s \r\n", temp);

    printf("end\r\n");
    return 0;
}
