#include "control_filter.h"

int lpf_init(LowPassFilter *filter, float alpha, float initial_value)
{
    if (filter == 0 || alpha < 0.0f || alpha > 1.0f) {
        return 0;
    }

    filter->alpha = alpha;
    filter->state = initial_value;
    return 1;
}

float lpf_update(LowPassFilter *filter, float input)
{
    if (filter == 0) {
        return 0.0f;
    }

    filter->state += filter->alpha * (input - filter->state);
    return filter->state;
}

float lpf_value(const LowPassFilter *filter)
{
    if (filter == 0) {
        return 0.0f;
    }

    return filter->state;
}

int moving_average_init(MovingAverageFilter *filter, float *buffer, int window_size)
{
    if (filter == 0 || buffer == 0 || window_size <= 0) {
        return 0;
    }

    filter->buffer = buffer;
    filter->window_size = window_size;
    filter->index = 0;
    filter->count = 0;
    filter->sum = 0.0f;

    for (int i = 0; i < window_size; ++i) {
        buffer[i] = 0.0f;
    }

    return 1;
}

float moving_average_update(MovingAverageFilter *filter, float input)
{
    if (filter == 0 || filter->buffer == 0 || filter->window_size <= 0) {
        return 0.0f;
    }

    if (filter->count < filter->window_size) {
        filter->count++;
    } else {
        filter->sum -= filter->buffer[filter->index];
    }

    filter->buffer[filter->index] = input;
    filter->sum += input;
    filter->index = (filter->index + 1) % filter->window_size;
    return filter->sum / (float)filter->count;
}

void moving_average_reset(MovingAverageFilter *filter)
{
    if (filter == 0 || filter->buffer == 0 || filter->window_size <= 0) {
        return;
    }

    filter->index = 0;
    filter->count = 0;
    filter->sum = 0.0f;

    for (int i = 0; i < filter->window_size; ++i) {
        filter->buffer[i] = 0.0f;
    }
}
