#ifndef MSGPACK_SERVER_H_
#define MSGPACK_SERVER_H_

#include <msgpack/rpc/server.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace rpc {
  using namespace msgpack;
  using namespace msgpack::rpc;
}  // namespace rpc

class msgpack_server : public rpc::dispatcher {
public:
  typedef rpc::request request;

  msgpack_server()
  {
    fd_ = open("file", O_RDONLY | O_DIRECT);
    if (fd_ < 0) {
      exit(1);
    }
    struct stat sbuf;
    if (fstat(fd_, &sbuf) < 0) {
      perror("fstat");
      exit(1);
    }
    num_blks_ = sbuf.st_size / 4096;
    srand((unsigned) time(NULL));
  }

  void dispatch(request req)
  {
    try {
      std::string method;
      req.method().convert(&method);

      if (method == "echo") {
        //std::cout << "recieved" << std::endl;
        msgpack::type::tuple<int> params;
        req.params().convert(&params);
        req.result(params.get<0>());
        //std::cout << "sent" << std::endl;
      } else if (method == "random_read") {
        char *buf;
        posix_memalign((void **)&buf, 512, 4096);
        off_t blk_id = rand() % num_blks_; 
        //std::cout << "pread for blk: " << blk_id << std::endl;
        if (pread(fd_, buf, 4096, blk_id * 4096) < 0) {
          perror("pread");
        }
        free(buf);
        req.result(1);
      } else if (method == "loop") {
        int total = 0;
        for (int i = 0; i < 100000000; ++i) {
          ++total;
        }
        req.result(1);
      } else {
        req.error(msgpack::rpc::NO_METHOD_ERROR);
      }

    } catch (msgpack::type_error& e) {
      req.error(msgpack::rpc::ARGUMENT_ERROR);
      return;

    } catch (std::exception& e) {
      req.error(std::string(e.what()));
      return;
    }
  }

private:
  int fd_;
  off_t num_blks_;
};
#endif /* msgpack_server.h */
