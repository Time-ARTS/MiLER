# pid.py — discrete PID controller
import time
import settings


class PID:
    def __init__(
        self,
        kp: float = settings.PID_KP,
        ki: float = settings.PID_KI,
        kd: float = settings.PID_KD,
    ):
        self.kp = kp
        self.ki = ki
        self.kd = kd

        self._integral = 0.0
        self._prev_error = 0.0
        self._prev_time = time.monotonic()

    def compute(self, error: float) -> float:
        """
        Takes the current error and returns a correction value.
        Call once per control loop iteration.
        """
        now = time.monotonic()
        dt = now - self._prev_time
        if dt <= 0:
            dt = 1e-6

        self._integral += error * dt
        derivative = (error - self._prev_error) / dt

        output = self.kp * error + self.ki * self._integral + self.kd * derivative

        self._prev_error = error
        self._prev_time = now

        return output

    def reset(self):
        self._integral = 0.0
        self._prev_error = 0.0
        self._prev_time = time.monotonic()
