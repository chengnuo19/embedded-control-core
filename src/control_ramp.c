#include "control_ramp.h"

static float abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

int ramp_init(RampGenerator *ramp, float initial_value, float rate_per_second)
{
    if (ramp == 0 || rate_per_second < 0.0f) {
        return 0;
    }

    ramp->current = initial_value;
    ramp->rate_per_second = rate_per_second;
    return 1;
}

void ramp_reset(RampGenerator *ramp, float value)
{
    if (ramp == 0) {
        return;
    }

    ramp->current = value;
}

float ramp_update(RampGenerator *ramp, float target, float dt)
{
    float max_step;

    if (ramp == 0) {
        return 0.0f;
    }
    if (dt <= 0.0f) {
        return ramp->current;
    }

    max_step = ramp->rate_per_second * dt;
    ramp->current = rate_limit(target, ramp->current, max_step);
    return ramp->current;
}

float rate_limit(float target, float current, float max_step)
{
    float delta;

    if (max_step < 0.0f) {
        max_step = -max_step;
    }

    delta = target - current;
    if (abs_float(delta) <= max_step) {
        return target;
    }

    return current + (delta > 0.0f ? max_step : -max_step);
}

float clamp_range(float value, float low, float high)
{
    if (low > high) {
        float tmp = low;
        low = high;
        high = tmp;
    }

    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}
