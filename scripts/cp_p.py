import json
import os

from holiday import get_holiday
from datetime import datetime, timedelta


data = json.load(open("option_chains_20190101_20200101.json", encoding="utf-8"))

result = {}

for date, l1 in data.items():
    start_dt = datetime.strptime(date, "%Y%m%d")
    if start_dt.weekday() >= 5:
        continue

    try:
        end_dt = start_dt + timedelta(days=365) 
    except Exception as e:
        print("error:", date)
        raise e
    holidays = get_holiday(start_dt, end_dt)

    if date in holidays:
        continue

    result[date] = {}

    for cp, l2 in l1.items():
        result[date][cp] = []
        for item in l2:
            last_trade_date = datetime.strptime(item["last_tradedate"], "%a, %d %b %Y %H:%M:%S %Z")
            expire_date = 0
            start_dt = datetime.strptime(date, "%Y%m%d")

            while start_dt < last_trade_date:
                a = start_dt.strftime("%Y%m%d")
                if start_dt.weekday() < 5 and a not in holidays:
                    expire_date += 1
                start_dt += timedelta(days=1)

            item['expiredate_t'] = expire_date
            result[date][cp].append(item)
            print(f"{item['option_code']} in {date}-{last_trade_date.strftime('%Y%m%d')} expire date: {expire_date}")


with open("option_chains_2020.json", "w", encoding="utf-8") as f:
    f.write(json.dumps(result))


