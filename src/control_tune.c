#include "control_tune.h"

#include <stdlib.h>

static int is_space_char(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static char upper_char(char ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return (char)(ch - 'a' + 'A');
    }
    return ch;
}

static const char *skip_spaces(const char *cursor)
{
    while (cursor != 0 && is_space_char(*cursor)) {
        cursor++;
    }
    return cursor;
}

static int read_token(const char **cursor, char *token, int token_size)
{
    int length = 0;
    const char *p;

    if (cursor == 0 || *cursor == 0 || token == 0 || token_size <= 1) {
        return 0;
    }

    p = skip_spaces(*cursor);
    if (*p == '\0') {
        return 0;
    }

    while (*p != '\0' && !is_space_char(*p)) {
        if (length < token_size - 1) {
            token[length++] = upper_char(*p);
        }
        p++;
    }

    token[length] = '\0';
    *cursor = p;
    return length > 0;
}

static int equals_text(const char *left, const char *right)
{
    int i = 0;

    if (left == 0 || right == 0) {
        return 0;
    }

    while (left[i] != '\0' && right[i] != '\0') {
        if (upper_char(left[i]) != upper_char(right[i])) {
            return 0;
        }
        i++;
    }

    return left[i] == '\0' && right[i] == '\0';
}

static int find_param(const char *name, TuneParam *params, int param_count)
{
    if (name == 0 || params == 0 || param_count <= 0) {
        return -1;
    }

    for (int i = 0; i < param_count; ++i) {
        if (equals_text(name, params[i].name)) {
            return i;
        }
    }

    return -1;
}

static void set_invalid(TuneResult *result)
{
    if (result == 0) {
        return;
    }

    result->type = TUNE_COMMAND_INVALID;
    result->param_index = -1;
    result->value = 0.0f;
}

int tune_parse_line(const char *line, TuneParam *params, int param_count, TuneResult *result)
{
    char command[12];
    char name[20];
    char value_text[24];
    const char *cursor = line;
    int param_index;
    char *end_ptr;
    float value;

    set_invalid(result);
    if (line == 0 || result == 0 || !read_token(&cursor, command, (int)sizeof(command))) {
        return 0;
    }

    if (equals_text(command, "STOP")) {
        result->type = TUNE_COMMAND_STOP;
        return 1;
    }
    if (equals_text(command, "ZERO")) {
        result->type = TUNE_COMMAND_ZERO;
        return 1;
    }
    if (equals_text(command, "START")) {
        result->type = TUNE_COMMAND_START;
        return 1;
    }

    if (equals_text(command, "GET")) {
        if (!read_token(&cursor, name, (int)sizeof(name))) {
            return 0;
        }
        param_index = find_param(name, params, param_count);
        if (param_index < 0) {
            return 0;
        }
        result->type = TUNE_COMMAND_GET;
        result->param_index = param_index;
        result->value = params[param_index].value != 0 ? *params[param_index].value : 0.0f;
        return 1;
    }

    if (!equals_text(command, "SET")) {
        return 0;
    }
    if (!read_token(&cursor, name, (int)sizeof(name)) || !read_token(&cursor, value_text, (int)sizeof(value_text))) {
        return 0;
    }

    param_index = find_param(name, params, param_count);
    if (param_index < 0 || params[param_index].value == 0) {
        return 0;
    }

    end_ptr = 0;
    value = strtof(value_text, &end_ptr);
    if (end_ptr == value_text || *end_ptr != '\0') {
        return 0;
    }
    if (value < params[param_index].min_value || value > params[param_index].max_value) {
        return 0;
    }

    *params[param_index].value = value;
    result->type = TUNE_COMMAND_SET;
    result->param_index = param_index;
    result->value = value;
    return 1;
}
