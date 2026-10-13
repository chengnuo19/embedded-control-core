#ifndef CONTROL_FILTER_H
#define CONTROL_FILTER_H

typedef struct {
    float alpha;
    float state;
} LowPassFilter;

typedef struct {
    float *buffer;
    int window_size;
    int index;
    int count;
    float sum;
} MovingAverageFilter;

int lpf_init(LowPassFilter *filter, float alpha, float initial_value);
float lpf_update(LowPassFilter *filter, float input);
float lpf_value(const LowPassFilter *filter);

int moving_average_init(MovingAverageFilter *filter, float *buffer, int window_size);
float moving_average_update(MovingAverageFilter *filter, float input);
void moving_average_reset(MovingAverageFilter *filter);

#endif
