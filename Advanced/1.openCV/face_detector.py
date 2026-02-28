import cv2
import argparse
import os


class FaceDetector:
    def __init__(
        self,
        camera_index=0,
        model_path=None,
        config_path=None,
        conf_threshold=0.6,
        input_size=300,
        backend=cv2.dnn.DNN_BACKEND_DEFAULT,
        target=cv2.dnn.DNN_TARGET_CPU,
    ):
        self.cap = cv2.VideoCapture(camera_index)
        if not self.cap.isOpened():
            raise RuntimeError("Failed to open the camera")

        self.conf_threshold = conf_threshold
        self.input_size = input_size

        if model_path is None or config_path is None:
            raise RuntimeError(
                "DNN model files not set. Provide --model and --config paths."
            )

        self.net = cv2.dnn.readNetFromCaffe(config_path, model_path)
        self.net.setPreferableBackend(backend)
        self.net.setPreferableTarget(target)

        cv2.namedWindow("Face Detection", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Face Detection", 640, 480)

    def detect(self, frame):
        h, w = frame.shape[:2]
        blob = cv2.dnn.blobFromImage(
            cv2.resize(frame, (self.input_size, self.input_size)),
            1.0,
            (self.input_size, self.input_size),
            (104.0, 177.0, 123.0),
        )
        self.net.setInput(blob)
        detections = self.net.forward()

        face_count = 0
        for i in range(detections.shape[2]):
            confidence = float(detections[0, 0, i, 2])
            if confidence < self.conf_threshold:
                continue

            box = detections[0, 0, i, 3:7] * [w, h, w, h]
            (x1, y1, x2, y2) = box.astype("int")
            x1 = max(0, x1)
            y1 = max(0, y1)
            x2 = min(w - 1, x2)
            y2 = min(h - 1, y2)

            face_count += 1
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(
                frame,
                f"Face {confidence:.2f}",
                (x1, y1 - 8),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 0),
                2,
            )

        info = f"Faces: {face_count} | DNN | conf>={self.conf_threshold:.2f}"
        cv2.putText(
            frame,
            info,
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (255, 255, 255),
            2,
        )
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
                if key == ord("q"):
                    break

                if cv2.getWindowProperty("Face Detection", cv2.WND_PROP_VISIBLE) < 1:
                    break
        finally:
            self.cap.release()
            cv2.destroyAllWindows()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="OpenCV DNN Face Detection Demo")
    parser.add_argument("-c", "--camera", type=int, default=0, help="Camera index (default: 0)")
    parser.add_argument(
        "--model",
        type=str,
        required=False,
        default="models/res10_300x300_ssd_iter_140000.caffemodel",
        help="Path to Caffe .caffemodel",
    )
    parser.add_argument(
        "--config",
        type=str,
        required=False,
        default="models/deploy.prototxt",
        help="Path to Caffe .prototxt",
    )
    parser.add_argument(
        "--conf",
        type=float,
        default=0.6,
        help="Confidence threshold (default: 0.6)",
    )
    parser.add_argument(
        "--size",
        type=int,
        default=300,
        help="Input size for DNN (default: 300)",
    )
    args = parser.parse_args()

    model_path = os.path.abspath(args.model)
    config_path = os.path.abspath(args.config)

    if not os.path.isfile(model_path) or not os.path.isfile(config_path):
        raise RuntimeError(
            "Model files not found. Please place the Caffe model and config at:"
            f"\n  {model_path}\n  {config_path}"
        )

    try:
        detector = FaceDetector(
            camera_index=args.camera,
            model_path=model_path,
            config_path=config_path,
            conf_threshold=args.conf,
            input_size=args.size,
        )
        detector.run()
    except Exception as e:
        print(f"Program error: {e}")
