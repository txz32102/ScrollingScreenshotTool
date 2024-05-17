## task 1
现在有一个数据流
```c++
std::vector<BYTE> screenshotData = CaptureScreenshot(startPoint, endPoint);
```
如上函数，读了起始点和结束点位置，返回一个BYTE的数据结构
写一个函数把它转换成各种格式的图片，我认为这个函数应该是这样的
```c++
void SaveImage(std::vector<BYTE> data, string path, string format)
```
就是把data读入， 转换为给定format，比如png之后保存在path里面，请不要用opencv！opencv太冗余了
见文件`demo/need_to_write.cpp`,修改从`215`行开始！请将修改的文件命名为`demo/Done1.cpp`