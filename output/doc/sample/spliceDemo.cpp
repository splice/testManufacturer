#include "splice.h"
#include <iostream>

// TODO: Set your provider key here
static const uint8_t PROVIDER_KEY[splice::PROVIDER_KEY_SIZE] = {0xa6, 0x62, 0x58, 0x56, 0x22, 0x5b, 0xde, 0xfb, 0xc5, 0xc3, 0x9, 0x3a, 0x3a, 0xd9, 0x45, 0xda, 0xd6, 0x7e, 0x74, 0x4d, 0x6, 0x5c, 0xbf, 0x5, 0x7f, 0x89, 0xdc, 0x45, 0xb6, 0x28, 0x96, 0x94, 0xc9, 0x58, 0xac, 0xf1, 0xa2, 0x85, 0x30, 0x41, 0x9d, 0x76};

// TODO: Set your product name here
static const char PRODUCT_NAME[] = "testPlugin";

static const bool SIGN = true;

int main(int argc,  char* argv[])
{
	// Read plugin name from args
	std::string productName;
	if (argc > 1 ) {
		productName = argv[1];
	} else {
		productName = PRODUCT_NAME;
	}

	splice::Repository repository(PROVIDER_KEY);
	splice::Lease lease = repository.loadLease(productName);

	std::cout << "Validity...........: " << splice::validityToString(lease.validity) << std::endl;
	std::cout << "Format.............: " << (int)lease.format << std::endl;
	std::cout << "Product............: " << lease.product << std::endl;
	std::cout << "State..............: " << (int)lease.state << std::endl;
	std::cout << "ID.................: " << lease.id << std::endl;
	std::cout << "User...............: " << lease.user << std::endl;
	std::cout << "Creation time......: " << splice::timeToString(lease.creationTime) << std::endl;
	std::cout << "Expiration time....: " << splice::timeToString(lease.expirationTime) << std::endl;
	std::cout << "Encoded payload....: " << lease.encodedPayload << std::endl;
	std::cout << "Decoded payload....: " << lease.decodedPayload << std::endl;

	if (lease.validity == splice::Validity_OK)
	{
		if (SIGN) {
			if (splice::verifySignature(lease.encodedPayload)) {
				std::cout << "Payload is valid and signed" << std::endl;
			} else {
				std::cout << "Encoded payload is invalid" << std::endl;
			}
		} else {
			// TODO: Run custom authorization procedure with lease.decodedPayload
			std::cout << "custom authorization here" << std::endl;
		}
		return 0;
	}
	else
	{
		std::cout << "Invalid lease file" << std::endl;
		return 1;
	}
}
