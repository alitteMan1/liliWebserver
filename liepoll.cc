#include <iostream>
#include <sys/epoll.h>
#include "liepoll.h"
#include "log.h"

// int li_epoll_create( int size ) {
//     int epfd = epoll_create(size);
//     if (epfd == -1) {
//         std::cerr << "epoll_create error" << std::endl;
//         exit(1);
//     }
//     return epfd;
// }

// void li_epoll_add_event(int epfd, int fd, uint32_t events) {
//     struct epoll_event event;
//     event.events = events;
//     event.data.fd = fd;
//     if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1){
//         std::cerr << "li_epoll_add_event: epoll_ctl error" << std::endl;
//         exit(1);
//     }
// }

// void li_epoll_del_event(int epfd, int fd, uint32_t events) {
//     struct epoll_event event;
//     event.events = events;
//     event.data.fd = fd;
//     if(epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&event) == -1){
//         std::cerr << "li_epoll_del_event: epoll_ctl error" << std::endl;
//         exit(1);
//     }
// }


namespace thefool{
//static auto logger = std::make_shared<thefool::Logger>();

        int Liliepoll::lilicreate(){
            fd_ = epoll_create(size_);
            if(fd_ == -1){
                THEFOOL_LOG_DEBUG(logger_)<< "epoll_create error" << std::endl;
                exit(1);
            }
            THEFOOL_LOG_DEBUG(logger_)<< "epoll_create success"<<"fd_="<<fd_;
            return fd_;
        }

        void Liliepoll::liliadd(int fd,uint32_t events){
            struct epoll_event event;
            event.events = events;
            event.data.fd = fd;
            if(epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event) == -1){
                THEFOOL_LOG_DEBUG(logger_)<< "epoll_ctl error" << std::endl;
                exit(1);
            }
        }

        void Liliepoll::liliaddread(int fd){
            struct epoll_event event;
            event.events = EPOLLIN  | EPOLLET |  EPOLLHUP;
            event.data.fd = fd;
            if(epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event)){
                THEFOOL_LOG_DEBUG(logger_)<< "epoll_ctl error" << std::endl;
                exit(1);
            }
        }

        void Liliepoll::liliaddwrite(int fd){
            struct epoll_event event;
            event.events = EPOLLOUT | EPOLLET ;
            event.data.fd = fd;
            if(epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event)){
                THEFOOL_LOG_DEBUG(logger_)<< "epoll_ctl error" << std::endl;
                exit(1);
            }
        }

        void Liliepoll::lilidel(int fd){
            if(-1 == epoll_ctl(fd_, EPOLL_CTL_DEL, fd, 0)){
                THEFOOL_LOG_DEBUG(logger_)<< "epoll_ctl error" << std::endl;
                exit(1);
            }
        }
        int Liliepoll::get_epoll_fd(){
            return fd_;
        }
   
}