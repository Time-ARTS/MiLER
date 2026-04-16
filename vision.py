# vision.py — camera capture and line detection
import cv2
import numpy as np
import settings

try:
    from picamera2 import Picamera2
    PICAMERA2_AVAILABLE = True
except ImportError:
    PICAMERA2_AVAILABLE = False


class Vision:
    """
    Captures frames from the Pi camera (or a USB cam as fallback),
    applies thresholding to isolate the black line, and exposes the
    processed frame for the quadrant handler to work with.
    """

    def __init__(self):
        self._frame = None
        self._cap = None
        self._picam = None

        if PICAMERA2_AVAILABLE:
            self._init_picamera2()
        else:
            print("[Vision] picamera2 not found — falling back to OpenCV VideoCapture")
            self._init_opencv()

    # ------------------------------------------------------------------
    # Init helpers
    # ------------------------------------------------------------------

    def _init_picamera2(self):
        self._picam = Picamera2()
        config = self._picam.create_preview_configuration(
            main={"size": (settings.FRAME_WIDTH, settings.FRAME_HEIGHT), "format": "RGB888"}
        )
        self._picam.configure(config)
        self._picam.start()
        print("[Vision] picamera2 started")

    def _init_opencv(self):
        self._cap = cv2.VideoCapture(0)
        if not self._cap.isOpened():
            raise RuntimeError("Could not open camera with OpenCV VideoCapture")
        self._cap.set(cv2.CAP_PROP_FRAME_WIDTH,  settings.FRAME_WIDTH)
        self._cap.set(cv2.CAP_PROP_FRAME_HEIGHT, settings.FRAME_HEIGHT)
        print("[Vision] OpenCV VideoCapture started")

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def get_processed_frame(self) -> np.ndarray:
        """
        Returns a binary (thresholded) frame where white pixels are the
        black line and black pixels are the background.
        """
        raw = self._grab_raw_frame()
        return self._process(raw)

    def detected_line(self) -> bool:
        """Quick check — is there a line in the current frame?"""
        frame = self.get_processed_frame()
        if frame is None or frame.size == 0:
            return False
        moments = cv2.moments(frame, binaryImage=True)
        return moments["m00"] > settings.MIN_LINE_AREA

    def release(self):
        if self._picam:
            self._picam.stop()
        if self._cap:
            self._cap.release()

    # ------------------------------------------------------------------
    # Internals
    # ------------------------------------------------------------------

    def _grab_raw_frame(self) -> np.ndarray:
        if self._picam:
            # picamera2 returns RGB888 — convert to BGR for OpenCV
            rgb = self._picam.capture_array()
            return cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)
        else:
            ok, frame = self._cap.read()
            if not ok:
                raise RuntimeError("Failed to read frame from camera")
            return frame

    def _process(self, frame: np.ndarray) -> np.ndarray:
        """BGR → grayscale → blur → threshold → morphology"""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)

        cutoff = int((1.0 - settings.COLOR_THRESHOLD) * 255)
        _, binary = cv2.threshold(blurred, cutoff, 255, cv2.THRESH_BINARY_INV)

        # Clean up noise
        kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
        binary = cv2.morphologyEx(binary, cv2.MORPH_OPEN,  kernel, iterations=1)
        binary = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel, iterations=2)

        self._frame = binary
        return binary

    @property
    def last_frame(self) -> np.ndarray | None:
        """The most recently processed (binary) frame."""
        return self._frame
