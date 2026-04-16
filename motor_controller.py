# motor_controller.py — drives two DC motors via GPIO (TB6612 / L298N)
import time
import settings

try:
    import RPi.GPIO as GPIO
    GPIO_AVAILABLE = True
except ImportError:
    GPIO_AVAILABLE = False
    print("[MotorController] RPi.GPIO not available — running in stub mode (no motors)")


class MotorController:
    """
    Controls left and right motors via H-bridge (TB6612 or similar).

    Speed range: -255 (full reverse) to +255 (full forward).
    """

    def __init__(self):
        self._pwm_left = None
        self._pwm_right = None

        if GPIO_AVAILABLE:
            self._setup_gpio()

    # ------------------------------------------------------------------
    # Setup
    # ------------------------------------------------------------------

    def _setup_gpio(self):
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)

        pins = [
            settings.STANDBY_PIN,
            settings.EN_LEFT,  settings.IN1_LEFT,  settings.IN2_LEFT,
            settings.EN_RIGHT, settings.IN1_RIGHT, settings.IN2_RIGHT,
        ]
        for pin in pins:
            GPIO.setup(pin, GPIO.OUT)

        # PWM on enable pins
        self._pwm_left  = GPIO.PWM(settings.EN_LEFT,  settings.PWM_FREQUENCY)
        self._pwm_right = GPIO.PWM(settings.EN_RIGHT, settings.PWM_FREQUENCY)
        self._pwm_left.start(0)
        self._pwm_right.start(0)

        # Release standby
        GPIO.output(settings.STANDBY_PIN, GPIO.HIGH)
        print("[MotorController] GPIO ready")

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def drive(self, left_speed: float, right_speed: float):
        """
        Drive both motors. Speeds are clamped to [-255, 255].
        Positive = forward, negative = reverse.
        """
        left_speed  = max(settings.MIN_SPEED, min(settings.MAX_SPEED, left_speed))
        right_speed = max(settings.MIN_SPEED, min(settings.MAX_SPEED, right_speed))

        self._set_motor(
            left_speed,
            settings.IN1_LEFT, settings.IN2_LEFT, self._pwm_left,
            label="L"
        )
        self._set_motor(
            right_speed,
            settings.IN1_RIGHT, settings.IN2_RIGHT, self._pwm_right,
            label="R"
        )

    def stop(self):
        """Brake both motors."""
        self.drive(0, 0)

    def backtrack(self, duration_s: float = settings.BACKTRACK_DURATION_S):
        """Reverse both motors briefly to recover a lost line."""
        self.drive(-settings.DEFAULT_SPEED, -settings.DEFAULT_SPEED)
        time.sleep(duration_s)
        self.stop()

    def self_test(self):
        """Short forward/backward pulse to verify wiring."""
        print("[MotorController] Self-test: forward 0.5 s")
        self.drive(150, 150)
        time.sleep(0.5)
        print("[MotorController] Self-test: reverse 0.5 s")
        self.drive(-150, -150)
        time.sleep(0.5)
        self.stop()
        print("[MotorController] Self-test complete")

    def cleanup(self):
        self.stop()
        if GPIO_AVAILABLE:
            if self._pwm_left:  self._pwm_left.stop()
            if self._pwm_right: self._pwm_right.stop()
            GPIO.cleanup()

    # ------------------------------------------------------------------
    # Internals
    # ------------------------------------------------------------------

    def _set_motor(self, speed: float, in1: int, in2: int, pwm, label: str):
        duty = abs(speed) / 255.0 * 100.0  # GPIO PWM expects 0-100

        if not GPIO_AVAILABLE:
            direction = "FWD" if speed > 0 else ("REV" if speed < 0 else "STOP")
            print(f"  [Motor {label}] {direction}  duty={duty:.1f}%")
            return

        if speed > 0:
            GPIO.output(in1, GPIO.HIGH)
            GPIO.output(in2, GPIO.LOW)
        elif speed < 0:
            GPIO.output(in1, GPIO.LOW)
            GPIO.output(in2, GPIO.HIGH)
        else:
            GPIO.output(in1, GPIO.LOW)
            GPIO.output(in2, GPIO.LOW)

        pwm.ChangeDutyCycle(duty)
