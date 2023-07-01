//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//	Update: added Serializaiton
//  msgpack tutorial https://blog.devgenius.io/send-and-serialize-your-cv-mat-using-msgpack-and-zeromq-17c9ce53eacf



#include <zmq.hpp>
#include <string>
#include <iostream>
#include <msgpack.hpp>
#include "utils.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>


#define sleep(n)	Sleep(n)
#endif



int main () {
	const int length = sizeof(Content);
	Content response;
    //  Prepare our context and socket
    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");

    while (true) {
        zmq::message_t request;
		msgpack::unpacked unpacked_data;
		msgpack::sbuffer serialized;	
		
        socket.recv (request, zmq::recv_flags::none);
		msgpack::unpack(unpacked_data, static_cast<const char*>(request.data()), request.size());
		
		Content receivedData = unpacked_data.get().as<Content>();
		std::cout<<"Message header: " << receivedData.header << std::endl;
		std::cout<<"Message body: " << receivedData.body << std::endl;
		{
			std::cout<<"Message vector: ";
			for (int x : receivedData.vec) std::cout << x << " ";
			std::cout << std::endl;
		}
		
		if (strcmp(receivedData.header, "Subject") == 0) {
			std::cout<<"Subject it is! " << std::endl;
			strcpy(response.header, "Subject");
			strcpy(response.body, "content11");
			response.vec.assign({11, 21, 31});
			msgpack::pack(&serialized, response);
			zmq::message_t reply (serialized.size());
			memcpy(reply.data(), serialized.data(), serialized.size());
			socket.send(reply, zmq::send_flags::none);
		}

		else {
			std::cout<<"Public Key not requested! " << std::endl;
			zmq::message_t reply (5);
			memcpy (reply.data (), "Error", 5);
			socket.send (reply, zmq::send_flags::none);
		}
		
        //  Do some 'work'
        sleep(1);
    }
    return 0;
}
