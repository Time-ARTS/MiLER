# loop.py — main control loop
import time
import settings

from vision import Vision
from quadrant_handler import QuadrantHandler
from pid import PID
from motor_controller import MotorController
from frame_buffer import FrameBuffer


class Loop:
    def __init__(self, frame_buffer: FrameBuffer):
        self.vision    = Vision()
        self.quadrants = QuadrantHandler()
        self.pid       = PID()
        self.motors    = MotorController()
        self._buf      = frame_buffer   # shared with main thread for rendering

        self._line_detected  = False
        self._last_line_time = time.monotonic()

    # ------------------------------------------------------------------
    # Entry point
    # ------------------------------------------------------------------

    def run(self, running_flag):
        """
        running_flag: threading.Event — loop runs while it is set.
        """
        print("[Loop] Starting")
        while running_flag.is_set():
            if not self._line_test():
                print("[Loop] Line lost — stopping")
                self.motors.stop()
                break
            self._step()

    # ------------------------------------------------------------------
    # Single iteration
    # ------------------------------------------------------------------

    def _step(self):
        frame  = self.vision.get_processed_frame()
        points = self.quadrants.process(frame)

        if not points:
            # Still push a frame so the debug window keeps updating
            if settings.ENABLE_DEBUG:
                rois = [q.roi for q in self.quadrants.quadrants]
                self._buf.put(frame, self.vision.last_raw_frame,
                              rois, [], 0.0, 0.0)
            return

        # --- error: deviation of average x from frame centre ---
        frame_centre_x = settings.FRAME_WIDTH / 2.0
        avg_x          = sum(p[0] for p in points) / len(points)
        error          = avg_x - frame_centre_x   # positive → line is right

        correction  = self.pid.compute(error)
        base        = float(settings.DEFAULT_SPEED)
        left_speed  = max(settings.MIN_SPEED,
                          min(settings.MAX_SPEED, base + correction))
        right_speed = max(settings.MIN_SPEED,
                          min(settings.MAX_SPEED, base - correction))

        if settings.ENABLE_DRIVING:
            self.motors.drive(left_speed, right_speed)

        if settings.ENABLE_DEBUG:
            rois = [q.roi for q in self.quadrants.quadrants]
            # Write to shared buffer — main thread will render this
            self._buf.put(
                frame,
                self.vision.last_raw_frame,
                rois,
                points,
                left_speed,
                right_speed,
            )

        time.sleep(settings.ROBOT_LOOP_INTERVAL_S)

    # ------------------------------------------------------------------
    # Line-loss detection + backtrack recovery
    # ------------------------------------------------------------------

    def _line_test(self) -> bool:
        frame  = self.vision.get_processed_frame()
        points = self.quadrants.process(frame)

        if points:
            self._line_detected  = True
            self._last_line_time = time.monotonic()
            return True

        print("[Loop] Line lost — attempting backtrack recovery…")
        recovery_start = time.monotonic()

        while time.monotonic() - recovery_start < settings.MAX_RECOVERY_TIME_S:
            self.motors.backtrack(settings.BACKTRACK_DURATION_S)
            frame  = self.vision.get_processed_frame()
            points = self.quadrants.process(frame)
            if points:
                print("[Loop] Line re-acquired")
                self._last_line_time = time.monotonic()
                return True

        print("[Loop] Recovery failed")
        self._line_detected = False
        return False

    def cleanup(self):
        self.motors.cleanup()
        self.vision.release()
# loop.py — main control loop
import time
import settings
import debug_renderer

from vision import Vision
from quadrant_handler import QuadrantHandler
from pid import PID
from motor_controller import MotorController


class Loop:
    def __init__(self):
        self.vision    = Vision()
        self.quadrants = QuadrantHandler()
        self.pid       = PID()
        self.motors    = MotorController()

        self._line_detected = False
        self._last_line_time = time.monotonic()

    # ------------------------------------------------------------------
    # Entry point (called from main.py when running == True)
    # ------------------------------------------------------------------

    def run(self, running_flag):
        """
        running_flag: a threading.Event or similar object with .is_set().
        Keeps looping until the flag is cleared or the line is lost too long.
        """
        print("[Loop] Starting")
        while running_flag.is_set():
            if not self._line_test():
                print("[Loop] Line lost — stopping")
                self.motors.stop()
                break
            self._step()

    # ------------------------------------------------------------------
    # Single iteration
    # ------------------------------------------------------------------

    def _step(self):
        frame = self.vision.get_processed_frame()
        points = self.quadrants.process(frame)

        if not points:
            return

        # --- Control signal: deviation of average x from frame centre ---
        frame_centre_x = settings.FRAME_WIDTH / 2.0
        avg_x = sum(p[0] for p in points) / len(points)
        error = avg_x - frame_centre_x   # positive → line is to the right

        correction = self.pid.compute(error)

        # Positive correction → steer right (speed up right motor, slow left)
        base = float(settings.DEFAULT_SPEED)
        left_speed  = base + correction
        right_speed = base - correction

        # Clamp
        left_speed  = max(settings.MIN_SPEED, min(settings.MAX_SPEED, left_speed))
        right_speed = max(settings.MIN_SPEED, min(settings.MAX_SPEED, right_speed))

        if settings.ENABLE_DRIVING:
            self.motors.drive(left_speed, right_speed)

        if settings.ENABLE_DEBUG:
            rois = [q.roi for q in self.quadrants.quadrants]
            debug_renderer.render(frame, rois, points, left_speed, right_speed)

        time.sleep(settings.ROBOT_LOOP_INTERVAL_S)

    # ------------------------------------------------------------------
    # Line-loss detection + backtrack recovery
    # ------------------------------------------------------------------

    def _line_test(self) -> bool:
        frame = self.vision.get_processed_frame()
        points = self.quadrants.process(frame)

        if points:
            self._line_detected = True
            self._last_line_time = time.monotonic()
            return True

        print("[Loop] Line lost — attempting backtrack recovery…")
        recovery_start = time.monotonic()

        while time.monotonic() - recovery_start < settings.MAX_RECOVERY_TIME_S:
            self.motors.backtrack(settings.BACKTRACK_DURATION_S)

            frame = self.vision.get_processed_frame()
            points = self.quadrants.process(frame)

            if points:
                print("[Loop] Line re-acquired")
                self._last_line_time = time.monotonic()
                return True

        print("[Loop] Recovery failed")
        self._line_detected = False
        return False

    def cleanup(self):
        self.motors.cleanup()
        self.vision.release()
        debug_renderer.close_window()
