#include <math.h>
#include <stdio.h>

#include "control_filter.h"
#include "control_pid.h"
#include "control_ramp.h"
#include "control_tune.h"

static int failures = 0;

static void expect_close(const char *name, float actual, float expected, float tol)
{
    if (fabsf(actual - expected) > tol) {
        printf("FAIL %s: actual=%f expected=%f\n", name, actual, expected);
        failures++;
    }
}

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL %s: actual=%d expected=%d\n", name, actual, expected);
        failures++;
    }
}

static void test_pid_limits_output_and_integral(void)
{
    PidController pid;
    pid_init(&pid, 2.0f, 1.0f, 0.0f, 0.01f, -50.0f, 50.0f, -10.0f, 10.0f);

    float first = pid_update(&pid, 100.0f, 0.0f);
    expect_close("pid clamps first output", first, 50.0f, 0.001f);

    for (int i = 0; i < 500; ++i) {
        (void)pid_update(&pid, 100.0f, 0.0f);
    }

    expect_close("pid clamps integral high", pid_integral(&pid), 10.0f, 0.001f);
    expect_close("pid reset clears integral", (pid_reset(&pid), pid_integral(&pid)), 0.0f, 0.001f);
}

static void test_pid_derivative_uses_measurement_to_avoid_setpoint_kick(void)
{
    PidController pid;
    pid_init(&pid, 0.0f, 0.0f, 1.0f, 0.01f, -1000.0f, 1000.0f, -1000.0f, 1000.0f);

    float first = pid_update(&pid, 10.0f, 0.0f);
    float second = pid_update(&pid, 20.0f, 0.0f);

    expect_close("pid first derivative output", first, 0.0f, 0.001f);
    expect_close("pid ignores pure setpoint step in derivative", second, 0.0f, 0.001f);
}

static void test_low_pass_filter_and_moving_average(void)
{
    LowPassFilter lpf;
    lpf_init(&lpf, 0.25f, 0.0f);
    expect_close("lpf first update", lpf_update(&lpf, 8.0f), 2.0f, 0.001f);
    expect_close("lpf second update", lpf_update(&lpf, 8.0f), 3.5f, 0.001f);

    MovingAverageFilter avg;
    float buffer[4] = {0};
    moving_average_init(&avg, buffer, 4);
    expect_close("avg one sample", moving_average_update(&avg, 4.0f), 4.0f, 0.001f);
    expect_close("avg two samples", moving_average_update(&avg, 8.0f), 6.0f, 0.001f);
    expect_close("avg full window", moving_average_update(&avg, 12.0f), 8.0f, 0.001f);
    expect_close("avg rolls window", moving_average_update(&avg, 16.0f), 10.0f, 0.001f);
    expect_close("avg drops oldest", moving_average_update(&avg, 20.0f), 14.0f, 0.001f);
}

static void test_ramp_and_rate_limiter_are_time_step_based(void)
{
    RampGenerator ramp;
    ramp_init(&ramp, 0.0f, 5.0f);

    expect_close("ramp rises by rate times dt", ramp_update(&ramp, 12.0f, 0.2f), 1.0f, 0.001f);
    expect_close("ramp continues rising", ramp_update(&ramp, 12.0f, 0.2f), 2.0f, 0.001f);
    expect_close("ramp does not overshoot target", ramp_update(&ramp, 2.4f, 1.0f), 2.4f, 0.001f);

    expect_close("rate limit clamps positive step", rate_limit(10.0f, 0.0f, 3.0f), 3.0f, 0.001f);
    expect_close("rate limit clamps negative step", rate_limit(-10.0f, 0.0f, 3.0f), -3.0f, 0.001f);
    expect_close("rate limit accepts small step", rate_limit(2.0f, 0.0f, 3.0f), 2.0f, 0.001f);
}

static void test_numeric_helpers_handle_invalid_inputs(void)
{
    PidController pid;
    expect_int("pid init rejects zero dt", pid_init(&pid, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, -1.0f, 1.0f), 0);

    LowPassFilter lpf;
    expect_int("lpf rejects alpha below range", lpf_init(&lpf, -0.1f, 0.0f), 0);
    expect_int("lpf rejects alpha above range", lpf_init(&lpf, 1.1f, 0.0f), 0);

    MovingAverageFilter avg;
    expect_int("moving average rejects empty window", moving_average_init(&avg, 0, 0), 0);

    RampGenerator ramp;
    expect_int("ramp rejects negative rate", ramp_init(&ramp, 0.0f, -1.0f), 0);
}

static void test_tune_parser_sets_named_parameter_in_range(void)
{
    float kp = 1.0f;
    float ki = 0.0f;
    TuneParam params[] = {
        {"KP", &kp, 0.0f, 20.0f},
        {"KI", &ki, 0.0f, 10.0f},
    };
    TuneResult result;

    expect_int("tune accepts SET command", tune_parse_line("SET KP 2.500", params, 2, &result), 1);
    expect_int("tune result type SET", result.type, TUNE_COMMAND_SET);
    expect_int("tune result index", result.param_index, 0);
    expect_close("tune writes parameter", kp, 2.5f, 0.001f);
    expect_close("tune leaves other parameter", ki, 0.0f, 0.001f);
}

static void test_tune_parser_rejects_unknown_or_out_of_range_parameter(void)
{
    float kp = 1.0f;
    TuneParam params[] = {
        {"KP", &kp, 0.0f, 20.0f},
    };
    TuneResult result;

    expect_int("tune rejects unknown parameter", tune_parse_line("SET KD 3.0", params, 1, &result), 0);
    expect_close("unknown parameter does not write", kp, 1.0f, 0.001f);
    expect_int("tune rejects out of range value", tune_parse_line("SET KP 21.0", params, 1, &result), 0);
    expect_close("out of range does not write", kp, 1.0f, 0.001f);
}

static void test_tune_parser_recognizes_control_commands(void)
{
    TuneResult result;

    expect_int("tune accepts STOP", tune_parse_line("STOP", 0, 0, &result), 1);
    expect_int("tune STOP type", result.type, TUNE_COMMAND_STOP);
    expect_int("tune accepts ZERO", tune_parse_line("ZERO", 0, 0, &result), 1);
    expect_int("tune ZERO type", result.type, TUNE_COMMAND_ZERO);
    expect_int("tune accepts START", tune_parse_line("START", 0, 0, &result), 1);
    expect_int("tune START type", result.type, TUNE_COMMAND_START);
}

int main(void)
{
    test_pid_limits_output_and_integral();
    test_pid_derivative_uses_measurement_to_avoid_setpoint_kick();
    test_low_pass_filter_and_moving_average();
    test_ramp_and_rate_limiter_are_time_step_based();
    test_numeric_helpers_handle_invalid_inputs();
    test_tune_parser_sets_named_parameter_in_range();
    test_tune_parser_rejects_unknown_or_out_of_range_parameter();
    test_tune_parser_recognizes_control_commands();

    if (failures != 0) {
        printf("%d test(s) failed\n", failures);
        return 1;
    }

    printf("All control core tests passed\n");
    return 0;
}
