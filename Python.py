import cv2
import mediapipe as mp
import numpy as np
from cvzone.PIDModule import PID
from cvzone.SerialModule import SerialObject

mp_drawing = mp.solutions.drawing_utils
mp_pose = mp.solutions.pose

cap = cv2.VideoCapture(0)
arduino = SerialObject(digits=3)

def make_240p():
    cap.set(3, 360)
    cap.set(4, 240)

make_240p()
    
xPID = PID([0.033, 0, 0.02], 640 // 2, axis=0)
yPID = PID([0.03, 0, 0.02], 480 // 2 - 40, axis=1)

xAngle, yAngle = 90, 90
human_detected = 0

# Setup mediapipe instance
with mp_pose.Pose(min_detection_confidence=0.5, min_tracking_confidence=0.5) as pose:
    while cap.isOpened():
        ret, frame = cap.read()

        # Recolor image to RGB
        image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        image.flags.writeable = False

        # Make detection
        results = pose.process(image)

        # Recolor back to BGR
        image.flags.writeable = True
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

        # Extract landmarks
        try:
            landmarks = results.pose_landmarks.landmark
            left_shoulder = [landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].visibility,
                             landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].x,
                             landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y]
            right_shoulder = [landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].visibility,
                              landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].x,
                              landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].y]

            center_x = (landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].x +
                        landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].x) / 2
            center_y = (landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].y +
                        landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y) / 2
            center_z = (landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].visibility +
                        landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].visibility) / 2
            center_chest = [center_z, center_x, center_y]

            a = np.array(center_chest)

            if a[0] > 0.8:
                human_detected = 1

                cx = a[1]  # cx = x
                cy = a[2]  # cy = y

                cx_converted = int(np.interp(cx, [0, 1], [0, 640]))
                cy_converted = int(np.interp(cy, [0, 1], [0, 480]))

                resultX = int(xPID.update(cx_converted))
                resultY = int(yPID.update(cy_converted))
                xAngle -= resultX
                yAngle += resultY

                image = xPID.draw(image, [cx_converted, cy_converted])
                image = yPID.draw(image, [cx_converted, cy_converted])

        except:
            human_detected = 0
            pass

        # Render detections
        print(human_detected)
        arduino.sendData([xAngle, yAngle, human_detected])
        cv2.imshow('Mediapipe Feed', image)

        if cv2.waitKey(10) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
