#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64
#include <fuse.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
int fd;
fuse_operations x;
int attr(const char *path, struct stat * st) {
//write(fd,"called\n",6);
cout << "attr " << path << endl;
return -1;
}
int main(int argc, char ** argv) {
fd=open("log",O_RDWR|O_CREAT);
x.getattr=attr;
fuse_main(argc, argv,&x, NULL);
return 0;
}
