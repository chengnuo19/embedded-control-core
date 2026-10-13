#include "control_pid.h"

static float clamp_float(float value, float low, float high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

int pid_init(PidController *pid,
             float kp,
             float ki,
             float kd,
             float dt,
             float output_min,
             float output_max,
             float integral_min,
             float integral_max)
{
    if (pid == 0 || dt <= 0.0f || output_min > output_max || integral_min > integral_max) {
        return 0;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->dt = dt;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integral_min = integral_min;
    pid->integral_max = integral_max;
    pid_reset(pid);
    return 1;
}

void pid_set_gains(PidController *pid, float kp, float ki, float kd)
{
    if (pid == 0) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void pid_set_target_period(PidController *pid, float dt)
{
    if (pid == 0 || dt <= 0.0f) {
        return;
    }

    pid->dt = dt;
}

void pid_reset(PidController *pid)
{
    if (pid == 0) {
        return;
    }

    pid->integral = 0.0f;
    pid->previous_measurement = 0.0f;
    pid->has_previous_measurement = 0;
}

float pid_update(PidController *pid, float target, float measurement)
{
    float error;
    float derivative;
    float output;

    if (pid == 0) {
        return 0.0f;
    }

    error = target - measurement;
    pid->integral += pid->ki * error * pid->dt;
    pid->integral = clamp_float(pid->integral, pid->integral_min, pid->integral_max);

    derivative = 0.0f;
    if (pid->has_previous_measurement) {
        derivative = -(measurement - pid->previous_measurement) / pid->dt;
    }

    output = pid->kp * error + pid->integral + pid->kd * derivative;
    output = clamp_float(output, pid->output_min, pid->output_max);

    pid->previous_measurement = measurement;
    pid->has_previous_measurement = 1;
    return output;
}

float pid_integral(const PidController *pid)
{
    if (pid == 0) {
        return 0.0f;
    }

    return pid->integral;
}
