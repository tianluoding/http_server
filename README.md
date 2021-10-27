# http_server

参考 https://github.com/qinguoyi/TinyWebServer 和 《Linux高性能服务器编程》游双

C++实现http服务器，处理GET POST请求

* **线程池**和**epoll(ET模式)**实现one reactor+多线程事件处理模型
* 使用状态机解析http请求
* 使用基于时间轮定时器处理非活动连接
* 实现同步/异步日志系统，记录服务器运行
* 添加前端代码，如登录、注册功能，请求canvas+js实现的小游戏、图片、视频等

## docker安装mysql
* 拉取mysql镜像`sudo docker pull mysql`
* 运行mysql镜像`sudo docker run --name XXXX -p PORT:PORT -e MYSQL_ROOT_PASSWORD=XXXXXX -d mysql:latest`
* 开启容器`sudo docker start NAME`
* 进入容器`sudo docker exec -it NAME /bin/bash`
* mysql登录`mysql -u root -p`

## 后续工作
* 压测
* 实现one thread one loop模型
