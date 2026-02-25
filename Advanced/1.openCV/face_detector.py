import cv2
import argparse
import os


class FaceDetector:
    def __init__(self, camera_index=0):
        self.cap = cv2.VideoCapture(camera_index)
        if not self.cap.isOpened():
            raise RuntimeError("Failed to open the camera")

        # LBP cascade path: same parent dir as haarcascades
        data_dir = os.path.dirname(cv2.data.haarcascades)
        lbp_path = os.path.join(data_dir, "lbpcascades", "lbpcascade_frontalface_improved.xml")

        # Use LBP cascade (more robust with glasses) as primary
        self.face_cascade_lbp = cv2.CascadeClassifier(lbp_path)
        # If LBP not available, fall back to Haar only
        self.lbp_available = not self.face_cascade_lbp.empty()

        # Haar as fallback
        self.face_cascade_haar = cv2.CascadeClassifier(
            cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
        )
        self.eye_cascade = cv2.CascadeClassifier(
            cv2.data.haarcascades + "haarcascade_eye_tree_eyeglasses.xml"
        )

        self.detect_eyes = True
        self.use_lbp = self.lbp_available

        cv2.namedWindow("Face Detection", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Face Detection", 640, 480)

    def detect(self, frame):
        """Detect faces using dual-cascade strategy"""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        # CLAHE for better contrast (works better than simple equalizeHist)
        clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        gray = clahe.apply(gray)

        # Try LBP first (faster, better with glasses), fallback to Haar
        if self.use_lbp:
            faces = self.face_cascade_lbp.detectMultiScale(
                gray, scaleFactor=1.1, minNeighbors=4, minSize=(80, 80)
            )
            method = "LBP"
        else:
            faces = self.face_cascade_haar.detectMultiScale(
                gray, scaleFactor=1.1, minNeighbors=6, minSize=(80, 80)
            )
            method = "Haar"

        for (x, y, w, h) in faces:
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.putText(frame, "Face", (x, y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

            if self.detect_eyes:
                # Search eyes only in upper 60% of face region
                eye_h = int(h * 0.6)
                roi_gray = gray[y:y + eye_h, x:x + w]
                roi_color = frame[y:y + eye_h, x:x + w]
                eyes = self.eye_cascade.detectMultiScale(
                    roi_gray, scaleFactor=1.05, minNeighbors=6, minSize=(25, 25)
                )
                # Keep at most 2 eyes
                eyes = sorted(eyes, key=lambda e: e[2] * e[3], reverse=True)[:2]
                for (ex, ey, ew, eh) in eyes:
                    cv2.rectangle(roi_color, (ex, ey), (ex + ew, ey + eh), (255, 0, 0), 2)

        face_count = len(faces)
        eye_status = "Eyes: ON" if self.detect_eyes else "Eyes: OFF"
        info = f"Faces: {face_count} | {method} | {eye_status}"
        cv2.putText(frame, info, (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
        cv2.putText(frame, "e: toggle eyes | m: switch model | q: quit",
                    (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)
        return frame

    def run(self):
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    break

                result = self.detect(frame)
                cv2.imshow("Face Detection", result)

                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break
                elif key == ord('e'):
                    self.detect_eyes = not self.detect_eyes
                elif key == ord('m'):
                    if self.lbp_available:
                        self.use_lbp = not self.use_lbp

                if cv2.getWindowProperty("Face Detection", cv2.WND_PROP_VISIBLE) < 1:
                    break
        finally:
            self.cap.release()
            cv2.destroyAllWindows()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="OpenCV Face Detection Demo")
    parser.add_argument("-c", "--camera", type=int, default=0, help="Camera index (default: 0)")
    args = parser.parse_args()

    try:
        detector = FaceDetector(camera_index=args.camera)
        detector.run()
    except Exception as e:
        print(f"Program error: {e}")
