#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64
#include <fuse.h>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
using namespace std;
char * test_file=NULL;
long SIZE=1024L*1024*1024*16;
//int fd;
fuse_operations x;
time_t now;
int attr(const char *path, struct stat * st) {
//write(fd,"called\n",6);
cout << "attr " << path << endl;
if (string(path)=="/") {
  st->st_mode=0444|S_IFDIR;
  st->st_nlink=1;
  st->st_uid=0;
  st->st_gid=0;
  st->st_size=0;
  st->st_atim.tv_sec=now;
  st->st_mtim.tv_sec=now;
  st->st_ctim.tv_sec=now;
  }
if (string(path)=="/test_file") {
  st->st_mode=0444|S_IFREG;
  st->st_nlink=1;
  st->st_uid=0;
  st->st_gid=0;
  st->st_size=SIZE;
  st->st_atim.tv_sec=now;
  st->st_mtim.tv_sec=now;
  st->st_ctim.tv_sec=now;
  }
return 0;
}
int opendir(const char *path, struct fuse_file_info *) {
cout << "opendir " << path << endl;
return 0;
}
int readdir(const char *path, void * buffer, fuse_fill_dir_t filler, off_t offset,
                        struct fuse_file_info *x) {
cout << "readdir " << path << endl;
if (string(path)=="/") {
  filler(buffer,".",NULL,0);
  filler(buffer,"..",NULL,0);
  filler(buffer,"test_file",NULL,0);
  return 0;
  }
return -1;
}
int tjread(const char *path,char *data, size_t len, off_t off,
                        struct fuse_file_info *x) {
cout << "read " << path << endl;
if (string(path)=="/test_file") {
int newlen=(((SIZE-off)<len)?SIZE-off:len);
if (newlen<0) newlen=0;
if (newlen) memmove(data,test_file+off,newlen);
return newlen;
}
return -1;
}
int tjwrite(const char *path,const char *data, size_t len, off_t off,
                        struct fuse_file_info *x) {
cout << "write " << path << endl;
if (string(path)=="/test_file") {
int newlen=(((SIZE-off)<len)?SIZE-off:len);
if (newlen<0) newlen=0;
if (newlen) memmove(test_file+off,data,newlen);
return newlen;
}
return -1;
}
int tjopen(const char *path, struct fuse_file_info *x) {
cout << "open " << path << endl;
if (!test_file) test_file=(char*)malloc(SIZE);
return 0;
}
int main(int argc, char ** argv) {
//fd=open("log",O_RDWR|O_CREAT);
now=time(NULL);
x.getattr=attr;
x.opendir=opendir;
x.readdir=readdir;
x.read=tjread;
x.write=tjwrite;
x.open=tjopen;
fuse_main(argc, argv,&x, NULL);
return 0;
}
