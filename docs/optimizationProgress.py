from matplotlib import pyplot as plt


stockfishNPS = 190_717_779
pythonChessNPS = 91_843
ourPerformance = [
    ("Starting point", 5_980_028),
    ("replace std::vector with MoveList", 12_245_777)
]
labels, values = zip(*ourPerformance)

# Plot
plt.figure(figsize=(8, 5))
plt.yscale('log')
plt.bar(labels, values, color='royalblue', label='Our Performance')
plt.axhline(y=stockfishNPS, color='red', linestyle='--', label='Stockfish')
plt.axhline(y=pythonChessNPS, color='green', linestyle='--', label='python-chess')

# Labels and title
plt.ylabel("Nodes Per Second (NPS)")
plt.title("Performance Comparison")
plt.legend()
plt.grid(axis='y', linestyle='--', alpha=0.7)

# Show plot
plt.savefig("performanceProgress.png")
