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
  使用Apachebench工具
  

`ab -n 10000 -c 100 URL`

测试10000个请求，100并发的情况，对应的服务器配置，在VBOX下6G内存+4核CPU，在本机运行测试程序，测试结果如下

```shell
Server Software:
Server Hostname:        ****
Server Port:            ****

Document Path:          /
Document Length:        1131 bytes

Concurrency Level:      100
Time taken for tests:   119.681 seconds
Complete requests:      10000
Failed requests:        0
Total transferred:      11890000 bytes
HTML transferred:       11310000 bytes
Requests per second:    83.56 [#/sec] (mean)
Time per request:       1196.808 [ms] (mean)
Time per request:       11.968 [ms] (mean, across all concurrent requests)
Transfer rate:          97.02 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   11  97.6      1    1015
Processing:     6 1176 1295.6    546    7227
Waiting:        1  814 1063.4    212    5327
Total:          7 1187 1303.6   1101    7228

Percentage of the requests served within a certain time (ms)
  50%   1101
  66%   1263
  75%   1489
  80%   2234
  90%   3223
  95%   4226
  98%   5260
  99%   5336
 100%   7228 (longest request)
```

可以看到请求最长响应时间是7.2s，failed requests为0，RPS为83.56


* 实现one thread one loop模型
