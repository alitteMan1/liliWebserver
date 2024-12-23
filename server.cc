#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<sys/epoll.h>
//#include "liepoll.h"
#include "webserver.h"


#define MAX_CONNECTIONS 100
int main(){

    thefool::WebServer server;
    server.init();
    server.start();
    return 0;
}