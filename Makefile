objs=msgpack_server msgpack_client_nb msgpack_client_mt simple_server_mt simple_client_mt

CPPFLAGS=-O2 -I/home/hiroyuki/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LDFLAGS=-L/home/hiroyuki/lib -lmsgpack-rpc -lpthread

all: $(objs)

msgpack_server: msgpack_server.cc msgpack_server.h
	g++ $< -o $@ $(LDFLAGS) $(CPPFLAGS)

msgpack_server_mt: msgpack_server_mt.cc
	g++ $< -o $@ $(LDFLAGS) $(CPPFLAGS) 

msgpack_client_nb: msgpack_client_nb.cc
	g++ $< -o $@ $(LDFLAGS) $(CPPFLAGS)

simple_server_mt: simple_server_mt.cc
	g++ $< -o $@ $(LDFLAGS) $(CPPFLAGS)

simple_client_mt: simple_client_mt.cc
	g++ $< -o $@ $(LDFLAGS) $(CPPFLAGS)

data:
	 dd if=/dev/zero of=file bs=1M count=10240

clean:
	rm -rf *.o $(objs)
