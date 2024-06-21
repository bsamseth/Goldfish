import matplotlib.pyplot as plt
import numpy as np

with open("tuner_log.csv", "r") as f:
    labels = f.readline().strip().split(",")

data = np.loadtxt("tuner_log.csv", delimiter=",", skiprows=1)

normalized = data[1:, :] / data[0, :]


for i in range(normalized.shape[1]):
    plt.plot(normalized[:, i], label=labels[i])

plt.legend()
plt.xlabel("Iteration")
plt.ylabel("Normalized Value")
plt.show()
