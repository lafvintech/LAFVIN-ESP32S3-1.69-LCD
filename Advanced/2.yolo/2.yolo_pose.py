import cv2
import time
import argparse
from ultralytics import YOLO

# Skeleton connections: pairs of keypoint indices to draw bones
# YOLOv8-pose keypoints: 0=nose, 1=left_eye, 2=right_eye, 3=left_ear, 4=right_ear,
# 5=left_shoulder, 6=right_shoulder, 7=left_elbow, 8=right_elbow, 9=left_wrist,
# 10=right_wrist, 11=left_hip, 12=right_hip, 13=left_knee, 14=right_knee,
# 15=left_ankle, 16=right_ankle
SKELETON = [
    (0, 1), (0, 2), (1, 3), (2, 4),           # head
    (5, 6),                                     # shoulders
    (5, 7), (7, 9),                             # left arm
    (6, 8), (8, 10),                            # right arm
    (5, 11), (6, 12),                           # torso
    (11, 12),                                   # hips
    (11, 13), (13, 15),                         # left leg
    (12, 14), (14, 16),                         # right leg
]

# Colors for left/right sides
COLORS = {
    "left":  (255, 128, 0),   # orange
    "right": (0, 128, 255),   # blue
    "center": (0, 255, 0),    # green
}

def get_bone_color(i, j):
    """Assign color based on body side"""
    left_indices = {1, 3, 5, 7, 9, 11, 13, 15}
    right_indices = {2, 4, 6, 8, 10, 12, 14, 16}
    if i in left_indices or j in left_indices:
        return COLORS["left"]
    if i in right_indices or j in right_indices:
        return COLORS["right"]
    return COLORS["center"]


def draw_skeleton(frame, keypoints, conf_threshold=0.5):
    """Draw keypoints and skeleton on frame"""
    kpts = keypoints.data[0]  # shape: (17, 3) -> x, y, conf

    # Draw bones
    for (i, j) in SKELETON:
        if kpts[i][2] > conf_threshold and kpts[j][2] > conf_threshold:
            pt1 = (int(kpts[i][0]), int(kpts[i][1]))
            pt2 = (int(kpts[j][0]), int(kpts[j][1]))
            color = get_bone_color(i, j)
            cv2.line(frame, pt1, pt2, color, 2, cv2.LINE_AA)

    # Draw keypoints
    for idx, kpt in enumerate(kpts):
        if kpt[2] > conf_threshold:
            cx, cy = int(kpt[0]), int(kpt[1])
            cv2.circle(frame, (cx, cy), 4, (0, 255, 255), -1, cv2.LINE_AA)


def main(camera_index=0):
    cap = cv2.VideoCapture(camera_index)
    if not cap.isOpened():
        raise RuntimeError("Failed to open the camera")

    # Load YOLOv8 nano pose model (auto-downloads ~6MB on first run)
    model = YOLO("yolov8n-pose.pt")

    cv2.namedWindow("YOLOv8 Pose", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("YOLOv8 Pose", 640, 480)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            start_time = time.time()

            # Run inference
            results = model(frame, verbose=False)

            # Draw results
            for result in results:
                # Draw bounding boxes
                if result.boxes is not None:
                    for box in result.boxes:
                        x1, y1, x2, y2 = map(int, box.xyxy[0])
                        conf = float(box.conf[0])
                        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                        cv2.putText(frame, f"Person {conf:.2f}", (x1, y1 - 10),
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

                # Draw skeletons
                if result.keypoints is not None:
                    for kp in result.keypoints:
                        draw_skeleton(frame, kp)

            # FPS
            fps = 1.0 / (time.time() - start_time + 1e-9)
            person_count = len(results[0].boxes) if results[0].boxes is not None else 0
            cv2.putText(frame, f"FPS: {fps:.1f} | Persons: {person_count}",
                        (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
            cv2.putText(frame, "q: quit", (10, 60),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

            cv2.imshow("YOLOv8 Pose", frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            if cv2.getWindowProperty("YOLOv8 Pose", cv2.WND_PROP_VISIBLE) < 1:
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="YOLOv8 Pose Estimation Demo")
    parser.add_argument("-c", "--camera", type=int, default=0, help="Camera index (default: 0)")
    args = parser.parse_args()

    try:
        main(camera_index=args.camera)
    except Exception as e:
        print(f"Program error: {e}")
