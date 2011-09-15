#include <msgpack/rpc/client.h>
#include <msgpack/rpc/session_pool.h>
#include <mp/functional.h>

using namespace mp::placeholders;

namespace rpc {
  using namespace msgpack::rpc;
}

void echo_callback(rpc::future f, rpc::loop lo)
{
  try {
    int result = f.get<int>();
    std::cout << result << std::endl;
  } catch (msgpack::rpc::remote_error& e) {
    std::cout << e.what() << std::endl;
    exit(1);
  }
  lo->end();
}

int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);

  if (argc != 3) {
    std::cerr << argv[0] << " num_reqs echo/random_read" << std::endl;
    exit(1);
  }
  int num_reqs = atoi(argv[1]);
  std::string method = argv[2];
  std::cout << "num_reqs: " << num_reqs << std::endl;

  rpc::session_pool sp;
  //sp.start(100);

  rpc::future f[num_reqs];
  for (int i = 0; i < num_reqs; ++i) {
    rpc::session s = sp.get_session("127.0.0.1", 9999);
    if (method == "echo") {
      f[i] = s.call("echo", 10);
    } else if (method == "random_read" || method == "loop") {
      f[i] = s.call(method.c_str());
    }
    //f.attach_callback(mp::bind(echo_callback, _1, sp.get_loop()));
  }

  for(int i=0; i < num_reqs; ++i) {
    int ret = f[i].get<int>();
    //std::cout << "get for " << i << ": " << ret << std::endl;
  }

  return 0;
}
