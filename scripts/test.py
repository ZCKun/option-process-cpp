import os
from glob import glob

for month in glob(os.path.join("/run/media/x2h1z/My Book/BaiduNetdiskDownload/LEVEL2_shanghai/2020/2020", "*")):
    if not os.path.isdir(month):
        continue
    month = month.replace(" ", "\ ")
    date = os.path.split(month)[-1]
    f = os.path.join(month, "510050.csv")
    out_path = os.path.join("data", date)
    if not os.path.exists(out_path):
        os.mkdir(out_path)
        
    os.system(f"mv {f} {out_path}")
