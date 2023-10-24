#include "splice.h"
#include <iostream>

// TODO: Set your provider key here
static const uint8_t PROVIDER_KEY[splice::PROVIDER_KEY_SIZE] = {0x1a, 0xe9, 0x85, 0xf6, 0xef, 0xda, 0xc7, 0xd6, 0x7b, 0x23, 0xda, 0x5, 0x49, 0x16, 0x9e, 0x63, 0xcb, 0x37, 0xd6, 0xe9, 0x1f, 0x2, 0x8d, 0x78, 0x3e, 0x44, 0xee, 0x92, 0x3c, 0x7, 0x7e, 0x55, 0xfa, 0x2e, 0xc8, 0x63, 0x90, 0xca, 0xe5, 0x46, 0x27, 0x12};

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
