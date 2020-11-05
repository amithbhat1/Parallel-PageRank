import sys

file_path = sys.argv[1]

f = open(file_path, "r")

times = []

for line in f.readlines():
    line = line.strip()
    line = line.split()
    times.append(float(line[0]))
sum_time = 0

num_times = len(times)

for t in times:
    sum_time += t

avg_time = sum_time / num_times

print(f"average time of {num_times} entries: {avg_time}")

f.close()