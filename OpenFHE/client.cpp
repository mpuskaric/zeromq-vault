//
//  Homomorphic encryption client 
//  Connects REQ socket to tcp://localhost:5555
//	@author: Miroslav Puskaric https://github.com/mpuskaric


#include <string>
#include <iostream>
#include "utils.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

const char server_1[] = "127.0.0.1";
const char server_2[] = "127.0.0.1";
const char server_3[] = "127.0.0.1";

int main ()
{
	char sub[10];
	//ZeroMQ
	zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
	
	//OpenFHE	
	uint32_t multDepth = 5;
	uint32_t scaleFactorBits = 50;
	uint32_t batchSize = 8;
	
	SecretKeyDist secretKeyDist = UNIFORM_TERNARY;
	CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(multDepth);
    parameters.SetScalingModSize(scaleFactorBits);
	parameters.SetKeySwitchTechnique(HYBRID); 		//HYBRID
	parameters.SetScalingTechnique(FLEXIBLEAUTO); 	//FIXEDMANUAL FLEXIBLEAUTO
    parameters.SetBatchSize(batchSize);
	//parameters.SetSecurityLevel(HEStd_128_classic);

	CC fdaCC = GenCryptoContext(parameters);

	fdaCC->Enable(PKE);
	fdaCC->Enable(KEYSWITCH);
	fdaCC->Enable(PRE);			//ReKeyGen, Reencrypt
	fdaCC->Enable(FHE);
	fdaCC->Enable(LEVELEDSHE); 	//EvalMultKeyGen
	fdaCC->Enable(ADVANCEDSHE); //EvalSumKeyGen
	
	Key_Pair kp = fdaCC->KeyGen();
	fdaCC->EvalMultKeyGen(kp.secretKey);
	
	std::vector<double> x1 = {0.25, 0.5, 0.75, 1.0, 2.0, 3.0, 4.0};
	PT ptxt = fdaCC->MakeCKKSPackedPlaintext(x1);
	CT ctxt = fdaCC->Encrypt(ptxt,kp.publicKey);
	
	// Workflow: CC -> PubKey -> MultKey -> CText 
	zmq::message_t msg = prep_message<CC>(CryptoC, fdaCC); //#1
	
	//ZeroMQ - message send
    std::cout << "Connecting to server..." << std::endl;
    socket.connect("tcp://127.0.0.1:5555");
	socket.send(msg, zmq::send_flags::none);
	
	while (true)
	{
		zmq::message_t reply;
		Message response;
		socket.recv (reply, zmq::recv_flags::none);
		
		std::memcpy(response.header, reply.data(), sizeof(response.header));

		if (strcmp(response.header, subject(Result)) == 0) //#9
		{
			std::cout<<"Message header: " << response.header << std::endl;
			{
				std::istringstream is(std::string(static_cast<const char*>(reply.data())+ sizeof(response.header), 
										static_cast<const char*>(reply.data()) + reply.size()));
				CT result;
				Serial::Deserialize(result, is, SerType::BINARY);

				PT r;
				fdaCC->Decrypt(kp.secretKey, result, &r);
				r->SetLength(batchSize);
				std::cout << "result " << r << std::endl;
			}
			std::exit(1);
		}
		
		else if (strcmp(response.header, subject(PubKey)) == 0) 
		{

			continue;
		}
		
		else if (strcmp(response.header, subject(MultKey)) == 0) 
		{

			continue;
		}
		
		else if (strcmp(response.header, subject(CryptoC)) == 0) 
		{

			continue;
		}
		
		else if (strcmp(response.header, subject(Acknowledge)) == 0)
		{
			std::cout << "Acknowledge received! " << std::endl;
			break;
			//continue;
		}
		
		else if (strcmp(response.header, subject(R_PubKey)) == 0) //#3
		{
			zmq::message_t msg = prep_message<Public_Key>(PubKey, kp.publicKey);
			socket.send(msg, zmq::send_flags::none);
			continue;
		}
		
		else if (strcmp(response.header, subject(R_CText)) == 0)  //#7
		{
			zmq::message_t msg = prep_message<CT>(CText, ctxt);
			socket.send(msg, zmq::send_flags::none);
			continue;
		}
		
		else if (strcmp(response.header, subject(R_MultKey)) == 0) //#5
		{
			zmq::message_t msg = prep_multkey(MultKey, fdaCC);
			socket.send(msg, zmq::send_flags::none);
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
