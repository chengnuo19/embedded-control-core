#ifndef CONTROL_TUNE_H
#define CONTROL_TUNE_H

typedef enum {
    TUNE_COMMAND_INVALID = 0,
    TUNE_COMMAND_SET = 1,
    TUNE_COMMAND_GET = 2,
    TUNE_COMMAND_STOP = 3,
    TUNE_COMMAND_ZERO = 4,
    TUNE_COMMAND_START = 5
} TuneCommandType;

typedef struct {
    const char *name;
    float *value;
    float min_value;
    float max_value;
} TuneParam;

typedef struct {
    TuneCommandType type;
    int param_index;
    float value;
} TuneResult;

int tune_parse_line(const char *line, TuneParam *params, int param_count, TuneResult *result);

#endif
