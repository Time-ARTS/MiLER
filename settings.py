# settings.py — MiLER configuration

# --- PID ---
PID_KP = 0.5
PID_KI = 0.0
PID_KD = 0.1

# --- Speed ---
MAX_SPEED = 255
MIN_SPEED = -255
DEFAULT_SPEED = 200
STEERING_COUNT = 50

# --- Vision ---
COLOR_THRESHOLD = 0.80      # fraction of 255; pixels darker than this are "line"
MIN_LINE_AREA = 500         # minimum contour area to count as a detected line

# --- Quadrants ---
QUADRANT_NUM = 24
QUADRANT_SPACING_Y = 15.0
QUADRANT_OFFSET_Y = 0.0

# --- Camera ---
FRAME_WIDTH = 640
FRAME_HEIGHT = 480

# --- Timing ---
ROBOT_LOOP_INTERVAL_S = 0.03    # ~33 fps
BACKTRACK_DURATION_S = 0.3
MAX_RECOVERY_TIME_S = 10.0
SLEEP_TIME_S = 60.0
MAX_RUNTIME_S = 300.0

# --- Motor GPIO pins (BCM numbering) ---
STANDBY_PIN = 26

EN_LEFT  = 22
IN1_LEFT = 17
IN2_LEFT = 27

EN_RIGHT  = 25
IN1_RIGHT = 23
IN2_RIGHT = 24

PWM_FREQUENCY = 1000   # Hz

# --- Button ---
START_STOP_PIN = 12
ACTIVE_LOW = True
DEBOUNCE_S = 0.5
POLL_INTERVAL_S = 0.05

# --- Feature flags ---
ENABLE_DRIVING = True
ENABLE_VISION  = True
ENABLE_DEBUG   = True
