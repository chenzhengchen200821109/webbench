webbench is a very simple tool for benchmarking WWW or proxy servers.
Its implementation is short and simple. It used multi-process model.  
  
bench/ is another webbench tool using coroutine model, which is based on Tecent's libco.

# 简介  
bench/libcox是一个基于[libevent]开发的现代化C++11高性能网络服务器，自带TCP/UDP/HTTP等协议的异步非阻塞式的服务器和客户端库。

# 特性

1. 现代版的C++11接口
1. 非阻塞异步接口都是C++11的functional/bind形式的回调仿函数（不是[libevent]中的C风格的函数指针）
1. CPU多核友好和线程安全
1. 非阻塞纯异步多线程TCP服务器/客户端
1. 非阻塞纯异步多线程HTTP服务器/客户端
1. 非阻塞纯异步多线程UDP服务器
1. 支持多进程模式

# 项目由来  
开发过程中，接口设计方面基本上大部分是参考[muduo]项目来设计和实现的，当然也做了一些取舍和增改；同时也大量借鉴了[Golang]的一些设计哲学和思想。下面举几个小例子来说明一下：

1. `Duration` ： 这是一个时间区间相关的类，自带时间单位信息，参考了[Golang]项目中的`Duration`实现。我们在其他项目中见到太多的时间是不带单位的，例如`timeout`，到底是秒、毫秒还是微秒？需要看文档说明或具体实现，好一点的设计会将单位带在变量名中，例如`timeout_ms`，但还是没有`Duration`这种独立的类好。目前C++11中也有类似的实现`std::chrono::duration`，但稍显复杂，没有咱们这个借鉴[Golang]实现的版本来的简单明了
2. `Buffer` ： 这是一个缓冲区类，融合了`muduo`和[Golang]两个项目中相关类的设计和实现
3. `http::Server` : 这是一个http服务器类，自带线程池，它的事件循环和工作线程调度，完全是线程安全的，业务层不用太多关心跨线程调用问题。同时，还将http服务器的核心功能单独抽取出来形成`http::Service`类，是一个可嵌入型的服务器类，可以嵌入到已有的[libevent]事件循环中
4. 网络地址的表达就仅仅使用`"ip:port"`这种形式字符串表示，就是参考[Golang]的设计
5. `httpc::ConnPool`是一个http的客户端连接池库，设计上尽量考虑高性能和复用。以后基于此还可以增加负载均衡和故障转移等特性

另外，我们实现过程中极其重视线程安全问题，一个事件相关的资源必须在其所属的`EventLoop`（每个`EventLoop`绑定一个线程）中初始化和析构释放，这样我们能最大限度的减少出错的可能。为了达到这个目标我们重载`event_add`和`event_del`等函数，每一次调用`event_add`，就在对应的线程私有数据中记录该对象，在调用`event_del`时，检查之前该线程私有数据中是否拥有该对象，然后在整个程序退出前，再完整的检查所有线程的私有数据，看看是否仍然有对象没有析构释放。具体实现稍有区别，详细代码实现可以参考 
我们如此苛刻的追求线程安全，只是为了让一个程序能**安静的平稳的退出或Reload**，因为我们深刻的理解“编写永远运行的系统，和编写运行一段时间后平静关闭的系统是两码事”，后者要困难的多得多。
