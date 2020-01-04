#include <stdio.h>
#include <string.h>
#include "picoshparser.h"

int main(int argc, char **argv)
{
    psr_parse_context_t ctx = {};
    int64_t urgency = 1;
    int incremental = 0;

    while (1) {
        if (!psr_parse_dictionary(&ctx, argv[1], strlen(argv[1])))
            goto Fail;
        if (ctx.dict_name_len == 0)
            break;
        if (ctx.dict_name_len == 1) {
            if (ctx.dict_name[0] == 'u') {
                if (!psr_parse_item_int(&ctx, &urgency))
                    goto Fail;
                if (!(0 <= urgency && urgency <= 7))
                    goto Fail;
            } else if (ctx.dict_name[0] == 'i') {
                if (!psr_parse_item_boolean(&ctx, &incremental))
                    goto Fail;
            }
        }
    }

    printf("u=%d,i=%d\n", (int)urgency, (int)incremental);
    return 0;
Fail:
    printf("parse failure\n");
    return 1;
}
