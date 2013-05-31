#ifndef LEVIATHAN_RSAENCRYPTION
#define LEVIATHAN_RSAENCRYPTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "MPUINT.h"
#include "Random.h"
#include "ExceptionInvalidArguement.h"

namespace Leviathan{

	class RSAEncrypter : public Object{
	public:
		DLLEXPORT RSAEncrypter::RSAEncrypter();
		DLLEXPORT RSAEncrypter::~RSAEncrypter();

		DLLEXPORT void GenerateKeys(int keybitlength) throw(...);

		DLLEXPORT static bool RunTests(int count);


		// random generation //
		DLLEXPORT static void RandomMPUINT(mpuint &val);


		/*--------------------------------------
		Original Functions written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
		Code Edited by Henri Hyyryläinen
		----------------------------------------*/
		DLLEXPORT static void RSAKEYGeneration(mpuint &d, mpuint &e, mpuint &n);
		DLLEXPORT static inline void EncryptDecrypt(mpuint &result, const mpuint &source, const mpuint &e, const mpuint &n){
			mpuint::Power(source, e, n, result);
		}
	private:
		// keys //

		mpuint PrivateKey;
		mpuint PublicKey;
		mpuint ModulusN;
	};
}

#endif