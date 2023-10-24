#pragma once

#include <string>
#include <cstdint>

namespace splice
{
	static const int PROVIDER_KEY_SIZE = 42;
	static const int HASH_SIZE = 32;

	enum Validity
	{
		Validity_OK,                 // Everything is OK
		Validity_MISSING,            // No lease file found, or the file could not be read
		Validity_EXPIRED,            // Lease is expired
		Validity_INACTIVE,           // Lease is inactive
		Validity_BEGINS_IN_FUTURE,   // Lease creation time is in the future compared to the system clock
		Validity_EXPIRATION_TOO_FAR, // Lease expiration time is too far into the future (90 days)
		Validity_INVALID_FORMAT,     // Format field in lease file does not have the expected value
		Validity_INVALID_PRODUCT,    // Product name in the lease file does not match the expected product
		Validity_PARSE_ERROR,        // Lease file seems to be corrupt
		Validity_INCORRECT_SIGNATURE,// Lease was signed and signature does not pass verification
		Validity_UNKNOWN_ERROR       // Unknown error (should not happen)
	};

	struct Lease
	{
		Lease();

		uint8_t      format;
		std::string  product;
		int8_t       state;
		uint64_t     id;
		std::string  user;
		uint8_t      hash[HASH_SIZE];
		int64_t      creationTime;
		int64_t      expirationTime;
		Validity     validity;
		std::string  encodedPayload;
		std::string  decodedPayload;
	};

	class Repository
	{
	public:
		Repository(const uint8_t *providerKey);

		Lease loadLease(const std::string &product) const;

	private:
		std::string _providerKey;
	};

	bool verifySignature(std::string encodedPayload);

	std::string validityToString(Validity validity);

	std::string decode(std::string encodedPayload);

	std::string hashToString(const uint8_t *data);

	std::string timeToString(int64_t time);
}
