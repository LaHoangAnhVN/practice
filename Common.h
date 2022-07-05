#include <sys/socket.h>
#include "check.hpp"
#include "Library.h"

constexpr int PORT = 60001;

struct Request{
    enum Type{
        REQ_OPEN,
        REQ_UNLINK,
        REQ_GET_RIGHTS,
        REQ_SET_RIGHTS,
    };

    Type request_type;
    int target_uid;
    char name[256];
    mode_t mode;
    right_t rights;
};

inline bool try_send(int fd, const Request& request){
    errno = 0;
    int size = check_except(send(fd, &request, sizeof(Request), 0), EPIPE);
    return size>0;
}


inline bool try_recv(int fd, Request& request){
    errno = 0;
    int size = check_except(recv(fd, &request, sizeof(Request), MSG_WAITALL), EPIPE);
    return  size > 0;
}

inline bool try_send(int fd, const int& request){
    errno = 0;
    int size = check_except(send(fd, &request, sizeof(int), 0), EPIPE);
    return size>0;
}


inline bool try_recv(int fd, int& request){
    errno = 0;
    int size = check_except(recv(fd, &request, sizeof(int), MSG_WAITALL), EPIPE);
    return  size > 0;
}
