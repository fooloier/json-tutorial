#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define ISWHITESPACE(ch)    ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}


static int lept_parse_literal(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':
            if (c->json[1] != 'r' || c->json[2] != 'u' || c->json[3] != 'e')
                return LEPT_PARSE_INVALID_VALUE;
            c->json += 4;
            v->type = LEPT_TRUE;
            break;
        case 'f':
            if (c->json[1] != 'a' || c->json[2] != 'l' || c->json [3] != 's' || c->json[4] != 'e')
                return LEPT_PARSE_INVALID_VALUE;
            c->json += 5;
            v->type = LEPT_FALSE;
            break;
        case 'n':
            if (c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l')
                return LEPT_PARSE_INVALID_VALUE;
            c->json += 4;
            v->type = LEPT_NULL;
            break;
        default:
            break;
    }
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end, *dot;
    /* \TODO validate number */
    if (c->json[0] != '-' && !ISDIGIT(c->json[0]))
        return LEPT_PARSE_INVALID_VALUE;
    if (c->json[0] == '0' && !(c->json[1] == 'e' || c->json[1] == 'E' || ISWHITESPACE(c->json[1]) || c->json[1] == '\0'))
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    if ((dot = strchr(c->json, '.')) && !ISDIGIT(*(dot + 1)))
        return LEPT_PARSE_INVALID_VALUE;
    if (fabs(v->n = strtod(c->json, &end)) == HUGE_VAL)
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':
        case 'f':
        case 'n':  return lept_parse_literal(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
