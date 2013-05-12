/**
 * \file mifareultralightcpcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C PC/SC commands.
 */

#include "../commands/mifareultralightcpcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifareultralightcchip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

#include <openssl/rand.h>
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"


namespace logicalaccess
{	
	MifareUltralightCPCSCCommands::MifareUltralightCPCSCCommands()
		: MifareUltralightPCSCCommands()
	{
		
	}

	MifareUltralightCPCSCCommands::~MifareUltralightCPCSCCommands()
	{
		
	}

	void MifareUltralightCPCSCCommands::authenticate(boost::shared_ptr<TripleDESKey> authkey)
	{
		unsigned char result[256];
		size_t resultlen = 256;

		// Get RndB from the PICC
		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x1A, 0x00, 0x00, result, &resultlen);
		EXCEPTION_ASSERT_WITH_LOG((resultlen - 2) >= 8, CardException, "Authentication failed. The PICC return a bad buffer.");

		openssl::DESCipher cipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);
		unsigned char* keydata = authkey->getData();
		openssl::DESSymmetricKey deskey(openssl::DESSymmetricKey::createFromData(std::vector<unsigned char>(keydata, keydata + authkey->getLength())));
		openssl::DESInitializationVector desiv = openssl::DESInitializationVector::createNull();

		std::vector<unsigned char> encRndB(result, result + resultlen - 2);
		std::vector<unsigned char> rndB;
		cipher.decipher(encRndB, rndB, deskey, desiv, false);
		
		RAND_seed(result, 8);
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		std::vector<unsigned char> rndA;
		rndA.resize(8);
		if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> rndAB;
		rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
		rndAB.insert(rndAB.end(), rndB.begin(), rndB.end());

		std::vector<unsigned char> encRndAB;
		cipher.cipher(rndAB, encRndAB, deskey, desiv, false);

		// Send Ek(RndAB) to the PICC and get RndA'
		resultlen = 256;
		getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xAF, 0x00, 0x00, static_cast<unsigned char>(encRndAB.size()), &encRndAB[0], encRndAB.size(), result, &resultlen);
		EXCEPTION_ASSERT_WITH_LOG((resultlen - 2) >= 8, CardException, "Authentication failed. The PICC return a bad buffer.");

		std::vector<unsigned char> encRndA1(result, result + resultlen - 2);
		std::vector<unsigned char> rndA1;
		cipher.decipher(encRndA1, rndA1, deskey, desiv, false);

		EXCEPTION_ASSERT_WITH_LOG(rndA == rndA1, CardException, "Authentication failed. RndA' != RndA.");
	}
}

