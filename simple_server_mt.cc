#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *reader(void *p);

struct thread_arg_t {
  int id;
  int sock;
};

int fd;
int num_blks;
std::string method;

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << argv[0] << " num_threads echo/random_read" << std::endl;
    exit(1);
  }
  int num_threads = atoi(argv[1]);
  method = argv[2];

  int sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  int on = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

  struct sockaddr_in saddr;
  memset((char *) &saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(9999);

  if (bind(sock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(sock, num_threads * 10) < 0) {
    perror("bind");
    exit(1);
  }

  fd = open("file", O_RDONLY | O_DIRECT);
  if (fd < 0) {
    exit(1);
  }
  struct stat sbuf;
  if (fstat(fd, &sbuf) < 0) {
    perror("fstat");
    exit(1);
  }
  num_blks = sbuf.st_size / 4096;
  srand((unsigned) time(NULL));

  pthread_t tid[num_threads];
  thread_arg_t arg[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    arg[i].id = i;
    arg[i].sock = sock;
    if (pthread_create(&tid[i], NULL, reader, (void *) &arg[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }   
  }
  void *ret = NULL;
  for (int i = 0; i < num_threads; ++i) {
    if (pthread_join(tid[i], &ret)) {
    perror("pthread_join"); }   
  }

  return 0;
}

void *reader(void *p)
{ 
  thread_arg_t *arg = (thread_arg_t *) p;
  char recv_buf[4];

  int connfd;
  while (1) {
    struct sockaddr_in caddr;
    socklen_t caddr_len = sizeof(caddr);
    if ((connfd = accept(arg->sock, (struct sockaddr *) &caddr, &caddr_len)) < 0) {
      perror("accept");
    }
    //std::cout << "accepted" << std::endl;

    ssize_t size_recved = 0;
    while (size_recved < 4) {
      ssize_t size = recv(connfd, recv_buf+size_recved, sizeof(recv_buf)-size_recved, 0);
      if (size < 0) {
        perror("recv");
      }
      size_recved += size;
    }
    // assumes that first 4 bytes for count, next 4 bytes for offset
    int req = *(int *) recv_buf;
    //std::cout << "request: " << req << std::endl;

    int res;
    if (method == "echo") {
      res = req;
    } else if (method == "random_read") {
      char *buf;
      posix_memalign((void **)&buf, 512, 4096);
      off_t blk_id = rand() % num_blks; 
      //std::cout << "pread for blk: " << blk_id << std::endl;
      if (pread(fd, buf, 4096, blk_id * 4096) < 0) {
        perror("pread");
      }
      free(buf);
      res = 1;
    } else if (method == "loop") {
      int total = 0;
      for (int i = 0; i < 100000000; ++i) {
        ++total;
      }
      res = 1;
    }
    send(connfd, &res, sizeof(int), 0);
    close(connfd);
  }
  return NULL;
}
