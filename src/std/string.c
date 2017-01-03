/**
 * String API
 */

int string__to_number(type_t **args, argc_t argc, type_t *ret, scope_t *scope)
{
    (void) argc;
    (void) scope;

    ret->type = tid_num;

    unsigned i = 0;
    TYPE_NUMBER number = 0;

    const char *string = args[0]->value.string;

    if (string[0] == '-') {
        ++i;
    }

    for (; i < strlen(string); ++i) {
        if (!isdigit(string[i])) {
            scope->error = gc_add(scope->gc, malloc(sizeof(huserr_t)));
            scope->error->name = "convertionErr";
            scope->error->msg = "string format is not correct";
            goto error;
        }

        number *= 10;
        number += string[i] - '0';
    }

    if (string[0] == '-') {
        number *= -1;
    }

    ret->value.num = number;

    return 0;

error:
    return -1;
}

