#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "query.h"
#include "dbg.h"
#include "memory.h"

int stack_push(char **stack, char c, size_t *size, ssize_t *last)
{
    if ((ssize_t) *size <= *last + 1) {
        *size += 2;
        *stack = realloc(*stack, sizeof(char) * (*size));
        check_mem(*stack);
    }

    *last += 1;
    (*stack)[*last] = c;

    return 0;

error:
    return -1;
}

int stack_pop(char **stack, ssize_t *last)
{
    if (*last < 0) {
        return -1;
    }

    *last -= 1;

    return (*stack)[*last + 1];
}

int get_query(int interactive, int *line, char **buffer, size_t *size, FILE *f)
{
    int ret = 0, is_comment = false;
    int c, prev = '\0', prev_saved = '\\';
    char *stack = NULL;
    size_t stack_size = 0, i, j;
    ssize_t stack_last = -1;
    int linestart_i = 0;

    for (i = 0; ; ) {
        if (interactive) {
            goto error;
        } else {
            if ((c = fgetc(f)) == EOF) {
                ret = EOF;
                break;
            }
        }

        if (c == '\r') continue;

        if (!is_comment) {
            if (*size <= i + 3) {
                *size += 128;
                *buffer = realloc(*buffer, sizeof(char) * (*size));
            }

            (*buffer)[i] = c;
            (*buffer)[i + 1] = '\0';

            if (c == '-' && prev == '-') {
                --i;
                (*buffer)[i] = '\0';
                --i;
                is_comment = true;
                prev = (i > 0) ? (*buffer)[i] : '\0';
            } else if (stack && stack_last < (ssize_t) stack_size && stack_last >= 0) {
                if (stack[stack_last] == '\'') {
                    if (c == '\'') {
                        stack_pop(&stack, &stack_last);
                    }
                } else {
                    goto not_stack;
                }
            } else {
                not_stack:

                switch (c) {
                    case '\'':
                        stack_push(&stack, c, &stack_size, &stack_last);
                        break;

                    case '(':
                        stack_push(&stack, c, &stack_size, &stack_last);
                        break;

                    case ')':
                        if (stack[stack_last] != '(') {
                            sentinel("syntaxErr at %d:%ld", *line, i - linestart_i);
                        }

                        stack_pop(&stack, &stack_last);
                        break;

                    case '{':
                        stack_push(&stack, c, &stack_size, &stack_last);
                        break;

                    case '}':
                        if (stack[stack_last] != '{') {
                            sentinel("syntaxErr at %d:%ld", *line, i - linestart_i);
                        }

                        stack_pop(&stack, &stack_last);
                        break;
                }
            }
        }

        if (c == '\n') {
            linestart_i = i;
            *line += 1;

            if (prev_saved == '\\') {
                *buffer = realloc(*buffer, sizeof(char) * (++(*size)));
                (*buffer)[i] = '\n';
                (*buffer)[i + 1] = '\0';
                if (prev == '\\') {
                    (*buffer)[i - 1] = '\n';
                    (*buffer)[i] = '\0';
                    --i;
                    
                    if (i >= 0) {
                        c = (*buffer)[i];
                    }
                }
            } else if (stack_last == -1) {
                if (is_comment) {
                    (*buffer)[i + 1] = '\n';
                    (*buffer)[i + 2] = '\0';
                }
                break;
            } else {
                (*buffer)[i] = '\n';
                (*buffer)[i + 1] = '\0';                
                is_comment = false;
            }
        }

        if (!is_comment) {
            prev = c;
        }

        prev_saved = c;

        if (!is_comment) ++i;
    }

    if (stack_last != -1) {
        sentinel("syntaxErr at %d:%ld", *line, i - linestart_i);
        goto error;
    }

    FREE(stack);

    return ret;

error:
    FREE(stack);

    return -1;
}
