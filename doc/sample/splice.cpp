#include "splice.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <iostream>
#include <strstream>

#include "ed25519/ed25519.h"
#include "libb64/encode.h"
#include "libb64/decode.h"

#if defined(_WIN32)

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <ShlObj.h>

namespace splice
{
	static std::wstring getFilePath(const std::string& product)
	{
		std::wstring fullPath;
		PWSTR path;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path)))
		{
			fullPath = std::wstring(path) + L"\\SpliceSettings\\license\\" + std::wstring(product.begin(), product.end()) + L".lic";
			CoTaskMemFree(path);
		}
		return fullPath;
	}

	static bool localtime_threadsafe(tm *tm, time_t t)
	{
		return localtime_s(tm, &t) == 0;
	}
}

#elif defined(__APPLE__)

#include <CoreServices/CoreServices.h>

namespace splice
{
	static std::string getFilePath(const std::string& product)
	{
		char path[PATH_MAX];
		FSRef ref;
		OSType folderType = kDomainLibraryFolderType;
		FSFindFolder(kUserDomain, folderType, kCreateFolder, &ref);
		FSRefMakePath(&ref, (UInt8 *)path, PATH_MAX);
		return std::string(path) + "/Audio/Plug-Ins/Support/Splice/license/" + product + ".lic";
	}

	static bool localtime_threadsafe(tm *tm, time_t t)
	{
		return localtime_r(&t, tm);
	}
}

#else

#error "Unknown platform"

#endif

namespace splice
{
	static const int HEADER_OFFSET = 4;
	static const int HEADER_SIZE = 314;
	static const int STRING_FIELD_SIZE = 128;
	static const int FORMAT_VERSION = 2;
	static const int SIGNATURE_SIZE = 91;

	static const std::string PUBLIC_KEY = "A0QL4Ob+cdzPOqoOsxKU1glcbZhh4hBqacbZRX9oblk=";

	static void vigenere(
		std::string::iterator textStart,
		std::string::iterator textEnd,
		std::string::const_iterator keyStart,
		std::string::const_iterator keyEnd)
	{
		auto textIt = textStart;
		auto keyIt = keyStart;

		while (textIt != textEnd)
		{
			*textIt ^= *keyIt;
			++textIt;
			++keyIt;

			if (keyIt == keyEnd)
			{
				keyIt = keyStart;
			}
		}
	}

	static std::string loadLicenseFile(const std::string &product)
	{
		std::ifstream file(getFilePath(product), std::ios_base::binary | std::ios_base::ate);
		std::streampos size = file.tellg();
		if (size <= 0) { return ""; }
		std::string result;
		result.reserve((size_t)size);
		file.seekg(0, std::ios::beg);
		result.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		return result;
	}

	Lease::Lease()
		: format(0)
		, state(0)
		, id(0)
		, creationTime(0)
		, expirationTime(0)
		, validity(Validity_UNKNOWN_ERROR)
	{
		memset(hash, 0, sizeof(hash));
	}

	Repository::Repository(const uint8_t *providerKey)
		: _providerKey(providerKey, providerKey + PROVIDER_KEY_SIZE)
	{ }

