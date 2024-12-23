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
            int num = epoll_wait(epoll_fd_, epoll_events_, epoll_size_, -1);
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

                            //注册listenfd到某个线程。
                            //TODO: 处理客户端连接请求
                        }
                    }
                }
            }
        }
        

    }


    void WebServer::lilicreate(){
        epoll_fd_ = epoll_create(epoll_size_);
        if (epoll_fd_ == -1){
            THEFOOL_LOG_DEBUG(logger_) << "epoll_create error" << std::endl;
            exit(1);
        }
    }

    void WebServer::liliaddread(int fd) {
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1){
            THEFOOL_LOG_DEBUG(logger_) << "epoll_ctl error" << std::endl;
            exit(1);
        }
    }

    void WebServer::liliaddwrite(int fd){
        struct epoll_event event;
        event.events = EPOLLOUT | EPOLLET;
        event.data.fd = fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event)){
            THEFOOL_LOG_DEBUG(logger_) << "epoll_ctl error" << std::endl;
            exit(1);
        }
    }

    void WebServer::lilidel(int fd){
        if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, 0)){
            THEFOOL_LOG_DEBUG(logger_) << "epoll_ctl error" << std::endl;
            exit(1);
        }
    }

    void WebServer::init(){
        init_socket();
        init_epoll();
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

    

    }

    void WebServer::init_epoll(){
        lilicreate();
        liliaddread(listen_fd_);
    }

    void WebServer::init_threadpool(){
        THEFOOL_LOG_DEBUG(logger_) << "init threadpool test" << std::endl;
        
        std::thread t(&WebServer::threadloop, this);
        
    }
}