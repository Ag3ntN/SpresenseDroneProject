import cv2
import numpy as np

video = cv2.VideoCapture("/Users/yunosuke/Documents/DroneProject/TestCode/HSVFlame/out.mp4")
imshow = True

while imshow:
    ret, frame = video.read()
    if cv2.waitKey(100) & 0xFF == ord("q") or ret == False:
        imshow = False
        break
    
    lower = [18,50,50]
    upper = [35,255,255]
    blur = cv2.GaussianBlur(frame,(11,11),0)
    hsv = cv2.cvtColor(blur, cv2.COLOR_BGR2HSV)

    lower = np.array(lower,dtype='uint8')
    upper = np.array(upper,dtype='uint8')
    mask = cv2.inRange(hsv,lower,upper)

    hsvConv = cv2.bitwise_and(frame,hsv,mask=mask)
    gray = cv2.cvtColor(hsvConv,cv2.COLOR_BGR2GRAY)
    gray = np.float32(gray)
    dst = cv2.cornerHarris(gray, 2,3,0.04)
    #dst = cv2.dilate(dst,None)

    frame[dst>0.1*dst.max()]=[0,0,255]

    cv2.imshow("dst", frame)

cv2.destroyAllWindows()
video.release()



