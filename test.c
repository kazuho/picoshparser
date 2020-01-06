#include <stdio.h>
#include <string.h>
#include "picotest.h"
#include "picoshparser.h"

static int parse_priority(int64_t *urgency, int *incremental, const char *field)
{
    psr_parse_context_t ctx;
    const char *key;
    size_t key_len;

    *urgency = 1;
    *incremental = 0;

    psr_complex__init(&ctx, field, strlen(field));
    while (psr_complex__next_key(&ctx, &key, &key_len)) {
        if (key == NULL)
            goto Fail;
        if (key_len == 1) {
            if (key[0] == 'u') {
                if (!psr_parse_int_part(&ctx, urgency))
                    goto Fail;
                if (!(0 <= *urgency && *urgency <= 7))
                    goto Fail;
            } else if (key[0] == 'i') {
                if (!psr_parse_bool_part(&ctx, incremental))
                    goto Fail;
            }
        }
    }

    return 1;
Fail:
    return 0;
}

static void test_priority(void)
{
    int64_t u;
    int i;

    ok(parse_priority(&u, &i, "u=4"));
    ok(u == 4);
    ok(!i);
    ok(parse_priority(&u, &i, "u=5,i=?1"));
    ok(u == 5);
    ok(i);
    ok(parse_priority(&u, &i, "i=?1,u=0"));
    ok(u == 0);
    ok(i);
    ok(parse_priority(&u, &i, "x=1,i=?1,b=2,u=0,z=3"));
    ok(u == 0);
    ok(i);
}

int main(int argc, char **argv)
{
    subtest("priority", test_priority);
    return done_testing();
}
