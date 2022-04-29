import scipy.io.wavfile as wav
import numpy as np


data = np.genfromtxt("dump.txt", converters={0: lambda s: int(s, 16)})
print(data.shape)

data = data - 2**31
data = data.astype(np.int32)

# sample_rate = int(np.round(44100/(1000016e-6)))
print(np.min(data), np.max(data))
sample_rate = 44100
print(sample_rate)

wav.write("dump.wav", sample_rate, data)