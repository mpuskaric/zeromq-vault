//
//	Homomorphic encryption server 
//  server in C++
//  Binds REP socket to tcp://*:5555
//	@author: Miroslav Puskaric https://github.com/mpuskaric


#include <zmq.hpp>
#include <string>
#include <iostream>
#include "utils.h"
#include "server.h"
#include "openfhe.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

const char server_1[] = "127.0.0.1";

int main () {
	
	//OpenFHE
	CC fdaCC;
	Public_Key PKey;
	Eval_Key MKey;
	CT ctext, res;
	
	std::vector<double> x2 = {0.25, 0.5, 0.75, 1.0, 2.0, 3.0, 4.0};
	
    //  Prepare our context and socket
    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");
	
    while (true) {
		Message request;
        zmq::message_t incoming;
		msgpack::unpacked unpacked_data;
		msgpack::sbuffer serialized;
		
        socket.recv (incoming, zmq::recv_flags::none);
		std::memcpy(request.header, incoming.data(), sizeof(request.header));
		
		if (strcmp(request.header, subject(CText)) == 0) //#8
		{
			std::cout << "Received " << std::endl;
			ctext  = receive_params<CT>(incoming);
			// Do some data processing
			CT res = multiply(fdaCC, PKey, ctext, x2);
			zmq::message_t reply =	prep_message<CT>(Result, res);
			socket.send(reply, zmq::send_flags::none);
			continue;
		}

		else if	(strcmp(request.header, subject(PubKey)) == 0) //#4
		{
			PKey = receive_params<Public_Key>(incoming);
			zmq::message_t outgoing = prep_request(R_MultKey);
			socket.send(outgoing, zmq::send_flags::none);
			continue;
		}
		
		else if (strcmp(request.header, subject(MultKey)) == 0)	//#6
		{
			receive_multkey(incoming, fdaCC);
			zmq::message_t outgoing = prep_request(R_CText);
			socket.send(outgoing, zmq::send_flags::none);
			continue;
		}
		
		else if (strcmp(request.header, subject(CryptoC)) == 0) //#2
		{
			fdaCC = receive_params<CC>(incoming);
			zmq::message_t outgoing = prep_request(R_PubKey);
			socket.send(outgoing, zmq::send_flags::none);
			continue;
		}
		
		else if (strcmp(request.header, subject(Acknowledge)) == 0)
		{
			continue;
		}
		
		else 
		{
			std::cout << "Unknown message..." << std::endl;
			std::exit(1);
		}
		
        //  Do some 'work'
        //sleep(1);
    }
    return 0;
}
