#include <stdio.h>
#include <string.h>
#include "picoshparser.h"

int main(int argc, char **argv)
{
    psr_parse_context_t ctx = {};
    const char *key;
    size_t key_len;
    int64_t urgency = 1;
    int incremental = 0;

    psr_init_parse_context(&ctx, argv[1], strlen(argv[1]));
    while (psr_parse_dictionary(&ctx, &key, &key_len)) {
        if (key_len == 0)
            goto Fail;
        if (key_len == 1) {
            if (key[0] == 'u') {
                if (!psr_parse_int_part(&ctx, &urgency))
                    goto Fail;
                if (!(0 <= urgency && urgency <= 7))
                    goto Fail;
            } else if (key[0] == 'i') {
                if (!psr_parse_bool_part(&ctx, &incremental))
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
