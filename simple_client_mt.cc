#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <iostream>

void *reader(void *p);

struct thread_arg_t {
  int id;
  int num_accesses;
};
size_t blksize;
off_t num_blks;

int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);

  if (argc != 3) {
    std::cerr << argv[0] << " num_reqs num_threads" << std::endl;
    exit(1);
  }
  int num_reqs = atoi(argv[1]);
  int num_threads = atoi(argv[2]);
  std::cout << "num_reqs: " << num_reqs << std::endl;
  std::cout << "num_threads: " << num_threads << std::endl;

  pthread_t tid[num_threads];
  thread_arg_t arg[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    arg[i].id = i;
    arg[i].num_accesses = num_reqs / num_threads;
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

  for (int i = 0; i < arg->num_accesses; ++i) {
    int dst_ip;
    if ((dst_ip = inet_addr("127.0.0.1")) == INADDR_NONE) {
      struct hostent *he;
      if ((he = gethostbyname("127.0.0.1")) == NULL) {
        std::cerr << "gethostbyname failed" << std::endl;
        exit(1);
      }   
      memcpy((char *) &dst_ip, (char *) he->h_addr, he->h_length);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      perror("socket");
    }

    struct sockaddr_in server;
    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = dst_ip;
    server.sin_port = htons(9999);

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
      perror("connect");
      exit(1);
    }

    int req = 10;
    if (send(sock, (void *) &req, sizeof(int), 0) < 0) {
      perror("send");
    }

    int res;
    if (recv(sock, (int *) &res, sizeof(int), 0) < 0) {
      perror("recv");
    }
    //std::cout << "received: [" << res << "]" << std::endl;

    close(sock);
  }
  return NULL;
}
