#include <iostream>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include "check.hpp"

#define CONTROLLEN CMSG_LEN(sizeof(int))
#define CL_OPEN "open"
#define MAXLINE 4096

static struct cmsghdr *cmptr = NULL;

class Client{
    int _socket;

    int send_request(char *name, int oflag){
        pid_t pid;
        int len;
        char buf[10];
        struct iovec iov[3];
        static int fd[2] = {-1, -1};

        sprintf(buf, " %d", oflag);
        iov[0].iov_base = (void*)CL_OPEN;
        iov[0].iov_len = strlen(CL_OPEN) + 1;
        iov[1].iov_base = name;
        iov[1].iov_len = strlen(name);
        iov[2].iov_base = buf;
        iov[2].iov_len = strlen(buf) + 1;
        len = iov[0].iov_len + iov[1].iov_len + iov[2].iov_len;

        if(writev(_socket, &iov[0], 3) != len){
            return -1;
        }
        return recv_fd(write);
    }

public:
    Client(const char* name){
        sockaddr_un un;

        if(strlen(name) >= sizeof(un.sun_path)) errno = ENAMETOOLONG;

        _socket = check(socket(AF_UNIX, SOCK_STREAM, 0));

        memset(&un, 0, sizeof(un));
        un.sun_family = AF_UNIX;
        strcpy(un.sun_path, name);
        int len = offsetof(sockaddr_un, sun_path) + strlen(name);

        check(connect(_socket, (sockaddr*)&un, len));

        char line[4096];
        fgets(line, 4096, stdin);
        int fd_recv = send_request(line, O_RDONLY);

        std::cout<<fd_recv;
        close(_socket);
    }

    int recv_fd(ssize_t (*userfunc)(int, const void*, size_t)){
        int newfd, nr, status;
        char *ptr;
        char buf[MAXLINE];
        struct iovec iov[1];
        struct msghdr msg;

        status = -1;

        for(;;){
            iov[0].iov_base = buf;
            iov[0].iov_len = sizeof(buf);
            msg.msg_iov = iov;
            msg.msg_iovlen = 1;
            msg.msg_name = NULL;
            msg.msg_namelen = 0;
            if(cmptr == NULL && (cmptr = (struct cmsghdr*)malloc(CONTROLLEN)) == NULL) return -1;
            msg.msg_control = cmptr;
            msg.msg_controllen = CONTROLLEN;

            if((nr = recvmsg(_socket, &msg, 0)) < 0){
                return -1;
            }
            else{
                if(nr == 0) return -1;
            }

            for(ptr = buf; ptr <&buf[nr];){
                if(*ptr++ == 0){
                    if(ptr != &buf[nr - 1]) std::cout<<"Message format violation";
                    status = *ptr & 0xFF;

                    if(status == 0){
                        if(msg.msg_controllen < CONTROLLEN) std::cout<<"received code 0, but fd is missing";
                        newfd = *(int *) CMSG_DATA(cmptr);
                    }
                    else newfd = -status;
                    nr -= 2;
                }
            }

            if(nr > 0 && (*userfunc)(STDERR_FILENO, buf, nr) != nr) return -1;
            if(status >= 0) return(newfd);
        }
    }
};

int main(){
    Client client("Server");
}
