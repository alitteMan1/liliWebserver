#include <iostream>

#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "webserver.h"
namespace thefool{
    void WebServer::start(){
        while(!stop_){
            THEFOOL_LOG_DEBUG(logger_) << "epoll_wait start:" <<epoll_fd_ ;
            int num = epoll_wait(epoll_fd_, epoll_events_, epoll_size_, -1);
            THEFOOL_LOG_DEBUG(logger_) << "epoll_wait end";
            if (num == -1){
                if(errno == EINTR){
                    continue;
                }
                else{
                    THEFOOL_LOG_DEBUG(logger_) << "epoll_wait error" ;
                    exit(1);
                }
            }
            for(int i = 0; i < num; i++){
                int fd = epoll_events_[i].data.fd;
                if (fd == listen_fd_){
                    while(true){
                       // struct sockaddr_in client_addr;
                       // socklen_t client_len = sizeof(client_addr);
                        int client_fd = accept(listen_fd_, NULL, NULL);
                        if (client_fd == -1){
                            if (errno == EAGAIN || errno == EWOULDBLOCK){
                                break;
                            }
                            else{
                                THEFOOL_LOG_DEBUG(logger_) << "accept error" << std::endl;
                                exit(1);
                            }
                        }
                        else{
                            std::cout << "new client: " << client_fd << std::endl;
                            int opt = fcntl(client_fd, F_GETFL, 0);
                            fcntl(client_fd, F_SETFL, opt | O_NONBLOCK);
                            threads_epoll_[round_robin_].liliadd(client_fd, EPOLLIN | EPOLLET | EPOLLHUP);
                            round_robin_ = (round_robin_ + 1) % thread_num_;
                            //注册listenfd到某个线程。
                            //TODO: 处理客户端连接请求
                        }
                    }
                }

            }
        }
        

    }


    // void WebServer::lilicreate(){
    //     epoll_fd_ = epoll_create(epoll_size_);
    //     if (epoll_fd_ == -1){
    //         THEFOOL_LOG_DEBUG(logger_) << "epoll_create error" << std::endl;
    //         exit(1);
    //     }
    // }

    // void WebServer::liliaddread(int fd) {
    //     struct epoll_event event;
    //     event.events = EPOLLIN | EPOLLET;
    //     event.data.fd = fd;
    //     if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1){
    //         THEFOOL_LOG_DEBUG(logger_) << "epoll_ctl error" << std::endl;
    //         exit(1);
    //     }
    // }

    // void WebServer::liliaddwrite(int fd){
    //     struct epoll_event event;
    //     event.events = EPOLLOUT | EPOLLET;
    //     event.data.fd = fd;
    //     if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)){
    //         THEFOOL_LOG_DEBUG(logger_) << "epoll_ctl error" << std::endl;
    //         exit(1);
    //     }
    // }

    // void WebServer::lilidel(int fd){
    //     if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, 0)){
    //         THEFOOL_LOG_DEBUG(logger_) << "epoll_ctl error" << std::endl;
    //         exit(1);
    //     }
    // }

    void WebServer::init(){
        threads_epoll_.reserve(thread_num_);
        threads_epoll_fd_.reserve(thread_num_);
        for(int i = 0; i < thread_num_; i++){
            threads_epoll_.emplace_back(thread_epoll_size_,logger_);
            threads_epoll_fd_.emplace_back(threads_epoll_[i].lilicreate());
            THEFOOL_LOG_DEBUG(logger_) << "thread " << i << " epoll_fd: " << threads_epoll_fd_[i] << std::endl;
        }

        init_epoll(); 
        init_socket();
        init_threadpool();
        

    }

    void WebServer::init_socket(){
                
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ == -1){
            THEFOOL_LOG_DEBUG(logger_) << "socket error" << std::endl;
            exit(1);
        }

        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) == -1){
            THEFOOL_LOG_DEBUG(logger_) << "bind error" << std::endl;
            exit(1);
        }
        if (listen(listen_fd_, 10) == -1){
            THEFOOL_LOG_DEBUG(logger_) << "listen error" << std::endl;
            exit(1);
        }

        //设置非阻塞
        int flags = fcntl(listen_fd_, F_GETFL, 0);
        if(fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK) == -1){
            THEFOOL_LOG_DEBUG(logger_) << "fcntl error" << std::endl;
            exit(1);
        }
        epoll_.liliaddread(listen_fd_);
    

    }
    
    void WebServer::init_epoll(){
        
        epoll_fd_ = epoll_.lilicreate();
       // epoll_fd_ = epoll_.get_epoll_fd();
        //lilicreate();

        //init 子线程epoll
        for(int i = 0; i < thread_num_; i++){
            threads_epoll_fd_[i] = threads_epoll_[i].lilicreate();
        }
        
    }

    void WebServer::init_threadpool(){
        THEFOOL_LOG_DEBUG(logger_) << "init threadpool test" ;
        for(int i = 0; i < thread_num_; i++){
            std::thread t(&WebServer::threadloop, this, threads_epoll_[i]);
            t.detach();
            THEFOOL_LOG_DEBUG(logger_) << "init threadpool thread " << i << " end" ;
        }
       // std::thread t(&WebServer::threadloop, this);
       THEFOOL_LOG_DEBUG(logger_) << "init threadpool end" ;
        
    }

    void WebServer::threadloop(Liliepoll epoll_){

        while(true){
            struct epoll_event events[MAX_EVENTS];
            int n = epoll_wait(epoll_.get_epoll_fd(), events, MAX_EVENTS, -1);
            if (n == -1){
                //系统调用被信号中断而返回
                if (errno == EINTR){
                    continue;
                }
                else{
                    THEFOOL_LOG_DEBUG(logger_) << "epoll_wait error" << std::endl;
                    exit(1);
                }
            }
                         
            for(int i = 0; i < n; i++){
                       
                int fd = events[i].data.fd;
                int e = events[i].events;
                if (e & EPOLLIN){
                    //TODO: 处理读事件
                    while(true){
                        char buf[1024];
                        int n = recv(fd, buf, sizeof(buf), 0);
                        if (n == -1){
                            if (errno == EAGAIN || errno == EWOULDBLOCK){
                                break;
                            }
                            else{
                                THEFOOL_LOG_DEBUG(logger_) << "recv error" << std::endl;
                                exit(1);
                            }
                        }
                        else if (n == 0){
                            //客户端关闭连接

                            //epoll_.liliaddwrite(fd);
                            //epoll_.liliaddread(fd);
                            close(fd);
                            break;
                        }
                        else{
                            //TODO: 处理请求
                            //std::cout << "recv: " << buf << std::endl;
                            //TODO: 处理请求
                            //epoll_.liliaddwrite(fd);
                            //epoll_.liliaddread(fd);
                            //break;
                            write(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello, world!</h1></body></html>\n", 94);

                        }
                    
                    }
                }
            }

        }

    }
}