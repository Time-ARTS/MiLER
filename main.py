#!/usr/bin/env python3
# main.py — MiLER entry point
"""
Controls (type the letter and press Enter):
  r  — start / stop the robot
  d  — toggle debug window
  q  — quit

Works whether you are:
  - sitting at the Pi directly (keyboard + monitor)
  - connected via Remote Desktop (VNC, RDP, x2go)
  - connected via SSH  (ssh pi@<ip>  — no special flags needed)
"""
import os
import sys
import time
import queue
import threading

# ── Wayland / display fix — must happen before cv2 is imported ──────────
# Pi OS Bookworm (Debian 12) and later default to a Wayland compositor.
# OpenCV's imshow() needs X11/XCB.  Setting these variables tells the
# Qt/GTK backend to use XWayland automatically.
if "DISPLAY" not in os.environ:
    os.environ["DISPLAY"] = ":0"
os.environ.setdefault("QT_QPA_PLATFORM", "xcb")
# ────────────────────────────────────────────────────────────────────────

import settings
import debug_renderer
from loop import Loop
from frame_buffer import FrameBuffer


# ── Keyboard input thread ────────────────────────────────────────────────
# We read whole lines (press Enter after the letter). This works on every
# connection type: direct terminal, Remote Desktop terminal, plain SSH.
# No raw-terminal / tty tricks needed at all.

def _keyboard_thread(cmd_queue: queue.Queue, stop_event: threading.Event):
    """Runs in a daemon thread. Puts single-char commands onto cmd_queue."""
    print("[Keys] Type a command and press Enter:  r=start/stop  d=debug  q=quit")
    while not stop_event.is_set():
        try:
            line = input().strip().lower()
        except EOFError:
            # stdin closed (e.g. piped input finished) — treat as quit
            cmd_queue.put('q')
            break
        if line:
            cmd_queue.put(line[0])   # only the first character matters


# ── Main ─────────────────────────────────────────────────────────────────

def main():
    print("=== MiLER Python ===")
    print()

    buf         = FrameBuffer()
    loop        = Loop(buf)
    running     = threading.Event()   # set → robot should run
    stop_input  = threading.Event()   # set → keyboard thread should exit
    cmd_queue   : queue.Queue = queue.Queue()
    robot_thread: threading.Thread | None = None
    start_time  = time.monotonic()

    # Start keyboard listener in background
    kbd = threading.Thread(
        target=_keyboard_thread,
        args=(cmd_queue, stop_input),
        daemon=True,
    )
    kbd.start()

    if settings.ENABLE_DEBUG:
        debug_renderer.open_window()
        print("[Main] Debug window opened.")

    try:
        while True:

            # ── Handle commands from keyboard thread ──────────────────
            try:
                key = cmd_queue.get_nowait()
            except queue.Empty:
                key = None

            if key == 'q':
                print("Quit.")
                break

            elif key == 'r':
                if running.is_set():
                    print("Stopping robot.")
                    running.clear()
                    if robot_thread:
                        robot_thread.join(timeout=3)
                        robot_thread = None
                else:
                    print("Starting robot.")
                    running.set()
                    robot_thread = threading.Thread(
                        target=loop.run, args=(running,), daemon=True
                    )
                    robot_thread.start()

            elif key == 'd':
                settings.ENABLE_DEBUG = not settings.ENABLE_DEBUG
                print(f"Debug: {'ON' if settings.ENABLE_DEBUG else 'OFF'}")
                if settings.ENABLE_DEBUG:
                    debug_renderer.open_window()
                else:
                    debug_renderer.close_window()

            # ── Render latest frame (MUST be on main thread) ──────────
            if settings.ENABLE_DEBUG:
                data = buf.get()
                if data is not None:
                    binary, colour, rois, points, ls, rs = data
                    debug_renderer.render(binary, colour, rois, points, ls, rs)

            # ── Auto-stop if robot thread finished by itself ──────────
            if robot_thread and not robot_thread.is_alive():
                running.clear()
                robot_thread = None
                print("Robot stopped (line lost or error).")

            # ── Max runtime guard ─────────────────────────────────────
            if time.monotonic() - start_time > settings.MAX_RUNTIME_S:
                print("Max runtime reached. Exiting.")
                break

            time.sleep(settings.POLL_INTERVAL_S)

    except KeyboardInterrupt:
        print("\nInterrupted.")

    finally:
        stop_input.set()
        running.clear()
        if robot_thread:
            robot_thread.join(timeout=3)
        loop.cleanup()
        debug_renderer.close_window()
        print("Shutdown complete.")


if __name__ == "__main__":
    main()
