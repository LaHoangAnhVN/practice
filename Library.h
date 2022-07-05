#include <cstdlib>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include<iostream>


enum right_t{
    R_READ = 1,
    R_WRITE = 2,
    R_DELETE = 4,
};

int sec_init();

int sec_open(const char* name, mode_t mode);
int sec_opentat(int uid, const char* name, mode_t mode);

int sec_unlink(const char* name);

int sec_inlinkat(int uid, const char* name);

int sec_close();

right_t sec_grant(int uid, const char* file, right_t rights);

right_t sec_rewoke(int uid, const char* file, right_t rights);

right_t sec_sec_check(int uid, const char* file, right_t rights);