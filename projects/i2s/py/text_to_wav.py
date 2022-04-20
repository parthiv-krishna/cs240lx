import scipy.io.wavfile as wav
import numpy as np


data = np.genfromtxt("dump.txt")
print(data.shape)

wav.write("dump.wav", 44100, data)