import cv2
import numpy as np
import argparse


class ColorTracker:
    def __init__(self, camera_index=0):
        self.cap = cv2.VideoCapture(camera_index)
        if not self.cap.isOpened():
            raise RuntimeError("Failed to open the camera")

        # Calibrated color (HSV), None = not calibrated yet
        self.target_hsv = None
        self.hsv_tolerance = np.array([15, 60, 60])  # H, S, V tolerance
        self.target_name = "N/A"
        self.min_contour_area = 800

        # Named color ranges for display
        self.color_names = [
            ("Red",    [0, 10], [170, 180]),
            ("Orange", [10, 25], None),
            ("Yellow", [25, 35], None),
            ("Green",  [35, 85], None),
            ("Cyan",   [85, 100], None),
            ("Blue",   [100, 130], None),
            ("Purple", [130, 160], None),
            ("Pink",   [160, 170], None),
        ]

        cv2.namedWindow("Color Tracker", cv2.WINDOW_NORMAL)
        cv2.namedWindow("Mask", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Color Tracker", 640, 480)
        cv2.resizeWindow("Mask", 640, 480)

    def get_color_name(self, h):
        """Map hue value to color name"""
        for name, range1, range2 in self.color_names:
            if range1[0] <= h <= range1[1]:
                return name
            if range2 and range2[0] <= h <= range2[1]:
                return name
        return "Unknown"

    def draw_crosshair(self, frame):
        """Draw crosshair at center of frame"""
        h, w = frame.shape[:2]
        cx, cy = w // 2, h // 2
        size = 30
        color = (0, 255, 0)
        # Crosshair lines
        cv2.line(frame, (cx - size, cy), (cx + size, cy), color, 2)
        cv2.line(frame, (cx, cy - size), (cx, cy + size), color, 2)
        # Center circle
        cv2.circle(frame, (cx, cy), 6, color, 2)
        return cx, cy

    def calibrate(self, hsv_frame, cx, cy):
        """Sample color at crosshair position (average a small region)"""
        h, w = hsv_frame.shape[:2]
        r = 10  # sample radius
        y1, y2 = max(0, cy - r), min(h, cy + r)
        x1, x2 = max(0, cx - r), min(w, cx + r)
        region = hsv_frame[y1:y2, x1:x2]
        self.target_hsv = np.mean(region, axis=(0, 1)).astype(np.uint8)
        self.target_name = self.get_color_name(self.target_hsv[0])

    def create_mask(self, hsv_frame):
        """Create mask based on calibrated color"""
        if self.target_hsv is None:
            return np.zeros(hsv_frame.shape[:2], dtype=np.uint8)

        lower = np.clip(self.target_hsv.astype(int) - self.hsv_tolerance, 0, 255).astype(np.uint8)
        upper = np.clip(self.target_hsv.astype(int) + self.hsv_tolerance, 0, 255).astype(np.uint8)

        # Handle hue wraparound for red-ish colors
        if lower[0] > upper[0]:
            mask1 = cv2.inRange(hsv_frame, np.array([0, lower[1], lower[2]]), upper)
            mask2 = cv2.inRange(hsv_frame, lower, np.array([180, upper[1], upper[2]]))
            mask = cv2.bitwise_or(mask1, mask2)
        else:
            mask = cv2.inRange(hsv_frame, lower, upper)

        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)
        return mask

    def find_and_draw(self, frame, mask):
        """Find contours and draw bounding boxes with color name"""
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        count = 0
        for c in contours:
            if cv2.contourArea(c) < self.min_contour_area:
                continue
            x, y, w, h = cv2.boundingRect(c)
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 255), 2)
            cv2.putText(frame, self.target_name, (x, y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
            count += 1
        return count

    def draw_info(self, frame, count):
        """Draw status info on frame"""
        if self.target_hsv is not None:
            info = f"Tracking: {self.target_name} | Objects: {count}"
        else:
            info = "Aim crosshair at target, press SPACE to calibrate"
        cv2.putText(frame, info, (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
        cv2.putText(frame, "SPACE: calibrate | q: quit", (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

    def run(self):
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    break

                hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
                cx, cy = self.draw_crosshair(frame)
                mask = self.create_mask(hsv)
                count = self.find_and_draw(frame, mask)
                self.draw_info(frame, count)

                cv2.imshow("Color Tracker", frame)
                cv2.imshow("Mask", mask)

                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break
                elif key == ord(' '):
                    self.calibrate(hsv, cx, cy)

                if (cv2.getWindowProperty("Color Tracker", cv2.WND_PROP_VISIBLE) < 1 or
                    cv2.getWindowProperty("Mask", cv2.WND_PROP_VISIBLE) < 1):
                    break
        finally:
            self.cap.release()
            cv2.destroyAllWindows()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="OpenCV Color Tracking Demo")
    parser.add_argument("-c", "--camera", type=int, default=0, help="Camera index (default: 0)")
    args = parser.parse_args()

    try:
        tracker = ColorTracker(camera_index=args.camera)
        tracker.run()
    except Exception as e:
        print(f"Program error: {e}")
