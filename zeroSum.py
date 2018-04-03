import matplotlib.pyplot as plt
import numpy as np
import math

Depth = 6

t = np.linspace(0,127,128)
x = np.zeros(np.shape(t))
y = np.zeros(np.shape(t))
z = np.zeros(np.shape(t))

print(np.shape(x))
print(np.shape(t))

for i in range(len(t)):
    x[i] = (i**Depth)/(127**(Depth-1))
    y[i] = (math.log(i+1)/math.log(127))*127
    z[i] = i


plt.subplot(211)
plt.plot(t,x)
plt.plot(t,y)
plt.plot(t,z)
plt.show()
