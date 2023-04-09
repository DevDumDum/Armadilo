import cv2
import numpy as np
import matplotlib.pyplot as plt
import cvlib as cv
from cvlib.object_detection import draw_bbox
from numpy.lib.polynomial import poly

img = cv2.imread('C:/xampp/htdocs/Armadilo/DSURV/crate.jpg')

box, label, c_score = cv.detect_common_objects(img)
output = draw_bbox(img, box, label, c_score)

output = cv2.cvtColor(output, cv2.COLOR_BGR2RGB)
plt.figure(figsize=(10, 10))
plt.axis("off")
plt.imshow(output) 
plt.show()