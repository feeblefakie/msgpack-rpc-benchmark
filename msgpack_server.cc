#include "msgpack_server.h"
#include <msgpack/rpc/server.h>

namespace rpc {
  using namespace msgpack::rpc;
}

int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);

  if (argc != 2) {
    std::cerr << argv[0] << " num_threads" << std::endl;
    exit(1);
  }
  int num_threads = atoi(argv[1]);

  rpc::server svr;
  std::auto_ptr<rpc::dispatcher> dp(new msgpack_server());
  svr.serve(dp.get());
  svr.listen("0.0.0.0", 9999);
  svr.run(num_threads);

  return 0;
}