	Lease Repository::loadLease(const std::string &product) const
	{
		Lease result;
		int activeState = 1;

		std::string data = loadLicenseFile(product);

		if (data.size() == 0)
		{
			result.validity = Validity_MISSING;
			return result;
		}

		if (data.size() < HEADER_OFFSET + HEADER_SIZE)
		{
			result.validity = Validity_PARSE_ERROR;
			return result;
		}

		uint16_t licenseKeyLength = (uint8_t)data[0] | ((uint8_t)data[1] << 8);
		uint16_t licenseSigned = (uint8_t)data[2] | ((uint8_t)data[3] << 8);

		if ((int)data.size() < HEADER_OFFSET + HEADER_SIZE + licenseKeyLength)
		{
			result.validity = Validity_PARSE_ERROR;
			return result;
		}

		vigenere(
			data.begin() + HEADER_OFFSET,
			data.end(),
			_providerKey.begin(),
			_providerKey.end()
		);

		result.encodedPayload = data.substr(HEADER_OFFSET + HEADER_SIZE, licenseKeyLength);
		vigenere(
			data.begin() + HEADER_OFFSET,
			data.begin() + HEADER_OFFSET + HEADER_SIZE,
			result.encodedPayload.begin(),
			result.encodedPayload.begin() + std::min(24, (int)result.encodedPayload.size())
		);

		std::istringstream stream(data);
		stream.ignore(HEADER_OFFSET);
		stream.read((char*)&result.format, sizeof(result.format));

		if (result.format != FORMAT_VERSION)
		{
			result.validity = Validity_INVALID_FORMAT;
			return result;
		}

		char expectedProductField[STRING_FIELD_SIZE];
		memset(expectedProductField, 0, STRING_FIELD_SIZE);
		memcpy(expectedProductField, product.c_str(), std::min(STRING_FIELD_SIZE, (int)product.size()));

		char actualProductField[STRING_FIELD_SIZE + 1];
		actualProductField[STRING_FIELD_SIZE] = 0;
		stream.read(actualProductField, STRING_FIELD_SIZE);

		result.product = std::string(actualProductField);
		stream.read((char*)&result.state, sizeof(result.state));
		stream.read((char*)&result.id, sizeof(result.id));

		char userBuffer[STRING_FIELD_SIZE + 1];
		userBuffer[STRING_FIELD_SIZE] = 0;
		stream.read(userBuffer, STRING_FIELD_SIZE);
		result.user = std::string(userBuffer);

		stream.read((char*)result.hash, sizeof(result.hash));
		stream.read((char*)&result.creationTime, sizeof(result.creationTime));
		stream.read((char*)&result.expirationTime, sizeof(result.expirationTime));

		time_t now;
		time(&now);

		if(licenseSigned) {
			result.decodedPayload = decode(result.encodedPayload.substr(0, result.encodedPayload.size() - SIGNATURE_SIZE));
		} else {
			result.decodedPayload = decode(result.encodedPayload);
		}

		if (memcmp(actualProductField, expectedProductField, STRING_FIELD_SIZE) != 0)
		{
			result.validity = Validity_INVALID_PRODUCT;
		}
		else if (result.state < activeState)
		{
			result.validity = Validity_INACTIVE;
		}
		else if (result.creationTime > now + 86400) // 24 hours
		{
			result.validity = Validity_BEGINS_IN_FUTURE;
		}
		else if (result.expirationTime < now)
		{
			result.validity = Validity_EXPIRED;
		}
		else if (result.expirationTime > now + 7776000) // 90 Days
		{
			result.validity = Validity_EXPIRATION_TOO_FAR;
		}
		else
		{
			result.validity = Validity_OK;
		}

		return result;
	}

	std::string validityToString(Validity validity)
	{
		switch (validity)
		{
		case Validity_OK:
			return "OK";
		case Validity_MISSING:
			return "MISSING";
		case Validity_EXPIRED:
			return "EXPIRED";
		case Validity_INACTIVE:
			return "INACTIVE";
		case Validity_BEGINS_IN_FUTURE:
			return "BEGINS_IN_FUTURE";
		case Validity_EXPIRATION_TOO_FAR:
			return "EXPIRATION_TOO_FAR";
		case Validity_INVALID_FORMAT:
			return "INVALID_FORMAT";
		case Validity_INVALID_PRODUCT:
			return "INVALID_PRODUCT";
		case Validity_PARSE_ERROR:
			return "PARSE_ERROR";
		case Validity_UNKNOWN_ERROR:
		default:
			return "UNKNOWN_ERROR";
		}
	}

	std::string decode(std::string payload) {
		std::istrstream input(payload.c_str(), payload.size());
		std::stringstream output;
		base64::decoder D;
		D.decode(input, output);
		return output.str();
	}

	bool verifySignature(std::string licenseKey)
	{
		int delimiterLength = 3;
		std::size_t delimeterLocation = std::size_t(licenseKey.size() - SIGNATURE_SIZE);
		std::string signature = licenseKey.substr(delimeterLocation + delimiterLength);
		licenseKey = licenseKey.substr(0, delimeterLocation);

		std::string publicKey = decode(PUBLIC_KEY);
		signature = decode(signature);

		return ed25519_verify(
			(unsigned char * ) signature.data(),
			(unsigned char * ) licenseKey.data(),
			licenseKey.size(),
			(unsigned char * )publicKey.data()
		);
	}

	std::string hashToString(const uint8_t *hash)
	{
		static const char CHARS[] = "0123456789abcdef";

		std::string result;
		result.reserve(HASH_SIZE * 2);

		for (int i = 0; i < HASH_SIZE; i++)
		{
			result.push_back(CHARS[hash[i] >> 4]);
			result.push_back(CHARS[hash[i] & 0xf]);
		}

		return result;
	}

	std::string timeToString(int64_t time)
	{
		tm tm;
		if (!localtime_threadsafe(&tm, (time_t)time))
		{
			return "(invalid time)";
		}
		char buffer[20];
		strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &tm);
		return std::string(buffer);
	}
}
