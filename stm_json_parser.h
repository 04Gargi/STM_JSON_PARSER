#ifndef triallib_h
#define triallib_h

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

//enum to store list of constants
typedef enum {
	TOKEN_LEFT_BRACE,
	TOKEN_RIGHT_BRACE,
	TOKEN_LEFT_BRACKET,
	TOKEN_RIGHT_BRACKET,
	TOKEN_COMMA,
	TOKEN_COLON,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_NULL,
	TOKEN_TRUE,
	TOKEN_FALSE
} token_type;

typedef enum {
	VALUE_STRING, VALUE_INT, VALUE_NULL
} value_type;

typedef struct token {
	token_type type;
	char *value;
} token;

typedef struct {
	union {
		char *string;
		int integer;
		double number;
		int null;
	} as_union;
	char *key;
	value_type type;
} value;

void append_token(token *tokens, int *token_count, token_type type,
		const char *value);
token* lexer(const char *input, int *token_count);
void parse_object(value *obj, token *tokens, int *token_index);
value parse_value(token tok);
value parse_json(const char *json_string);

#endif
