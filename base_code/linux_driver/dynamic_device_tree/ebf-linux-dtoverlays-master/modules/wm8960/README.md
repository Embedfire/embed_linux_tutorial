## 安装alsa工具
```bash
sudo apt install alsa-utils
```
## 配置声卡

```bash
alsactl restore -f asound.state
```

若配置声卡失败，则会出现
```bash
pcm_write:2053:write error: Input/output error

```
## 播放音乐测试
```
aplay -D plughw:0,0 demo.wav
```
aplay只可以测试wav格式的音乐文件

