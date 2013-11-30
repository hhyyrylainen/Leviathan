#ifndef LEVIATHAN_RSAENCRYPTION
#define LEVIATHAN_RSAENCRYPTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Utility\Random.h"
#include "Exceptions\ExceptionInvalidArgument.h"
#include <iterator>

#pragma warning( disable: 4005 4127 )

#include <boost/limits.hpp>
#include <boost/integer.hpp>
#include <boost/xint/integer.hpp>
#include <boost/static_assert.hpp>

namespace xopt = boost::xint::options;
using boost::xint::callback_t;
using boost::xint::no_callback;

namespace Leviathan{

	static const std::size_t cbits = std::numeric_limits<char>::digits;

	template <std::size_t Bits>
	class Rsa : public Object{
	public:
		// We're going to use a fixed-length type for this example, primarily to
		// show how they would be used. The calculations require intermediate
		// results that are between two and three times the bit-size of the number
		// though. We'll also make it unsigned, and use secure mode to reduce the
		// chance that the key data gets written to disk anywhere.
		typedef typename boost::xint::integer_t<xopt::fixedlength<Bits * 3>, xopt::secure, xopt::negative_modulus> KeyNumber;

		// We also need the smallest type that can hold a Bits-length number of characters.
		typedef typename boost::uint_value_t<(Bits + cbits - 1) / cbits>::least SizeT;

		// There must be at least enough bits to handle one character and a SizeT.
		BOOST_STATIC_ASSERT(Bits > ((sizeof(SizeT) + 1) * cbits));

		// The number of bits needs to be even.
		BOOST_STATIC_ASSERT((Bits & 0x01) == 0);

		Rsa(const string keys);

		string publickey() const;
		string privatekey() const;

		string encrypt(const string data) const;
		string decrypt(const string data) const;

		static Rsa generate(callback_t callback = no_callback);

	private:
		static std::size_t calculate_blocksize(KeyNumber n);

		Rsa(const KeyNumber _n, const KeyNumber _d, const KeyNumber _e): n(_n), d(_d), e(_e) { blocksize = calculate_blocksize(n); };

		std::string number_to_binary_string(const KeyNumber num) const;
		KeyNumber binary_string_to_number(const std::string s) const;

		KeyNumber n, d, e;
		std::size_t blocksize;
	};

	template <std::size_t Bits>
	std::size_t Rsa<Bits>::calculate_blocksize(KeyNumber n) {
		// Round the size of n (in bits) down to the next lower multiple of the
		// number of bits in a character. That's how many characters we can fit into
		// a single block, for encryption purposes.
		std::size_t size_in_bits = log2(n) - 1;
		return (size_in_bits + cbits - 1) / cbits;
	}

	template <std::size_t Bits>
	std::string Rsa<Bits>::number_to_binary_string(const KeyNumber num) const {
		boost::xint::binary_t b = to_binary(num);
		std::string s;
		std::copy(b.begin(), b.end(), std::back_inserter(s));
		return s;
	}

	template <std::size_t Bits>
	typename Rsa<Bits>::KeyNumber Rsa<Bits>::binary_string_to_number(const
		std::string s) const
	{
		boost::xint::binary_t b;
		std::copy(s.begin(), s.end(), std::back_inserter(b));
		return KeyNumber(b);
	}

	template <std::size_t Bits>
	Rsa<Bits>::Rsa(const std::string keys) {
		// Make sure it's a proper key, by checking the signature.
		bool goodkey = true;
		if (keys.substr(0, 4) == "{RSA") {
			std::istringstream str(keys.substr(4));
			std::size_t recordedbits = 0;
			char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
			str >> recordedbits >> c1 >> e >> c2 >> n >> c3;
			if (c1 == ',' && c2 == ',') {
				if (c3 == ',') {
					// It's a private key, including the decryption key.
					str >> d >> c4;
					if (c4 != '}') goodkey = false;
				} else {
					// It's a public key, no decryption key is included.
					if (c3 != '}') goodkey = false;
				}
			} else goodkey = false;

			// Make sure it's the right size
			if (goodkey && recordedbits != Bits)
				throw std::out_of_range("Wrong number of bits");
		} else goodkey = false;

		if (!goodkey) throw std::invalid_argument("Not a valid key");
		blocksize = calculate_blocksize(n);
	}

