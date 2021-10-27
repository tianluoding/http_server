# http_server

参考 https://github.com/qinguoyi/TinyWebServer 和 《Linux高性能服务器编程》游双

实现http服务器，处理GET POST请求

* **线程池**和**epoll(ET模式)**实现one reactor+多线程事件处理模型
* 使用状态机解析http请求
* 使用基于时间轮定时器处理非活动连接
* 实现同步/异步日志系统，记录服务器运行
* 添加前端代码，如登录、注册功能，请求canvas+js实现的小游戏、图片、视频等

## 后续工作
* 压测
* 实现one thread one loop模型
