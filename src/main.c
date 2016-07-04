#include <stdio.h>
#include "tokenizer.h"
#include "parser.h"
#include "parser_element.h"

int main(void)
{
    token_t **tokens;
    token_config_t token_config;

    tokenizer__generate_config(&token_config);
    tokens = tokenizer__string(&token_config, "1 * (1 * 2), (1 + (1 * 2)), 3");

    parser_element_node_t *head = parser__parse(tokens);
    parser_element__printall(head, 0);

    parser_element__cleanall(head);
    tokenizer__clean_tokens(tokens);
}
