# SmartMatrix Arduino

该项目为 SmartMatrix 像素时钟的 Arduino 端，基于B站UP主 @大聪明的二手脑袋 的 EasyMatrix 项目修改而来。

相比原版新增功能：

- 添加了蓝牙配置功能，可使用小程序调整亮度、时钟颜色、动画、对时等；

- 添加外置 DS1302 时钟模块，断电后也可以保持时间，不用每次启动时重新对时；

- 删除原版 AP 模式配网相关代码，减少不必要的占用。

- 另外还有其它有意思的隐藏功能，期待各位的发现。



该项目配套的小程序源码：[Frspble/SmartMatrix (github.com)](https://github.com/Frspble/SmartMatrix)