//
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//	Update: added Serializaiton
//  msgpack tutorial https://blog.devgenius.io/send-and-serialize-your-cv-mat-using-msgpack-and-zeromq-17c9ce53eacf

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <msgpack.hpp>
#include "utils.h"

int main ()
{
	const int length = sizeof(Content);
	msgpack::sbuffer serialized;
	msgpack::unpacked unpacked_data;
    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);

    std::cout << "Connecting to hello world server..." << std::endl;
    socket.connect("tcp://127.0.0.1:5555");
	
	//testing struct
	Content tosend;
	strcpy(tosend.header, "Subject");
	strcpy(tosend.body, "content10");
	tosend.vec.assign({10, 20, 30, 40});
	
	msgpack::pack(&serialized, tosend);
	
	zmq::message_t msg(serialized.size());
	memcpy(msg.data(), serialized.data(), serialized.size());
	//std::cout << hexDump(msg) << std::endl;
	socket.send(msg, zmq::send_flags::none);
	
	//  Get the reply.
	zmq::message_t reply;
	socket.recv (reply, zmq::recv_flags::none);
	msgpack::unpack(unpacked_data, static_cast<const char*>(reply.data()), reply.size());
	Content response = unpacked_data.get().as<Content>();
	//std::cout << hexDump(reply) << std::endl;
	if (strcmp(response.header, "Subject") == 0) {
		std::cout<<"Message header: " << response.header << std::endl;
		std::cout<<"Message body: " << response.body << std::endl;
		{
			std::cout<<"Message vector: ";
			for (int x : response.vec) std::cout << x << " ";
			std::cout << std::endl;
		}
	}

    return 0;
}
