import requests
import sys

from datetime import datetime, timedelta
from inspect import currentframe, getframeinfo


def __LINE__(): return getframeinfo(currentframe())


url = "https://sp0.baidu.com/8aQDcjqpAAV3otqbppnN2DJv/api.php"
headers = {
    "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36",
    "Host": "sp0.baidu.com",
    "Referer": "https://www.baidu.com/s?ie=utf-8&f=8&rsv_bp=1&rsv_idx=1&tn=baidu&wd=%E8%8A%82%E5%81%87%E6%97%A5&fenlei=256&oq=2018%25E8%258A%2582%25E5%2581%2587%25E6%2597%25A5&rsv_pq=8de302bd002cc498&rsv_t=2cc3aU63WSGiphFIBTZomft6x1holOlQtHEE639qO%2FeRw1PqiovxWbJnZIk&rqlang=cn&rsv_enter=1&rsv_dl=tb&rsv_sug3=2&rsv_sug2=0&rsv_btype=t&inputT=106&rsv_sug4=548",
    "Accept": "*/*",
    "Accpet-Encoding": "gzip, deflate, br",
    "Accept-Language": "zh-CN,zh;q=0.9",
}


def dt_generator(start, end):
    if not end:
        end = datetime.today()
    ret = [dt for i in range((end - start).days + 1) if (dt := end - timedelta(days=i)).weekday() not in [5, 6]]
    return ret


def fetch(dt):
    params = {
        "query": dt.strftime("%Y年%-m月"),
        "co": "",
        "resource_id": "39043",
        "ie": "utf8",
        "oe": "gbk",
        "format": "json",
        "tn": "wisetpl"
    }

    resp = requests.get(url, params=params, headers=headers)
    if resp.status_code != 200:
        print("request HTTP Response status code error: ", resp.status_code)
        return None
    return resp


def get_holiday(start_date: datetime, end_date: datetime) -> set:
    holidays = set()

    while True:
        if start_date > end_date:
            break

        resp = fetch(start_date)
        try:
            data = resp.json()
            data = data['data'][0]['almanac']

            for d in data:
                t = f"{d['year']}{'0' + d['month'] if len(d['month']) == 1 else d['month']}{'0' + d['day'] if len(d['day']) == 1 else d['day']}"
                if 'status' in d and d['status'] == '1':
                    holidays.add(t)

            start_date += timedelta(90)

        except IndexError:
            print(f'[jjr_debug:{__LINE__()}]{start_date}')
            print(f'[jjr_debug:{__LINE__()}]{resp.text}')
            print(f'[jjr_debug:{__LINE__()}]{resp.url}')
            break

    return holidays


if __name__ == '__main__':
    args = sys.argv
    if len(args) <= 1:
        print(f"Usage: python3 {args[0]} start_date end_date[option]")
        sys.exit(1)

    try:
        start_dt = datetime.strptime(args[1], "%Y-%m-%d")
    except:
        start_dt = datetime.strptime(args[1], "%Y%m%d")

    if len(args) <= 2:
        end_dt = datetime.today()
    else:
        try:
            end_dt = datetime.strptime(args[2], "%Y-%m-%d")
        except:
            end_dt = datetime.strptime(args[2], "%Y%m%d")

    holidays = get_holiday(start_dt, end_dt)

    print(f"{start_dt.strftime('%Y-%m-%d')} - {end_dt.strftime('%Y-%m-%d')} 一共 {len(holidays)} 天节假日:")

    print('["', end='')
    print('","'.join(holidays), end='')
    print('"]')

