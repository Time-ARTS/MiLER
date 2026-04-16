# debug_renderer.py — optional OpenCV debug window
import cv2
import numpy as np
import settings

WINDOW_NAME = "MiLER DEBUG"

# colours (BGR)
COL_QUADRANT  = (245, 169, 184)
COL_POINT     = (91,  206, 250)
COL_CENTRELINE = (250, 206, 91)
COL_LINE      = (0,   255, 0)
COL_MOTOR     = (0,   0,   255)


def open_window():
    cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)


def close_window():
    cv2.destroyWindow(WINDOW_NAME)


def render(
    binary_frame: np.ndarray,
    quadrant_rois: list[tuple],
    points: list[tuple[float, float]],
    left_speed: float,
    right_speed: float,
):
    """
    Draw the debug overlay on a copy of the binary frame and show it.
    binary_frame  : the thresholded grayscale frame from Vision
    quadrant_rois : list of (x, y, w, h) tuples
    points        : detected midpoints [(x, y), ...]
    left_speed    : left motor speed (for the motor-direction indicator)
    right_speed   : right motor speed
    """
    display = cv2.cvtColor(binary_frame, cv2.COLOR_GRAY2BGR)
    h, w = display.shape[:2]
    cx = w // 2

    # Quadrant outlines
    for (qx, qy, qw, qh) in quadrant_rois:
        cv2.rectangle(display, (qx, qy), (qx + qw, qy + qh), COL_QUADRANT, 1)

    # Detected line midpoints
    for (px, py) in points:
        cv2.circle(display, (int(px), int(py)), 5, COL_POINT, -1)

    # Vertical centreline
    cv2.line(display, (cx, 0), (cx, h), COL_CENTRELINE, 2)

    # Smoothed line through detected points (polyline)
    if len(points) >= 2:
        pts = np.array([(int(p[0]), int(p[1])) for p in points], dtype=np.int32)
        cv2.polylines(display, [pts], False, COL_LINE, 2)

    # Motor direction indicator
    _draw_motor_indicator(display, left_speed, right_speed)

    # Speed readout
    cv2.putText(
        display,
        f"L:{left_speed:+.0f}  R:{right_speed:+.0f}",
        (10, 30),
        cv2.FONT_HERSHEY_SIMPLEX, 0.7, COL_MOTOR, 2,
    )

    cv2.imshow(WINDOW_NAME, display)
    cv2.waitKey(1)


def _draw_motor_indicator(frame, left: float, right: float):
    """Draw a simple curved arrow showing predicted robot path."""
    h, w = frame.shape[:2]
    diff = right - left
    curve = diff * 0.02

    prev = (w // 2, h - 1)
    for i in range(1, h // 2):
        t = i / 120.0
        bend = int(curve * t * t * 300)
        x = min(max(w // 2 + bend, 0), w - 1)
        y = h - 1 - i * 2
        curr = (x, y)
        cv2.line(frame, prev, curr, COL_MOTOR, 2)
        prev = curr
