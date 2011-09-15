#include <msgpack/rpc/client.h>
#include <msgpack/rpc/session_pool.h>
#include <mp/functional.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

using namespace mp::placeholders;

namespace rpc {
  using namespace msgpack::rpc;
}

void *echo_client(void *p);

struct thread_arg_t {
  int id;
  int num_accesses;
};
rpc::session_pool sp;
std::string method;

int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);

  if (argc != 4) {
    std::cerr << argv[0] << " num_reqs echo/random_read/loop num_threads" << std::endl;
    exit(1);
  }
  int num_reqs = atoi(argv[1]);
  method = argv[2];
  int num_threads = atoi(argv[3]);
  std::cout << "num_reqs: " << num_reqs << std::endl;
  std::cout << "num_threads: " << num_threads << std::endl;

  sp.start(num_threads);
  pthread_t tid[num_threads];
  thread_arg_t arg[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    arg[i].id = i;
    arg[i].num_accesses = num_reqs / num_threads;
    if (pthread_create(&tid[i], NULL, echo_client, (void *) &arg[i]) != 0) {
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

void *echo_client(void *p)
{ 
    thread_arg_t *arg = (thread_arg_t *) p;

    for (int i = 0; i < arg->num_accesses; ++i) {
      rpc::session s = sp.get_session("127.0.0.1", 9999);
      int res;
      if (method == "echo") {
        res = s.call("echo", 10).get<int>();
      } else if (method == "random_read" || method == "loop") {
        res = s.call(method.c_str()).get<int>();
      }
    }
    return NULL;
}
