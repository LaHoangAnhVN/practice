#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include "Common.h"
#include "check.hpp"
#include <sys/un.h>

class server{
    int _listening_socket;
    void Listen(int sock_fd, sockaddr_in addr){
        std::cout<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<std::endl;
        int user_id;
        if(!try_recv(sock_fd, user_id)) return;
        std::cout<<"Get UID:"<<user_id<<std::endl;

        Request request;
        if(!try_recv(sock_fd, request)) return;
        sec_open(request.name, O_APPEND);
        std::cout<<"File opened"<<std::endl;
    }
public:
    server(const char* name){
        struct sockaddr_un un;
        if(strlen(name) >= sizeof(un.sun_path)){
            errno = ENAMETOOLONG;
            //return(-1);
        }

        _listening_socket = check(socket(AF_UNIX, SOCK_STREAM, 0));
        unlink(name);

        memset(&un, 0, sizeof(un));
        un.sun_family = AF_UNIX;
        strcpy(un.sun_path, name);
        int len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

        check(bind(_listening_socket, (struct sockaddr*)&un, len));
        /*_listening_socket = check(socket(AF_UNIX, SOCK_STREAM, 0));
        sockaddr_in addr{};
        addr.sin_family = AF_UNIX;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        check(bind(_listening_socket, (sockaddr*)&addr, sizeof(addr)));*/
    }

    int start(){
        check(listen(_listening_socket, 1));
        sockaddr_un un{};
        socklen_t len = sizeof(un);

        struct stat statbuff;
        char* name;
        name = (char*)malloc(sizeof(un.sun_path)+1);

        int clifd = check(accept(_listening_socket, (struct sockaddr*)&un, &len));

        len -= offsetof(struct sockaddr_un, sun_path);
        memcpy(name, un.sun_path, len);
        name[len] = 0;

        if(stat(name, &statbuff) < 0) return -3;

#ifdef S_ISSOCK
        if(S_ISSOCK(statbuff.st_mode) == 0) return -4;
#endif

        if((statbuff.st_mode & (S_IRWXG | S_IRWXO)) || (statbuff.st_mode & S_IRWXU) != S_IRWXU) return -5;

        time_t staletime = time(NULL) - 30;

        if(statbuff.st_atime < staletime || statbuff.st_ctime < staletime || statbuff.st_mtime < staletime) return -6;

        uid_t uid_ptr = statbuff.st_uid;

        return clifd;
        /*while(true){
            int sock_fd = check(accept(_listening_socket, (sockaddr*)&addr, &len));
            std::cout<<"Connect from: "<<inet_ntoa(addr.sin_addr)<<std::endl;
            Listen(sock_fd, addr);
            close(sock_fd);
            std::cout<<"Disconnect from: "<<inet_ntoa(addr.sin_addr)<<std::endl;
        }*/
    }
};

int main(){
    server ser("Server");
    ser.start();
}