#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RSAENCRYPTION
#include "RSAEncryption.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "CommonMath.h"
#include "EuclideanFunctions.h"

/*--------------------------------------
Original Function written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
Code Edited by Henri Hyyryläinen
----------------------------------------*/
/*----------------------------------------------------------------------------
This function generates a (large) prime.
----------------------------------------------------------------------------*/
static void GeneratePrime(mpuint &p)
{
	RSAEncrypter::RandomMPUINT(p);
	p.value[p.length-1] |= 0x8000;
	p.value[0] |= 1;
	while (!MMath::IsPrime(p))
		p += 2;
}


void Leviathan::RSAEncrypter::RSAKEYGeneration(mpuint &d, mpuint &e, mpuint &n)
{
	mpuint p(d.length/2);
	GeneratePrime(p);
	mpuint q(d.length/2);
	GeneratePrime(q);
	mpuint pp(p);
	pp -= 1;
	mpuint qq(q);
	qq -= 1;
	mpuint pq(d.length);
	pq = pp;
	pq *= qq;
	n = p;
	n *= q;
	RSAEncrypter::RandomMPUINT(d);
	d %= pq;
	mpuint temp(d.length);
	mpuint g(d.length);
	while (true)
	{
		EuclideanAlgorithm(d, pq, e, temp, g);
		if (g == 1)
			break;
		d += 1;
	}
}

// ---------------- RSAEncrypter -------------------- //

// ------------------------------------ //
DLLEXPORT void Leviathan::RSAEncrypter::RandomMPUINT(mpuint &val){
	// fill with random numbers //
	for(unsigned i = 0; i < val.length; i++)
		val.value[i] = USHORT(Random::Get()->GetNumber(0, USHRT_MAX))  << 8 | USHORT(Random::Get()->GetNumber(0, USHRT_MAX));
}

DLLEXPORT bool Leviathan::RSAEncrypter::RunTests(int count){
	// number that is encrypted //
	int valuetosend = 254;

	RSAEncrypter encrypter;

	// create 512 bit keys //
	encrypter.GenerateKeys(512);

	// try encrypting the value with the keys //


}

DLLEXPORT void Leviathan::RSAEncrypter::GenerateKeys(int keybitlength){
	if(keybitlength % 16 != 0){

		throw ExceptionInvalidArguement(L"key not multiple of 16 bits", keybitlength, __WFUNCTION__, L"keybitlength");
	}

	PrivateKey = mpuint(keybitlength % 16);
	PublicKey = mpuint(keybitlength % 16);
	ModulusN = mpuint(keybitlength % 16);

	// generate key //
	RSAKEYGeneration(PrivateKey, PublicKey, ModulusN);

	// keys done //

}

// ------------------------------------ //

// ------------------------------------ //


