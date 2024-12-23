#pragma once
#include <stdint.h>
#include <sys/epoll.h>
#include "log.h"
int li_epoll_create(int size);
void li_epoll_del_event(int epfd, int fd, uint32_t events);
void li_epoll_add_event(int epfd, int fd, uint32_t events);
namespace thefool{
class Liliepoll {
    private:
        //epollfd
        int fd_;
        //epollfd size
        int size_;
        //logger
        Logger::ptr logger_;


    public:
        Liliepoll(): size_(0), fd_(0){};
        Liliepoll(int size, Logger::ptr logger)
        :size_(size),
        fd_(0){
            logger_ = logger;
        };
        int lilicreate();
        void liliadd(int fd, uint32_t events);
        void liliaddread(int fd);
        void liliaddwrite(int fd);
        void lilidel(int fd);
        int get_epoll_fd();

};
}