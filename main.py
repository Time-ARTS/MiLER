#!/usr/bin/env python3
# main.py — MiLER entry point
"""
Controls:
  r  — start / stop the robot
  d  — toggle debug window
  q  — quit
"""
import sys
import time
import threading
import settings
import debug_renderer

from loop import Loop


def read_key_nonblocking() -> str | None:
    """
    Returns a single character from stdin without blocking, or None.
    Works on Linux/macOS (requires a terminal).
    """
    try:
        import select, tty, termios
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            r, _, _ = select.select([sys.stdin], [], [], 0)
            if r:
                return sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)
    except Exception:
        pass
    return None


def main():
    print("=== MiLER Python ===")
    print("  r = start/stop")
    print("  d = toggle debug")
    print("  q = quit")
    print()

    loop = Loop()
    running = threading.Event()  # set → robot should run
    robot_thread: threading.Thread | None = None
    start_time = time.monotonic()

    if settings.ENABLE_DEBUG:
        debug_renderer.open_window()

    try:
        while True:
            # --- keyboard ---
            key = read_key_nonblocking()
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
                time.sleep(0.3)  # debounce

            elif key == 'd':
                settings.ENABLE_DEBUG = not settings.ENABLE_DEBUG
                print(f"Debug: {'ON' if settings.ENABLE_DEBUG else 'OFF'}")
                if settings.ENABLE_DEBUG:
                    debug_renderer.open_window()
                else:
                    debug_renderer.close_window()

            # --- auto-stop if robot thread finished on its own ---
            if robot_thread and not robot_thread.is_alive():
                running.clear()
                robot_thread = None
                print("Robot stopped (line lost or error).")

            # --- max runtime guard ---
            if time.monotonic() - start_time > settings.MAX_RUNTIME_S:
                print("Max runtime reached. Exiting.")
                break

            time.sleep(settings.POLL_INTERVAL_S)

    except KeyboardInterrupt:
        print("\nInterrupted.")

    finally:
        running.clear()
        if robot_thread:
            robot_thread.join(timeout=3)
        loop.cleanup()
        print("Shutdown complete.")


if __name__ == "__main__":
    main()
