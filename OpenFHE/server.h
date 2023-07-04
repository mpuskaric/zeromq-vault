//	@author: Miroslav Puskaric https://github.com/mpuskaric

#ifndef SERVER_H
#include <string>
#include <iostream>
#include "openfhe.h"

CT multiply(CC &cc, Public_Key &pKey, CT &c1, std::vector<double> &x2) 
{
	PT p2;
	CT result, c2;
	p2 = cc->MakeCKKSPackedPlaintext(x2);
	c2 = cc->Encrypt(p2, pKey);
	result = cc->EvalMult(c1,c2);
	return result;
}

#endif