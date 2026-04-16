# quadrant_handler.py — divides the frame into horizontal bands
# and finds the centroid of the line in each one.
import cv2
import numpy as np
from dataclasses import dataclass, field
from typing import Optional
import settings


@dataclass
class Quadrant:
    roi: tuple  # (x, y, w, h)

    def find_midpoint(self, frame: np.ndarray) -> Optional[tuple[float, float]]:
        """
        Finds the centroid of the largest contour inside this quadrant.
        Returns (x, y) in full-frame coordinates, or None if nothing found.
        """
        x, y, w, h = self.roi
        if w <= 0 or h <= 0:
            return None

        region = frame[y:y+h, x:x+w]
        contours, _ = cv2.findContours(region, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if not contours:
            return None

        largest = max(contours, key=cv2.contourArea)
        m = cv2.moments(largest)

        if m["m00"] == 0:
            return None

        cx = m["m10"] / m["m00"] + x  # offset back to full-frame coords
        cy = m["m01"] / m["m00"] + y
        return (cx, cy)


class QuadrantHandler:
    """
    Splits the frame into QUADRANT_NUM horizontal bands with a small gap
    between each, then finds the line centroid in every band.
    """

    def __init__(self):
        self._quadrants: list[Quadrant] = []
        self._frame_shape: tuple | None = None

    def setup(self, frame: np.ndarray):
        """Build quadrant grid from the first frame's dimensions."""
        h, w = frame.shape[:2]
        self._frame_shape = (h, w)
        self._quadrants = []

        num = settings.QUADRANT_NUM
        spacing = settings.QUADRANT_SPACING_Y
        offset = settings.QUADRANT_OFFSET_Y

        available_h = h - offset - spacing * (num - 1)
        quad_h = available_h / num

        for i in range(num):
            start_y = int(offset + i * (quad_h + spacing))
            actual_h = max(0, min(int(quad_h), h - start_y))
            self._quadrants.append(Quadrant(roi=(0, start_y, w, actual_h)))

    def process(self, frame: np.ndarray) -> list[tuple[float, float]]:
        """
        Returns a list of (x, y) midpoints — one per quadrant that found the line.
        Quadrants with no line return nothing (filtered out).
        """
        if not self._quadrants:
            self.setup(frame)

        points = []
        for q in self._quadrants:
            pt = q.find_midpoint(frame)
            if pt is not None:
                points.append(pt)

        return points

    @property
    def quadrants(self) -> list[Quadrant]:
        return self._quadrants
