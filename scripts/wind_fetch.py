import requests
import json
import pandas as pd

from datetime import datetime, timedelta
from concurrent.futures import ThreadPoolExecutor

url = "http://192.168.0.55:5000/optionchain"


def fetch_option_chain(us_code: str, date: datetime):
    d = date.strftime("%Y-%m-%d")
    params = {
        "us_code": us_code,
        "date": d,
        "cp": "全部",
        "month": "全部",
    }
    resp = requests.get(url, params=params)
    return resp.json()


def main():
    start = datetime.strptime("2018-01-01", "%Y-%m-%d")
    end = datetime.strptime("2019-01-01", "%Y-%m-%d")

    start_dt = start.strftime('%Y%m%d')
    end_dt = end.strftime('%Y%m%d')

    data = {}

    while start < end:
        if start.weekday() < 5:
            d = start.strftime("%Y%m%d")
            data[d] = {}
            print(f"fetch {d}")
            r = fetch_option_chain("510050.SH", start)
            df = pd.DataFrame(r)
            r = df.to_dict(orient="index")
            for k, v in r.items():
                cp = "put" if v['call_put'] == "认沽" else "call"
                if cp in data[d]:
                    data[d][cp].append(v)
                else:
                    data[d][cp] = [v]

        start += timedelta(days=1)

    with open(f"option_chains_{start_dt}_{end_dt}.json", "w", encoding="utf-8") as f:
        f.write(json.dumps(data))


if __name__ == '__main__':
    main()
