# debug_renderer.py — OpenCV debug window showing colour tracking overlay
import cv2
import numpy as np
import settings

WINDOW_NAME = "MiLER — Line Tracker"

# Overlay colours (BGR)
COL_QUADRANT    = (245, 169, 184)   # pink  — quadrant band borders
COL_POINT       = (0,   220, 255)   # yellow — detected midpoints
COL_CENTRELINE  = (255, 255,  80)   # cyan  — frame centre guide
COL_TRACK_LINE  = (0,   255,   0)   # green — fitted path through midpoints
COL_MOTOR       = (0,    80, 255)   # red   — predicted steering arc
COL_LABEL       = (255, 255, 255)   # white — text
COL_HIGHLIGHT   = (0,   255, 100)   # green — detected line pixels tint


def open_window():
    cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
    # Resize window to show both panels comfortably
    cv2.resizeWindow(WINDOW_NAME, settings.FRAME_WIDTH * 2, settings.FRAME_HEIGHT)


def close_window():
    cv2.destroyWindow(WINDOW_NAME)


def render(
    binary_frame:   np.ndarray,
    colour_frame:   np.ndarray | None,
    quadrant_rois:  list[tuple],
    points:         list[tuple[float, float]],
    left_speed:     float,
    right_speed:    float,
):
    """
    Build a side-by-side debug view and show it.

    Left panel  — colour camera feed with tracking overlay drawn on top.
    Right panel — binary (thresholded) frame showing exactly what the
                  vision pipeline sees, also with quadrant borders.

    Args:
        binary_frame  : thresholded grayscale frame from Vision
        colour_frame  : raw BGR frame from Vision (may be None on first tick)
        quadrant_rois : list of (x, y, w, h) tuples
        points        : detected midpoints [(x, y), …]
        left_speed    : left motor speed
        right_speed   : right motor speed
    """
    h, w = binary_frame.shape[:2]

    # ── Right panel: binary view ────────────────────────────────────────
    right = cv2.cvtColor(binary_frame, cv2.COLOR_GRAY2BGR)
    _draw_quadrant_borders(right, quadrant_rois)
    _draw_midpoints(right, points)
    _draw_centreline(right)
    _label(right, "BINARY / THRESHOLD", (8, 22))

    # ── Left panel: colour overlay ──────────────────────────────────────
    if colour_frame is not None and colour_frame.size > 0:
        left = colour_frame.copy()
    else:
        # Fallback: invert binary so background is white-ish
        left = cv2.cvtColor(cv2.bitwise_not(binary_frame), cv2.COLOR_GRAY2BGR)

    # Tint the detected line pixels green so the user can see what's found
    # binary white pixels = line; turn them into a green overlay
    line_mask = binary_frame > 0
    tint = left.copy()
    tint[line_mask] = (
        tint[line_mask] * 0.4 + np.array(COL_HIGHLIGHT, dtype=np.float32) * 0.6
    ).astype(np.uint8)
    left = tint

    _draw_quadrant_borders(left, quadrant_rois)
    _draw_midpoints(left, points)
    _draw_centreline(left)
    _draw_track_line(left, points)
    _draw_motor_arc(left, left_speed, right_speed)
    _draw_speed_hud(left, left_speed, right_speed, points)
    _label(left, "COLOUR + TRACKING OVERLAY", (8, 22))

    # ── Combine and show ────────────────────────────────────────────────
    combined = np.hstack([left, right])
    cv2.imshow(WINDOW_NAME, combined)
    cv2.waitKey(1)


# ── Drawing helpers ──────────────────────────────────────────────────────

def _draw_quadrant_borders(frame, rois):
    for (qx, qy, qw, qh) in rois:
        cv2.rectangle(frame, (qx, qy), (qx + qw, qy + qh), COL_QUADRANT, 1)


def _draw_midpoints(frame, points):
    for (px, py) in points:
        # Outer ring
        cv2.circle(frame, (int(px), int(py)), 7, (0, 0, 0), -1)
        # Filled centre
        cv2.circle(frame, (int(px), int(py)), 5, COL_POINT, -1)


def _draw_centreline(frame):
    h, w = frame.shape[:2]
    cx = w // 2
    cv2.line(frame, (cx, 0), (cx, h), COL_CENTRELINE, 1, cv2.LINE_AA)
    # Small tick labels
    cv2.putText(frame, "CENTRE", (cx + 4, 14),
                cv2.FONT_HERSHEY_SIMPLEX, 0.35, COL_CENTRELINE, 1)


def _draw_track_line(frame, points):
    """Draw a smooth polyline through the detected midpoints."""
    if len(points) < 2:
        return
    pts = np.array([(int(p[0]), int(p[1])) for p in points], dtype=np.int32)
    # Shadow
    cv2.polylines(frame, [pts], False, (0, 0, 0), 4, cv2.LINE_AA)
    # Actual line
    cv2.polylines(frame, [pts], False, COL_TRACK_LINE, 2, cv2.LINE_AA)


def _draw_motor_arc(frame, left: float, right: float):
    """Curved arrow showing the robot's predicted steering direction."""
    h, w = frame.shape[:2]
    diff  = right - left
    curve = diff * 0.02

    prev = (w // 2, h - 1)
    for i in range(1, h // 2):
        t    = i / 120.0
        bend = int(curve * t * t * 300)
        x    = min(max(w // 2 + bend, 0), w - 1)
        y    = h - 1 - i * 2
        curr = (x, y)
        cv2.line(frame, prev, curr, COL_MOTOR, 2, cv2.LINE_AA)
        prev = curr


def _draw_speed_hud(frame, left: float, right: float,
                    points: list[tuple[float, float]]):
    """Bottom-left HUD: speeds, error, and line-found status."""
    h, w = frame.shape[:2]

    # Background pill
    cv2.rectangle(frame, (0, h - 60), (260, h), (0, 0, 0), -1)

    frame_cx  = w // 2
    avg_x     = (sum(p[0] for p in points) / len(points)) if points else frame_cx
    error     = avg_x - frame_cx
    line_found = len(points) > 0

    status_col = (0, 220, 0) if line_found else (0, 0, 220)
    status_txt = f"LINE: {'FOUND (%d pts)' % len(points) if line_found else 'LOST'}"

    cv2.putText(frame, f"L:{left:+.0f}  R:{right:+.0f}",
                (6, h - 40), cv2.FONT_HERSHEY_SIMPLEX, 0.55, COL_LABEL, 1)
    cv2.putText(frame, f"Error: {error:+.1f} px",
                (6, h - 22), cv2.FONT_HERSHEY_SIMPLEX, 0.50, COL_LABEL, 1)
    cv2.putText(frame, status_txt,
                (6, h - 6),  cv2.FONT_HERSHEY_SIMPLEX, 0.50, status_col, 1)


def _label(frame, text: str, pos: tuple):
    """Small corner label."""
    cv2.putText(frame, text, pos,
                cv2.FONT_HERSHEY_SIMPLEX, 0.40, (0, 0, 0),     2)
    cv2.putText(frame, text, pos,
                cv2.FONT_HERSHEY_SIMPLEX, 0.40, COL_LABEL, 1)
