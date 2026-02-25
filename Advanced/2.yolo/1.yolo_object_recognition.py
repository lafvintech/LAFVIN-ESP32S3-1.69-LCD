import cv2
import time
import argparse
import numpy as np
from ultralytics import YOLO


# Generate distinct colors for each class
def generate_colors(n):
    colors = []
    for i in range(n):
        hue = int(180 * i / n)
        color = cv2.cvtColor(np.uint8([[[hue, 200, 255]]]), cv2.COLOR_HSV2BGR)[0][0]
        colors.append(tuple(int(c) for c in color))
    return colors


def draw_detections(frame, results, colors):
    """Draw bounding boxes with class-specific colors and labels"""
    count = 0
    for result in results:
        if result.boxes is None:
            continue
        for box in result.boxes:
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            conf = float(box.conf[0])
            cls_id = int(box.cls[0])
            color = colors[cls_id % len(colors)]
            label = f"{result.names[cls_id]} {conf:.2f}"

            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.55, 1)
            cv2.rectangle(frame, (x1, y1 - th - 8), (x1 + tw + 4, y1), color, -1)
            cv2.putText(frame, label, (x1 + 2, y1 - 4),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 1, cv2.LINE_AA)
            count += 1
    return count


def main(camera_index=0):
    cap = cv2.VideoCapture(camera_index)
    if not cap.isOpened():
        raise RuntimeError("Failed to open the camera")

    # Load YOLOv5s via ultralytics (auto-downloads ~14MB on first run)
    model = YOLO("yolov5su.pt")

    colors = generate_colors(80)

    cv2.namedWindow("YOLOv5 Detection", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("YOLOv5 Detection", 640, 480)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            start_time = time.time()

            results = model(frame, verbose=False, conf=0.25, iou=0.45)
            obj_count = draw_detections(frame, results, colors)

            fps = 1.0 / (time.time() - start_time + 1e-9)
            cv2.putText(frame, f"FPS: {fps:.1f} | Objects: {obj_count}",
                        (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
            cv2.putText(frame, "q: quit",
                        (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

            cv2.imshow("YOLOv5 Detection", frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            if cv2.getWindowProperty("YOLOv5 Detection", cv2.WND_PROP_VISIBLE) < 1:
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="YOLOv5 Object Detection Demo")
    parser.add_argument("-c", "--camera", type=int, default=0, help="Camera index (default: 0)")
    args = parser.parse_args()

    try:
        main(camera_index=args.camera)
    except Exception as e:
        print(f"Program error: {e}")
