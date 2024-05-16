## ScrollingScreenshotTool
c++ homework

## git 代理设置
```bash
git config --global http.proxy http://127.0.0.1:7890
git config --global https.proxy https://127.0.0.1:7890
```
取消代理
```bash
git config --global --unset http.proxy
git config --global --unset https.proxy
```

## 如何编译？

![Screenshot of the application](readme/1.png)

![Screenshot of the application](readme/2.png)

![Screenshot of the application](readme/3.png)

```bash
cl /EHsc /Zi /MD /Fe:ScrollingScreenshotTool.exe ScrollingScreenshotTool.cpp user32.lib gdi32.lib gdiplus.lib ole32.lib
```

```bash
cl /EHsc demo\main1.cpp user32.lib
```