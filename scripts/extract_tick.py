import pandas as pd
import os
import sys
from glob import glob
from datetime import datetime
from multiprocessing.pool import Pool

keyword = ['510050']
open_time = datetime.strptime("93000", "%H%M%S").time()
close_time = datetime.strptime("153000", "%H%M%S").time()


def process(fp: str):
    fpp, fn = os.path.split(fp)
    date = os.path.split(fpp)[-1]
    try:
        int(date)
    except:
        date = os.path.split(fpp)[-2]
    if not os.path.exists(out_path := os.path.join("data", date)):
        os.mkdir(out_path)
    out_path = os.path.join(out_path, "510050.csv")
    if os.path.exists(out_path):
        return

    print("process ", fp)
    df = pd.read_csv(fp, encoding="utf-8", dtype=str, header=None)
    df = df[df[0].isin(keyword)]

    dt = pd.to_datetime(df[1], format="%Y%m%d%H%M%S%f").dt.time
    df = df[(open_time <= dt) & (dt < close_time)]

    df.to_csv(out_path, encoding="utf-8", index=None, header=None)
    print("save to ", out_path)


if __name__ == '__main__':
    path = "/run/media/x2h1z/My Book/BaiduNetdiskDownload/LEVEL2_shanghai/2019"
    files = []
    for i in glob(os.path.join(path, "*")):
        date = os.path.split(i)[-1]
        try:
            datetime.strptime(date, "%Y%m%d")
        except:
            continue
        if not os.path.isdir(i) or date[:4] != "2019":
            continue
        if not os.path.exists(fp := os.path.join(i, "Tick.csv")):
            continue

        if os.path.isdir(fp):
            fp = os.path.join(fp, "Tick.csv")

        files.append(fp)

    for fp in files:
        if os.stat(fp).st_size > 0:
            process(fp)
    #
    # with Pool() as pool:
    #     for fp in files:
    #         pool.apply_async(process, args=(fp,))
    #     pool.close()
    #     pool.join()
