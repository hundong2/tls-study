#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <vector>

#include "mpz.h"

namespace {

TEST(MpzEndian, RoundTripBigEndian) {
	std::array<unsigned char, 8> bytes{
		0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	};

	mpz_class n = bnd2mpz(bytes.begin(), bytes.end());

	std::array<unsigned char, 8> out{};
	mpz2bnd(n, out.begin(), out.end());
	EXPECT_EQ(bytes, out);
}

TEST(MpzMath, PowmMatchesKnownValue) {
	// 4^13 mod 497 = 445 (classic RSA toy example)
	mpz_class base = 4;
	mpz_class exp = 13;
	mpz_class mod = 497;
	mpz_class r = powm(base, exp, mod);
	EXPECT_EQ(r, 445);
}

TEST(MpzMath, NextPrimeProducesPrime) {
	mpz_class n = 1000;
	mpz_class p = nextprime(n);
	EXPECT_GT(p, n);
	// mpz_probab_prime_p: 0 composite, 1 probably prime, 2 definitely prime
	EXPECT_NE(mpz_probab_prime_p(p.get_mpz_t(), 25), 0);
}

TEST(MpzRandom, RandomPrimeReasonableSizeAndPrime) {
	// Keep this small and fast.
	// random_prime(byte) should return a prime that fits within the given byte length.
	const unsigned byteLen = 2;
	mpz_class p = random_prime(byteLen);

	EXPECT_GT(p, 0);
	EXPECT_NE(mpz_probab_prime_p(p.get_mpz_t(), 25), 0);

	// p < 256^byteLen
	mpz_class limit = 1;
	for (unsigned i = 0; i < byteLen; i++) limit *= 256;
	EXPECT_LT(p, limit);
}

}  // namespace
