import sys
import os
import pandas as pd

from datetime import datetime

codes = []
open_time = datetime.strptime("93000", "%H%M%S")
close_time = datetime.strptime("153000", "%H%M%S")


def process(fp):
    df = pd.read_csv(fp, dtype=str, header=None)
    if codes:
        df2 = df[df[0].isin(codes)]
    else:
        df2 = df
    dt = pd.to_datetime(df2[1], format="%Y%m%d%H%M%S")
    df2 = df2[(open_time.time() <= dt.dt.time) & (dt.dt.time <= close_time.time())]

    fp, fn = os.path.split(fp)[-2:]
    date = os.path.split(fp)[-1]
    name, s = os.path.splitext(fn)
    out_fn = f"{name}_out{s}"
    if not os.path.exists(out_path := os.path.join("data", date)):
        os.mkdir(out_path)
    out_fp = os.path.join(out_path, out_fn)
    df2.to_csv(out_fp, encoding="utf-8", header=None, index=None)
    print("save to ", out_fp)


if __name__ == '__main__':
    process(sys.argv[1])
