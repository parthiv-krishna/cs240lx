import scipy.io.wavfile as wav
import numpy as np


data = np.genfromtxt("dump.txt")
print(data.shape)

sample_rate = int(np.round(44100/(588008e-6)))
print(sample_rate)

wav.write("dump.wav", sample_rate, data)