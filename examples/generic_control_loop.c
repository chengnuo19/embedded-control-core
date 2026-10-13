#include <stdio.h>

#include "control_filter.h"
#include "control_pid.h"
#include "control_ramp.h"
#include "control_tune.h"

typedef struct {
    float kp;
    float ki;
    float kd;
    float target;
    float output_limit;
    float target_rate;
} ControlParams;

static ControlParams g_params = {
    1.2f,
    0.4f,
    0.02f,
    80.0f,
    100.0f,
    150.0f,
};

static TuneParam g_tune_params[] = {
    {"KP", &g_params.kp, 0.0f, 50.0f},
    {"KI", &g_params.ki, 0.0f, 20.0f},
    {"KD", &g_params.kd, 0.0f, 10.0f},
    {"TARGET", &g_params.target, -1000.0f, 1000.0f},
    {"LIMIT", &g_params.output_limit, 0.0f, 1000.0f},
    {"RATE", &g_params.target_rate, 0.0f, 5000.0f},
};

static float fake_sensor_read(float output)
{
    static float plant_value = 0.0f;
    plant_value += 0.04f * (output - plant_value);
    return plant_value;
}

static void fake_actuator_write(float output)
{
    (void)output;
}

static void apply_serial_line(const char *line, PidController *pid, RampGenerator *ramp)
{
    TuneResult result;

    if (!tune_parse_line(line, g_tune_params, (int)(sizeof(g_tune_params) / sizeof(g_tune_params[0])), &result)) {
        printf("CMD ERR: %s\n", line);
        return;
    }

    if (result.type == TUNE_COMMAND_SET) {
        pid_set_gains(pid, g_params.kp, g_params.ki, g_params.kd);
        ramp->rate_per_second = g_params.target_rate;
        printf("CMD OK: %s\n", line);
    } else if (result.type == TUNE_COMMAND_ZERO) {
        pid_reset(pid);
        ramp_reset(ramp, 0.0f);
        printf("ZERO OK\n");
    } else if (result.type == TUNE_COMMAND_STOP) {
        g_params.target = 0.0f;
        printf("STOP OK\n");
    }
}

int main(void)
{
    const float dt = 0.01f;
    PidController pid;
    LowPassFilter measurement_filter;
    RampGenerator target_ramp;
    float output = 0.0f;

    if (!pid_init(&pid, g_params.kp, g_params.ki, g_params.kd, dt,
                  -g_params.output_limit, g_params.output_limit, -30.0f, 30.0f)) {
        return 1;
    }
    if (!lpf_init(&measurement_filter, 0.25f, 0.0f)) {
        return 1;
    }
    if (!ramp_init(&target_ramp, 0.0f, g_params.target_rate)) {
        return 1;
    }

    apply_serial_line("SET KP 1.8", &pid, &target_ramp);
    apply_serial_line("SET TARGET 60", &pid, &target_ramp);

    for (int tick = 0; tick < 250; ++tick) {
        float raw_measurement = fake_sensor_read(output);
        float measurement = lpf_update(&measurement_filter, raw_measurement);
        float smooth_target = ramp_update(&target_ramp, g_params.target, dt);

        output = pid_update(&pid, smooth_target, measurement);
        output = clamp_range(output, -g_params.output_limit, g_params.output_limit);
        fake_actuator_write(output);

        if (tick % 25 == 0) {
            printf("t=%03d target=%7.2f measured=%7.2f output=%7.2f\n",
                   tick, smooth_target, measurement, output);
        }
    }

    return 0;
}
