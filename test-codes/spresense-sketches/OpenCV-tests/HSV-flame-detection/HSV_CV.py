import cv2
import numpy as np

video = cv2.VideoCapture("/Users/yunosuke/Documents/DroneProject/TestCode/HSVFlame/out.mp4")
imshow = True

while imshow:
    ret, frame = video.read()
    blur = cv2.GaussianBlur(frame,(5,5),0)
    
    lower = [18,50,50]
    upper = [35,255,255]
    blur = cv2.GaussianBlur(frame,(11,11),0)
    hsv = cv2.cvtColor(blur, cv2.COLOR_BGR2HSV)

    lower = np.array(lower,dtype='uint8')
    upper = np.array(upper,dtype='uint8')
    mask = cv2.inRange(hsv,lower,upper)

    output = cv2.bitwise_and(frame,hsv,mask=mask)

    cv2.imshow("Output", output)

    if cv2.waitKey(100) & 0xFF == ord("q"):
        imshow = False

cv2.destroyAllWindows()
video.release()



