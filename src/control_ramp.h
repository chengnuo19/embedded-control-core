#ifndef CONTROL_RAMP_H
#define CONTROL_RAMP_H

typedef struct {
    float current;
    float rate_per_second;
} RampGenerator;

int ramp_init(RampGenerator *ramp, float initial_value, float rate_per_second);
void ramp_reset(RampGenerator *ramp, float value);
float ramp_update(RampGenerator *ramp, float target, float dt);
float rate_limit(float target, float current, float max_step);
float clamp_range(float value, float low, float high);

#endif
