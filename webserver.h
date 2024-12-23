#include <iostream>
#include <thread>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <sys/epoll.h>

#include "log.h"
#include "liepoll.h"
#define MAX_EVENTS 1024

namespace thefool{
class WebServer{

public:

    WebServer() : epoll_fd_(-1),  port_(6000), thread_num_(5), epoll_size_(1024), logger_(std::make_shared<Logger>("WebServer")), epoll_(epoll_size_, logger_){
        logger_->add_appender(std::make_shared<StdoutLogAppender>());

    };

    WebServer(YAML::Node config);

    WebServer(int port, int thread_num):port_(port),thread_num_(thread_num){

    };

    void init();

    void init_socket();

    void init_epoll();

    void init_threadpool();

    void start();

    void stop(){};

   // void lilicreate();

   // void liliaddread(int fd);

   // void liliaddwrite(int fd);

   // void lilidel(int fd);

    void threadloop(Liliepoll epoll_);
    
    private:
        int epoll_fd_;

        struct epoll_event epoll_events_[MAX_EVENTS];

        int listen_fd_;

        int port_;

        int thread_num_;

        std::vector<int> threads_epoll_fd_;
        std::vector<Liliepoll> threads_epoll_;
        int thread_epoll_size_ = 10;
        Logger::ptr logger_;

        int epoll_size_;

        bool stop_;

        Liliepoll epoll_;

        //线程轮询
        int round_robin_ = 0;


};
}