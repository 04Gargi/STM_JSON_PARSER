#include "triallib.h"

#define json_limit 256
#define size 50

static inline int is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static inline int is_number(char c) {
	return c >= '0' && c <= '9';
}

void append_token(token *tokens, int *token_count, token_type type,
		const char *value) {
	if (*token_count >= size) {
		printf("Error: Exceeded maximum token count\r\n");
		exit(1);
	}

	tokens[*token_count].type = type;
	tokens[*token_count].value = malloc(strlen(value) + 1);
	if (tokens[*token_count].value == NULL) {
		printf("Error: Memory allocation failed for token value\r\n");
		exit(1);
	}
	strcpy(tokens[*token_count].value, value);
	(*token_count)++;
}

token* lexer(const char *input, int *token_count) {
	int pos = 0;
	*token_count = 0;
	static token tokens[size];

	if (strlen(input) > json_limit) {
		printf("Error: Input JSON exceeds maximum allowed size\r\n");
		exit(1);
	}

	while (input[pos] != '*' && input[pos] != '\0') {
		char ch = input[pos];

		if (ch == '{') {
			append_token(tokens, token_count, TOKEN_LEFT_BRACE, "{");
			pos++;
		} else if (ch == '}') {
			append_token(tokens, token_count, TOKEN_RIGHT_BRACE, "}");
			pos++;
		} else if (ch == '[') {
			append_token(tokens, token_count, TOKEN_LEFT_BRACKET, "[");
			pos++;
		} else if (ch == ']') {
			append_token(tokens, token_count, TOKEN_RIGHT_BRACKET, "]");
			pos++;
		} else if (ch == ':') {
			append_token(tokens, token_count, TOKEN_COLON, ":");
			pos++;
		} else if (ch == ',') {
			append_token(tokens, token_count, TOKEN_COMMA, ",");
			pos++;
		} else if (ch == '"') {
			int start = ++pos;
			while (input[pos] != '"' && input[pos] != '*' && input[pos] != '\0') {
				pos++;
			}
			int len = pos - start;
			char sval[len + 1];
			strncpy(sval, &input[start], len);
			sval[len] = '\0';
			append_token(tokens, token_count, TOKEN_STRING, sval);
			pos++;
		} else if (is_whitespace(ch)) {
			pos++;
		} else if (is_number(ch) || ch == '-') {
			int start = pos;
			while (is_number(input[pos]) || input[pos] == '.') {
				pos++;
			}
			int len = pos - start;
			char numval[len + 1];
			strncpy(numval, &input[start], len);
			numval[len] = '\0';
			append_token(tokens, token_count, TOKEN_NUMBER, numval);
		} else if (strncmp(&input[pos], "null", 4) == 0) {
			append_token(tokens, token_count, TOKEN_NULL, "null");
			pos += 4;
		} else {
			printf("Unknown character: %c\r\n", ch);
			exit(1);
		}
	}

	return tokens;
}

value parse_value(token tok) {
	value val = { 0 };
	switch (tok.type) {
	case TOKEN_STRING:
		val.type = VALUE_STRING;
		val.as_union.string = strdup(tok.value);
		break;
	case TOKEN_NUMBER:
		val.type = VALUE_INT;
		val.as_union.integer = atoi(tok.value);
		break;
	case TOKEN_NULL:
		val.type = VALUE_NULL;
		break;
	default:
		printf("Error: Unexpected token type %d\r\n", tok.type);
		exit(1);
	}
	return val;
}

void parse_object(value *obj, token *tokens, int *token_index) {
	token tok = tokens[*token_index];
	if (tok.type != TOKEN_LEFT_BRACE) {
		printf("Syntax error: Expected opening brace\r\n");
		exit(1);
	}
	(*token_index)++;

	while (tokens[*token_index].type != TOKEN_RIGHT_BRACE) {
		token key_token = tokens[*token_index];
//		printf("%s",key_token.type);//new line
		if (key_token.type != TOKEN_STRING) {
			printf("Syntax error: Expected string key\r\n");
			exit(1);
		}

		obj->key = strdup(key_token.value);
		(*token_index)++;

		token colon_token = tokens[*token_index];
		if (colon_token.type != TOKEN_COLON) {
			printf("Syntax error: Expected colon\r\n");
			exit(1);
		}
		(*token_index)++;

		token value_token = tokens[*token_index];
		value parsed_value = parse_value(value_token);

		obj->type = parsed_value.type;
		switch (parsed_value.type) {
		case VALUE_STRING:
			obj->as_union.string = strdup(parsed_value.as_union.string);
			free(parsed_value.as_union.string);
			break;
		case VALUE_INT:
			obj->as_union.integer = parsed_value.as_union.integer;
			break;
		case VALUE_NULL:
			obj->as_union.null = 0;
			break;
		}

		(*token_index)++;

		token next_token = tokens[*token_index];
		if (next_token.type == TOKEN_COMMA) {
			(*token_index)++;
		} else if (next_token.type != TOKEN_RIGHT_BRACE) {
			printf("Syntax error: Expected comma or closing brace\r\n");
			exit(1);
		}
	}
	(*token_index)++;
}

value parse_json(const char *json_string) {
	int token_count = 0;
	token *tokens = lexer(json_string, &token_count);

	int token_index = 0;
	value result = { 0 };
	parse_object(&result, tokens, &token_index);

	for (int i = 0; i < token_count; i++) {
		free(tokens[i].value);
	}

	return result;
}
