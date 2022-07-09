#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "check.hpp"

#define CONTROLLEN CMSG_LEN(sizeof(int))

static struct cmsghdr *cmptr = NULL;

int oflag;
char *pathname;

int buf_args(char *buf, int (*optfunc)(int, char**)){
    char *ptr, *argv[50];
    int argc;

    if(strtok(buf, " \t\n") == NULL) return -1;
    argv[argc = 0] = buf;
    while((ptr = strtok(buf, " \t\n")) != NULL){
        if(++argc >= 49) return -1;
        argv[argc] = ptr;
    }
    argv[++argc] = NULL;
    return (*optfunc)(argc, argv);
}

int cli_args(int argc, char **argv){
    if(argc != 3 || strcmp(argv[0], "open") != 0){
        return -1;
    }
    pathname = argv[1];
    oflag = atoi(argv[2]);
    return 0;
}

class Server{
    int _listen_socket;
    void handle_request(char *buf, int nread, int fd){
        int new_fd;
        if(buf_args(buf, cli_args) < 0) return;
        if((new_fd = open(pathname, oflag)) < 0) {
            printf("Can nor open file %s.", pathname);
            return;
        }
        if(send_fd(new_fd) < 0) printf("Error");
        close(new_fd);
    }
public:
    Server(const char* name){
        struct sockaddr_un un;
        if(strlen(name) >= sizeof(un.sun_path)) errno = ENAMETOOLONG;
        _listen_socket = check(socket(AF_UNIX, SOCK_STREAM, 0));

        unlink(name);

        memset(&un, 0, sizeof(un));
        un.sun_family = AF_UNIX;
        strcpy(un.sun_path, name);
        int len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

        check(bind(_listen_socket, (sockaddr*)&un, len));
        check(listen(_listen_socket, 1));
        std::cout<<"Listening client ...";
    }

    void server_accept(){
        sockaddr_un un;
        int nread;
        char buf[4096];
        socklen_t len = sizeof(un);
        int sock_fd = check(accept(_listen_socket, (sockaddr*)&un, &len));
        std::cout << "Connect from: " << un.sun_path<<std::endl;

        for(;;){
            if((nread = read(STDIN_FILENO, buf, 4096)) < 0) printf("Error");
            else{
                if(nread == 0) break;
                handle_request(buf, nread, STDOUT_FILENO);
            }
        }

        close(sock_fd);
    }

    int send_fd(int fd_to_send){
        struct iovec iov[1];
        struct msghdr msg;
        char buf[2];

        iov[0].iov_base = buf;
        iov[0].iov_len = 2;
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;

        if(fd_to_send < 0) {
            msg.msg_control = NULL;
            msg.msg_controllen = 0;
            buf[1] = -fd_to_send;
            if (buf[1] == 0) buf[1] = 1;
        }
        else{
            if(cmptr == NULL && (cmptr = (struct cmsghdr*)malloc(CONTROLLEN)) == NULL) return -1;

            cmptr->cmsg_level = SOL_SOCKET;
            cmptr->cmsg_type = SCM_RIGHTS;
            cmptr->cmsg_len = CONTROLLEN;
            msg.msg_control = cmptr;
            msg.msg_controllen = CONTROLLEN;
            *(int*) CMSG_DATA(cmptr) = fd_to_send;
            buf[1] = 0;
        }

        buf[0] = 0;
        if(sendmsg(_listen_socket, &msg, 0) != 2) return -1;
        return 0;

    }
};

int main(){
    Server server("Server");
    server.server_accept();
}
