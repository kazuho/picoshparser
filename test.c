#include <stdio.h>
#include <string.h>
#include "picoshparser.h"

int main(int argc, char **argv)
{
    psr_parse_context_t ctx;
    int64_t urgency = 1;
    int incremental = 0;

    psr_complex__init(&ctx, argv[1], strlen(argv[1]));
    do {
        size_t key_len;
        const char *key = psr_complex__next_key(&ctx, &key_len);
        if (key == NULL)
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
    } while (!psr_complex__complete(&ctx));

    printf("u=%d,i=%d\n", (int)urgency, (int)incremental);
    return 0;
Fail:
    printf("parse failure\n");
    return 1;
}
