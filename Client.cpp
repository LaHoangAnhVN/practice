#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/un.h>
#include "Common.h"

class Client{
    int _socket;
    void Send(){
        struct ucred cred;
        cred.uid = getuid();
        int buff = cred.uid;
        //std::cout<<"UID: " << buff;
        if(!try_send(_socket, buff)) return;

        Request request;
        request.target_uid = buff;
        request.mode = S_IRUSR;
        strcpy(request.name, "test.txt");
        request.rights = R_READ;
        if(!try_send(_socket, request)) return;
    }
public:
    void start(const char* name){
        sockaddr_un un, sun;

        if (strlen(name) >= sizeof(un.sun_path)) errno = ENAMETOOLONG;

        _socket = check(socket(AF_UNIX, SOCK_STREAM, 0));

        memset(&un, 0, sizeof(un));
        un.sun_family = AF_UNIX;
        sprintf(un.sun_path, "%s%05ld", "var/tmp", (long)getpid());
        int len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

        unlink(un.sun_path);
        check(bind(_socket, (struct sockaddr*)&un, len));

        chmod(un.sun_path, S_IRWXU);

        memset(&sun, 0, sizeof(sun));
        sun.sun_family = AF_UNIX;
        strcpy(sun.sun_path, name);
        len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
        check(connect(_socket, (struct sockaddr*)&sun, len));

        /*_socket = check(socket(AF_UNIX, SOCK_STREAM, 0));
        sockaddr_in addr{};
        addr.sin_family = AF_UNIX;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        check(connect(_socket, (sockaddr*)&addr, sizeof(addr)));

        Send();
        close(_socket);*/
    }
};

int main(){
    Client client;
    client.start("Server");
}