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
#include <sys/ioctl.h>
#include <linux/fs.h>
using namespace std;
char * test_file=NULL;
const long SIZE=512;
const long SECTOR=512;
const long OFFSET=0;
const long CHUNK=512;
long ndev=3;
int fds[100]={};
char *chunk_buffer[100]={};
long current_line=-1;
const char *files[]={"/dev/sdb",
                     "/dev/sda",
                     "/dev/sdd",
                     "/dev/sdc",
                      NULL};
long tot_size=0;
long dev_size=0;
string layout="ra";
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
  st->st_size=tot_size;
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
long off_local=0;
for (long i=0;;++i) {
  long chunk_no  = off/CHUNK;
  long line_no   = chunk_no/ndev;
  long chunk_pos = off%CHUNK;
  if (current_line!=line_no) {
    current_line=line_no;
    for (int k=0;k<(ndev+1);++k) {
      int rd=pread(fds[k],chunk_buffer[k],CHUNK,SECTOR*OFFSET+line_no*CHUNK);
      if (rd<0)     throw "Error 2";
      if (rd<CHUNK) {
//      cout << rd << endl;
        return off_local;
        }
      }
// check the data:
    for (int j=0;j<CHUNK;++j) {
      char sum=0;
      for (int k=0;k<=ndev;++k) sum=sum^*(chunk_buffer[k]+j);
      if (sum!=0) {
        cerr << "wrong sum" << endl;
        throw "error 3";
        }
      }
    }
  long line_pos=ndev+1;
  if      (layout=="ls") { // left-symmetric
    line_pos       = chunk_no%(ndev+1);
    }
  else if (layout=="rs") { // right-symmetric
    line_pos       = (line_no%(ndev+1)+1+chunk_no%ndev)%(ndev+1);
    }
  else if (layout=="ra") { // right-assymetric
    long par_pos   =  line_no%(ndev+1);
    line_pos       = chunk_no%ndev;
    if (line_pos>=par_pos) ++line_pos;
    }
  else if (layout=="la") { // left-asymmetric
    long par_pos   =  ((-line_no-1)%(ndev+1)+ndev+1)%(ndev+1);
    line_pos       = chunk_no%ndev;
    if (line_pos>=par_pos) ++line_pos;
    }
  else {
    throw "Error 3: not implememted";
    }
  memmove(data+off_local,chunk_buffer[line_pos]+chunk_pos,SECTOR);
  off+=SECTOR;
  off_local+=SECTOR;
  if (off_local>=len) {
//  cout << off_local << " " << len << endl;
    return off_local;
    }
// char *chunk_buffer[100]={};
// long current_line=-1;
}
return -1;
}
return -1;
}
int tjwrite(const char *path,const char *data, size_t len, off_t off,
                        struct fuse_file_info *x) {
cout << "write " << path << endl;
return -1;
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
//if (!test_file) test_file=(char*)malloc(SIZE);
return 0;
}
int main(int argc, char ** argv) try {
//fd=open("log",O_RDWR|O_CREAT);
now=time(NULL);
x.getattr=attr;
x.opendir=opendir;
x.readdir=readdir;
x.read=tjread;
x.write=tjwrite;
x.open=tjopen;
//
struct stat st;
for (int i=0;files[i];++i) {
  fds[i]=open(files[i],O_RDONLY);
  if (fds[i]<0) {
    perror("open");
    throw "error 1";
    }
  if (stat(files[i],&st)<0) {
    cerr << files[i] << " ";
    perror("stat");
    exit(1);
    }
  chunk_buffer[i]=(char*)malloc(CHUNK);
//cout << st.st_size << endl;
  long dev_size1=-1;
  if ((st.st_mode&S_IFREG) == S_IFREG) {
    dev_size1=st.st_size;
    }
  if ((st.st_mode&S_IFBLK) == S_IFBLK) {
    if (ioctl(fds[i],BLKGETSIZE64,&dev_size1)==-1) {
      throw "Error 5";
      }
    }
  if (!dev_size) {
    dev_size=dev_size1-SECTOR*OFFSET;
    }
  if (dev_size!=(dev_size1-SECTOR*OFFSET)) throw "Devs must have the same size";
  tot_size+=dev_size;
  }
tot_size-=dev_size;
cout << tot_size << endl;
//
fuse_main(argc, argv,&x, NULL);
return 0;
} catch (char const * x) {
cerr << x << endl;
}
