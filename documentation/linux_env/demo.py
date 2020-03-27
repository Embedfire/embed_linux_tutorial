import json, requests

#输入地点
weather_place = "东莞"

#日期
date = []
#最高温与最低温
high_temp = []
low_temp = []
#天气
weather = []

# 请求天气信息
weather_url = "http://wthrcdn.etouch.cn/weather_mini?city=%s" % (weather_place)

response = requests.get(weather_url)
try:
    response.raise_for_status()
except:
    print("请求信息出错")
    
#将json文件格式导入成python的格式
weather_data = json.loads(response.text)

# 打印原始数据
# print(weather_data)
 
w = weather_data['data']

print("地点：%s" % w['city'])

#进行五天的天气遍历，并格式化输出
for i in range(len(w['forecast'])):
    date.append(w['forecast'][i]['date'])
    high_temp.append(w['forecast'][i]['high'])
    low_temp.append(w['forecast'][i]['low'])
    weather.append(w['forecast'][i]['type'])
    
    #输出
    print("日期：" + date[i])
    print("\t温度：最" + low_temp[i] + "\t最" + high_temp[i])
    print("\t天气：" + weather[i] + "\n")
    
print("\n今日着装：" + w['ganmao'])
print("当前温度：" + w['wendu'])
