/**
 * \file desfireiso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire ISO7816 commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav2iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samav2chip.hpp"
#include "samcrypto.hpp"
#include "samav2keyentry.hpp"
#include <openssl/rand.h>

#include <cstring>

namespace logicalaccess
{
	SAMAV2ISO7816Commands::SAMAV2ISO7816Commands()
	{
	}

	SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands()
	{
	}

	void		SAMAV2ISO7816Commands::GetVersion()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		SAMVersion	info;

		std::cout << "GetVersion Commands Called" << std::endl;

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x60, 0x00, 0x00, 0x00, result, &resultlen);


		if (resultlen == 33 && result[31] == 0x90 && result[32] == 0x00)
		{
			memcpy(&info, result, sizeof(info));
			
			if (info.hardware.vendorid == 0x04)
				std::cout << "Vendor: NXP" << std::endl;
			if (info.hardware.majorversion == 0x03)
				std::cout << "Major version: T1AD2060" << std::endl;
			if (info.hardware.minorversion == 0x04)
				std::cout << "Major version: T1AR1070" << std::endl;
			std::cout << "Storage size: " << (unsigned int)info.hardware.storagesize << std::endl;
			std::cout << "Communication protocol type : " << (unsigned int)info.hardware.protocoltype << std::endl;
		}
		else
			std::cout << "GetVersion has failed" << std::endl;
		std::cout << "Called Done" << std::endl;
	}

	boost::shared_ptr<SAMAV2KeyEntry>	SAMAV2ISO7816Commands::GetKeyEntry(unsigned int keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		boost::shared_ptr<SAMAV2KeyEntry> keyentry;
		boost::shared_ptr<KeyEntryInformation> keyentryinformation(new KeyEntryInformation);

		std::cout << "GetKeyEntry Commands Called" << std::endl;

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x64, keyno, 0x00, 0x00, result, &resultlen);

		if (resultlen >= 14 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
		{
			keyentry.reset(new SAMAV2KeyEntry());
			unsigned short set;

			set = result[resultlen - 4];
			set <<= 8;
			set += result[resultlen - 3];
			keyentry->setSET(set);

			keyentryinformation->kuc = result[resultlen - 5];
			keyentryinformation->cekv =  result[resultlen - 6];
			keyentryinformation->cekno =  result[resultlen - 7];
			keyentryinformation->desfirekeyno =  result[resultlen - 8];

			memcpy(keyentryinformation->desfireAid, result + resultlen - 11, 3);

			keyentryinformation->verc =  result[resultlen - 12];
			keyentryinformation->verb =  result[resultlen - 13];
			keyentryinformation->vera =  result[resultlen - 14];

			keyentry->setKeyEntryInformation(keyentryinformation);

			std::cout << "GetKeyEntry SUCCED" << std::endl;
		}
		else
			std::cout << "GetKeyEntry FAILED" << std::endl;
		std::cout << "Called Done" << std::endl;
		return keyentry;
	}

	void		SAMAV2ISO7816Commands::ChangeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KeyEntry> key)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		std::cout << "ChangeKeyEntry Commands Called" << std::endl;

		unsigned char proMas = 0;

		//proMas = 0x81;

		proMas = key->getUpdateMask();


		unsigned char data[60];
		memset(data, 0, 60);

		if (key->getKeyType() == SAMAV2_DES_KEY_SIZE)
		{
			memcpy(data, key->getKeyA(), 16);
			memcpy(data + 16, key->getKeyB(), 16);
			memcpy(data + 32, key->getKeyC(), 16);
		}
		//data[48] = 
		memcpy(data + 48, key->getKeyEntryInformation().get(), sizeof(KeyEntryInformation));
		
	//	data[15] = 1;
	//	data[57] = 1;
		//data[54] = 0xff;
		std::vector<unsigned char> iv;
		iv.resize(16, 0x00);

		std::vector<unsigned char> vectordata(data, data + 60);
		
		std::vector<unsigned char> encdatalittle = DESFireCrypto::desfire_encrypt(sessionkey, vectordata);


		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xc1, 0x01, proMas, 0x40, &encdatalittle[0], encdatalittle.size(), result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "SUCCED !!" << std::endl;
		else
			std::cout << "Failed :(" << std::endl;

		std::cout << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;

		std::cout << "Called Done" << std::endl;
	}


	void		SAMAV2ISO7816Commands::ActiveAV2Mode()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		std::cout << std::endl << "ActiveAV2Mode Commands Called" << std::endl;

		unsigned char data[5];
		memset(data, 0, sizeof(data));
		unsigned char p1 = 3;
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x10, p1, 0x00, 0x05, data,  0x05, 0x00, result, &resultlen);

		std::cout << resultlen << std::endl;

		
		std::cout << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;

		std::cout << "Called Done" << std::endl;
	}

	void			SAMAV2ISO7816Commands::AuthentificationHost()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		std::cout << std::endl << "AuthentificationHost Commands Called" << std::endl;
		unsigned char data[2];
		memset(data, 0, sizeof(data));
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xa4, 0x00, 0x00, 0x02, data, 0x02, 0x00, result, &resultlen);

		std::cout << resultlen << std::endl;
		
		std::cout << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;

		std::vector<unsigned char> key(16);
		for (std::vector<unsigned char>::iterator it = key.begin(); it != key.end(); ++it)
			(*it) = 0;

		std::vector<unsigned char> iv(16);
		for (std::vector<unsigned char>::iterator it = iv.begin(); it != iv.end(); ++it)
			(*it) = 0;

		//get encRNB
		std::vector<unsigned char> encRNB(result, result + resultlen - 2);

		std::cout << BufferHelper::getHex(encRNB) << std::endl;
		//dec RNB
		std::vector<unsigned char> RndB;// = SAMAV2Crypto::desfire_CBC_send(key, iv, encRNB);

		std::cout << BufferHelper::getHex(RndB) << std::endl;
		//Create RNB'
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), RndB.begin() + 1, RndB.begin() + 8);
		rndB1.push_back(RndB[0]);

		std::cout << BufferHelper::getHex(rndB1) << std::endl;

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		//Create our RndA
		std::vector<unsigned char>  rndA(8);
		if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		//create rndAB
		std::vector<unsigned char> rndAB;
		rndAB.clear();
		rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		std::cout << BufferHelper::getHex(rndAB) << std::endl;
		//enc rndAB
		std::vector<unsigned char> encRndAB;// = SAMAV2Crypto::desfire_CBC_send(key, iv, rndAB);

		unsigned char encRndABdata[16];
		for (int i = 0; i < 16; ++i)
			encRndABdata[i] = encRndAB[i];
		//send enc rndAB
		resultlen = sizeof(result);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xa4, 0x00, 0x00, 0x10, encRndABdata, 16, 0x00, result, &resultlen);

		std::cout << resultlen << std::endl;
		
		std::cout << "YOLO P2: " << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;

		std::vector<unsigned char> encRNA(result, result + resultlen - 2);
		std::vector<unsigned char> dencRndA;// = SAMAV2Crypto::desfire_CBC_send(key, iv, encRNA);

		//create rndA'
		std::vector<unsigned char> rndA1;
		rndA1.insert(rndA1.end(), rndA.begin() + 1, rndA.begin() + 8);
		rndA1.push_back(rndA[0]);
		
		std::cout << BufferHelper::getHex(dencRndA) << std::endl;
		std::cout << BufferHelper::getHex(rndA1) << std::endl;


		//Check if RNDA is our
		if (std::equal(dencRndA.begin(), dencRndA.end(), rndA1.begin()))
			std::cout << "AuthentificationHost SUCCES !" << std::endl;


		sessionkey.clear();

		sessionkey.insert(sessionkey.end(), rndA.begin(), rndA.begin() + 4);
		sessionkey.insert(sessionkey.end(), RndB.begin(), RndB.begin() + 4);
		sessionkey.insert(sessionkey.end(), rndA.begin(), rndA.begin() + 4);
		sessionkey.insert(sessionkey.end(), RndB.begin(), RndB.begin() + 4);

		std::cout << "Called Done" << std::endl;
	}

	std::string			SAMAV2ISO7816Commands::GetSAMTypeFromSAM()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x60, 0x00, 0x00, 0x00, result, &resultlen);

		if (resultlen > 3)
		{
			if (result[resultlen - 3] == 0xA1)
			{
				std::cout << "GetVersion say that the SAM is AV1" << std::endl;
				return "SAM_AV1";
			}
			else if (result[resultlen - 3] == 0xA2)
			{
				std::cout << "GetVersion say that the SAM is AV2" << std::endl;
				return "SAM_AV2";
			}
		}
		return "SAM_NONE";
	}
}
