import cv2

# Open the default camera (usually /dev/video0)
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Cannot open camera")
    exit()

while True:
    # Capture frame
    ret, frame = cap.read()

    if not ret:
        print("Error: Cannot read frame")
        break

    # Display the image
    cv2.imshow("USB Camera", frame)

    # Press 'q' to quit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release resources
cap.release()
cv2.destroyAllWindows()
