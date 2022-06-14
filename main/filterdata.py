from dateutil import parser
import sys

res = []
for frame in map(lambda str: str.split(' '), sys.stdin.readlines()):
    try:
        date, time, epoch, moteid, temperature, humidity, light, voltate = frame
        id = int(moteid)
        if id == 8:
            val = float(temperature)
            res.append((parser.parse(date+" "+time[:8]), val))
            #nmbr = f"{val}".replace(".", ",")
            
    except:
        continue

res.sort()

for date, val in res:
    print(f"{date};{str(val).replace('.',',')}")