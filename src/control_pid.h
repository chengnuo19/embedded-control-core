#ifndef CONTROL_PID_H
#define CONTROL_PID_H

typedef struct {
    float kp;
    float ki;
    float kd;
    float dt;
    float output_min;
    float output_max;
    float integral_min;
    float integral_max;
    float integral;
    float previous_measurement;
    int has_previous_measurement;
} PidController;

int pid_init(PidController *pid,
             float kp,
             float ki,
             float kd,
             float dt,
             float output_min,
             float output_max,
             float integral_min,
             float integral_max);

void pid_set_gains(PidController *pid, float kp, float ki, float kd);
void pid_set_target_period(PidController *pid, float dt);
void pid_reset(PidController *pid);
float pid_update(PidController *pid, float target, float measurement);
float pid_integral(const PidController *pid);

#endif