	template <std::size_t Bits>
	std::string Rsa<Bits>::publickey() const {
		std::ostringstream str;
		str << "{RSA" << Bits << ',' << e << ',' << n << '}';
		return str.str();
	}

	template <std::size_t Bits>
	std::string Rsa<Bits>::privatekey() const {
		std::ostringstream str;
		str << "{RSA" << Bits << ',' << e << ',' << n << ',' << d << '}';
		return str.str();
	}

	template <std::size_t Bits>
	std::string Rsa<Bits>::encrypt(const std::string data) const {
		// A proper implementation would pad the message with additional random
		// data, to avoid the low encryption exponent attack. This example
		// implementation does not.

		// The message may contain up to (blocksize - 1) extra bytes when it's
		// decrypted. Prepend a SizeT with the number of bytes to remove from the
		// end.
		const unsigned char mask = (unsigned char)(-1);
		std::string msg;
		SizeT trimblock = blocksize - ((data.length() + sizeof(SizeT)) % blocksize);
		if (trimblock == blocksize) trimblock = 0;
		for (std::size_t i = sizeof(SizeT); i > 0; --i) {
			msg += static_cast<char>(trimblock & mask);
			trimblock >>= cbits;
		}
		msg += data;

		// Split the message into blocks of blocksize and encrypt each one.
		std::string encrypted_msg;
		for (std::size_t block = 0; block * blocksize < msg.length(); ++block) {
			// Grab a block of blocksize bytes.
			std::string tblock = msg.substr(block * blocksize, blocksize);

			// Turn it into a KeyNumber.
			KeyNumber mnumber = binary_string_to_number(tblock);

			// Encrypt that number
			mnumber = powmod(mnumber, e, n);

			// Append the encrypted data to the return value, padded to the proper
			// block length.
			tblock = number_to_binary_string(mnumber);
			if (tblock.length() < blocksize) tblock += std::string(blocksize -
				tblock.length(), 0);
			encrypted_msg += tblock;
		}

		return encrypted_msg;
	}

	template <std::size_t Bits>
	std::string Rsa<Bits>::decrypt(const std::string encrypted_msg) const {
		std::string decrypted_msg;

		// Split the message into blocks of blocksize and decrypt each one.
		for (std::size_t block = 0; block * blocksize < encrypted_msg.length();
			++block)
		{
			// Grab a block of blocksize bytes.
			std::string tblock = encrypted_msg.substr(block * blocksize, blocksize);

			// Turn it into a KeyNumber.
			KeyNumber mnumber = binary_string_to_number(tblock);

			// Decrypt that number
			mnumber = powmod(mnumber, d, n);

			// Append the encrypted data to the return value, padded to the proper
			// block length.
			tblock = number_to_binary_string(mnumber);
			if (tblock.length() < blocksize) tblock += std::string(blocksize -
				tblock.length(), 0);
			decrypted_msg += tblock;
		}

		// Trim the added bytes off of it.
		SizeT trimblock = 0;
		for (std::size_t i = 0; i < sizeof(SizeT); ++i)
			trimblock |= (SizeT(decrypted_msg[i]) << (i * cbits));
		decrypted_msg = decrypted_msg.substr(sizeof(SizeT), decrypted_msg.length() -
			trimblock - sizeof(SizeT));

		return decrypted_msg;
	}

	template <std::size_t Bits>
	Rsa<Bits> Rsa<Bits>::generate(callback_t callback) {
		// Use the system's strong random number generator, via the XInt-provided
		// convenience class.
		boost::xint::strong_random_generator gen;

		// Generate two random prime numbers, each containing no more than half of
		// the requested bits, and compute the product.
		KeyNumber p = KeyNumber::random_prime(gen, Bits / 2, callback);
		KeyNumber q = KeyNumber::random_prime(gen, Bits / 2, callback);
		assert(p != q); // If they're identical, there's almost certainly a problem

		// Compute the product of the primes.
		KeyNumber n(p * q);

		// Select an encryption key e, such that e and (p - 1) * (q - 1) are
		// relatively prime. Encryption goes a lot faster if you use small primes
		// for this value, and 65537 is recommended by X.509 and PKCS #1.
		KeyNumber e(65537);

		// Compute the decryption key.
		KeyNumber d(invmod(e, (p - 1) * (q - 1)));

		// That's all we have to do. Just plug those numbers into an Rsa object and
		// return it.
		return Rsa<Bits>(n, d, e);
	}

}
#pragma warning( default: 4005 4127)
#endif