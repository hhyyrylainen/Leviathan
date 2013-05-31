#ifndef LEVIATHAN_MPUINT
#define LEVIATHAN_MPUINT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

/*--------------------------------------
Original class written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
Code Edited by Henri Hyyryläinen
----------------------------------------*/

//extern void numeric_overflow(void);

namespace Leviathan{

	class mpuint {

	private:
		unsigned short remainder(unsigned short);
		void shift(unsigned);
	public:
		// functions //
		mpuint(unsigned);
		mpuint(const mpuint &);
		~mpuint();

		bool IsZero(void) const;
		int Compare(const mpuint &) const;
		int Compare(unsigned short) const;
		char* edit(char *) const;
		void dump() const;

		// operators //
		void operator = (const mpuint &);
		void operator = (unsigned short);
		void operator += (const mpuint &);
		void operator += (unsigned short);
		void operator -= (const mpuint &);
		void operator -= (unsigned short);
		void operator *= (const mpuint &);
		void operator *= (unsigned short);
		void operator /= (const mpuint &);
		void operator /= (unsigned short);
		void operator %= (const mpuint &);
		void operator %= (unsigned short);

		// getting values //
		bool scan(const char *&);

		// short operators //
		bool mpuint::operator == (const mpuint &n) const {return Compare(n) == 0;}
		bool mpuint::operator != (const mpuint &n) const {return Compare(n) != 0;}
		bool mpuint::operator >  (const mpuint &n) const {return Compare(n) >  0;}
		bool mpuint::operator >= (const mpuint &n) const {return Compare(n) >= 0;}
		bool mpuint::operator <  (const mpuint &n) const {return Compare(n) <  0;}
		bool mpuint::operator <= (const mpuint &n) const {return Compare(n) <= 0;}
		bool mpuint::operator == (unsigned short n) const {return Compare(n) == 0;}
		bool mpuint::operator != (unsigned short n) const {return Compare(n) != 0;}
		bool mpuint::operator >  (unsigned short n) const {return Compare(n) >  0;}
		bool mpuint::operator >= (unsigned short n) const {return Compare(n) >= 0;}
		bool mpuint::operator <  (unsigned short n) const {return Compare(n) <  0;}
		bool mpuint::operator <= (unsigned short n) const {return Compare(n) <= 0;}

		// static functions //
		static void Divide(const mpuint &, const mpuint &, mpuint &, mpuint &);
		static void Power(const mpuint &, const mpuint &, const mpuint &, mpuint &);

		// data //
		unsigned short *value;
		unsigned length;
	};

}

#endif