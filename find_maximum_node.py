f = open("web-Google.txt", "r")

max = -1

for line in f:
    line = line.strip()
    line = line.split()
    left = int(line[0])
    right = int(line[1])
    if left > max:
        max = left
    elif right > max:
        max = right
f.close()

print(max)