/**
 * \file SAMAV1ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1ISO7816Commands commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav1iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samav1chip.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samkucentry.hpp"
#include "samcommands.hpp"
#include <openssl/rand.h>
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/cmac.hpp"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/thread/thread_time.hpp>
#include <cstring>

namespace logicalaccess
{
	SAMAV1ISO7816Commands::SAMAV1ISO7816Commands()	{ }

	SAMAV1ISO7816Commands::~SAMAV1ISO7816Commands()	{ }

	boost::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1> >	SAMAV1ISO7816Commands::getKeyEntry(unsigned char keyno)
	{
		std::vector<unsigned char> result;
		boost::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1> > keyentry;
		KeyEntryAV1Information keyentryinformation;

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x64, keyno, 0x00, 0x00);

		if ((result.size() == 14 || result.size() == 13) &&  result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
		{
			keyentry.reset(new SAMKeyEntry<KeyEntryAV1Information, SETAV1>());

			memcpy(keyentryinformation.set, &result[result.size() - 4], 2);
			keyentry->setSET(keyentryinformation.set);

			keyentryinformation.kuc = result[result.size() - 5];
			keyentryinformation.cekv = result[result.size() - 6];
			keyentryinformation.cekno = result[result.size() - 7];
			keyentryinformation.desfirekeyno =  result[result.size() - 8];

			memcpy(keyentryinformation.desfireAid, &result[result.size() - 11], 3);


			if (result.size() == 13)
			{
				keyentryinformation.verb = result[result.size() - 12];
				keyentryinformation.vera = result[result.size() - 13];
			}
			else
			{
				keyentryinformation.verc = result[result.size() - 12];
				keyentryinformation.verb = result[result.size() - 13];
				keyentryinformation.vera = result[result.size() - 14];
			}

			keyentry->setKeyEntryInformation(keyentryinformation);
			keyentry->setKeyTypeFromSET();
			keyentry->setUpdateMask(0);
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKeyEntry failed.");
		return keyentry;
	}

	void SAMAV1ISO7816Commands::changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1> > keyentry, boost::shared_ptr<DESFireKey> key)
	{
		if (d_crypto->d_sessionKey.size() == 0)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: AuthentificationHost have to be done before use such command.");

		std::vector<unsigned char> result;

		unsigned char proMas = 0;
		proMas = keyentry->getUpdateMask();

		size_t buffer_size = keyentry->getLength() + sizeof(KeyEntryAV1Information);
		unsigned char *data = new unsigned char[buffer_size];
		memset(data, 0, buffer_size);

		memcpy(data, &*(keyentry->getData()), keyentry->getLength());
		memcpy(data + 48, &keyentry->getKeyEntryInformation(), sizeof(KeyEntryAV1Information));
		
		std::vector<unsigned char> iv;
		iv.resize(16, 0x00);

		std::vector<unsigned char> vectordata(data, data + buffer_size);
		
		std::vector<unsigned char> encdatalittle;
		
		if (key->getKeyType() == DF_KEY_DES)
			encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
		else
			encdatalittle = d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xc1, keyno, proMas, (unsigned char)(encdatalittle.size()), encdatalittle);

		if (result.size() >= 2 &&  (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntry failed.");
	}

	void SAMAV1ISO7816Commands::authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		if (key->getKeyType() == DF_KEY_DES)
			authentificateHostDES(key, keyno);
		else
			authentificateHost_AES_3K3DES(key, keyno);
	}

	void SAMAV1ISO7816Commands::authentificateHost_AES_3K3DES(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		std::vector<unsigned char> result, data;
		unsigned char authMode = 0x00;

		data.push_back(keyno);
		data.push_back(key->getKeyVersion());

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, authMode, 0x00, 0x02, data, 0x00);
		if (result.size() >= 2 &&  (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0xaf))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost_AES_3K3DES P1 failed.");

		std::vector<unsigned char> encRndB(result.begin(), result.end() - 2);
		std::vector<unsigned char> encRndAB = d_crypto->authenticateHostP1(key, encRndB, keyno);

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), encRndAB, 0x00);
		if (result.size() >= 2 &&  (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost_AES_3K3DES P2 failed.");

		std::vector<unsigned char> encRndA1(result.begin(), result.end() - 2);
		d_crypto->authenticateHostP2(keyno, encRndA1, key);
	}

	void SAMAV1ISO7816Commands::authentificateHostDES(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		std::vector<unsigned char> result, data;
		unsigned char authMode = 0x00;
		size_t keylength = key->getLength();

		data.push_back(keyno);
		data.push_back(key->getKeyVersion());

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, authMode, 0x00, 0x02, data, 0x00);

		if (result.size() >= 2 &&  (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0xaf))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHostDES P1 failed.");

		std::vector<unsigned char> keyvec(key->getData(), key->getData() + keylength);
		
		std::vector<unsigned char> iv(keylength);
		memset(&iv[0], 0, keylength);

		//get encRNB
		std::vector<unsigned char> encRNB(result.begin(), result.end() - 2);

		//dec RNB
		std::vector<unsigned char> RndB =  d_crypto->desfire_CBC_send(keyvec, iv, encRNB);

		//Create RNB'
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), RndB.begin() + 1, RndB.begin() + RndB.size());
		rndB1.push_back(RndB[0]);

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		//Create our RndA
		std::vector<unsigned char>  rndA(8);
		if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		//create rndAB
		std::vector<unsigned char> rndAB;
		rndAB.clear();
		rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		//enc rndAB
		std::vector<unsigned char> encRndAB =  d_crypto->desfire_CBC_send(keyvec, iv, rndAB);

		//send enc rndAB
		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), encRndAB, 0x00);
		if (result.size() >= 2 &&  (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHostDES P2 failed.");


		std::vector<unsigned char> encRndA1(result.begin(), result.end() - 2);
		std::vector<unsigned char> dencRndA1 =  d_crypto->desfire_CBC_send(keyvec, iv, encRndA1);

		//create rndA'
		std::vector<unsigned char> rndA1;
		rndA1.insert(rndA1.end(), rndA.begin() + 1, rndA.begin() + rndA.size());
		rndA1.push_back(rndA[0]);
		
		//Check if RNDA is our
		if (!std::equal(dencRndA1.begin(), dencRndA1.end(), rndA1.begin()))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHostDES Final Check failed.");

		d_crypto->d_sessionKey.clear();

		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(), rndA.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(), RndB.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(), rndA.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(), RndB.begin() + 4);
	}
	 
	void SAMAV1ISO7816Commands::changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> kucentry, boost::shared_ptr<DESFireKey> key)
	{
		if (d_crypto->d_sessionKey.size() == 0)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: AuthentificationHost have to be done before use such command.");

		std::vector<unsigned char> result;

		unsigned char data[6] = {};
		memcpy(data, &kucentry->getKucEntryStruct(), 6);
		std::vector<unsigned char> vectordata(data, data + 6);
		std::vector<unsigned char> encdatalittle;

		if (key->getKeyType() == DF_KEY_DES)
			encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
		else
			encdatalittle = d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

		int proMas = kucentry->getUpdateMask();

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xcc, kucno, proMas, 0x08, encdatalittle);

		if (result.size() >= 2 && (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntry failed.");
	}


	std::vector<unsigned char> SAMAV1ISO7816Commands::decipherData(std::vector<unsigned char> data, bool islastdata)
	{
		std::vector<unsigned char> result;
		unsigned char p1 = 0x00;
		std::vector<unsigned char> datawithlength(3);

		if (!islastdata)
			p1 = 0xaf;
		else
		{
			datawithlength[0] = (unsigned char)(data.size() & 0xff0000);
			datawithlength[1] = (unsigned char)(data.size() & 0x00ff00);
			datawithlength[2] = (unsigned char)(data.size() & 0x0000ff);
		}
		datawithlength.insert(datawithlength.end(), data.begin(), data.end());

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xdd, p1, 0x00, (unsigned char)(datawithlength.size()), datawithlength, 0x00);

		if (result.size() >= 2 &&  result[result.size() - 2] != 0x90 &&
			((p1 == 0x00 && result[result.size() - 1] != 0x00) || (p1 == 0xaf && result[result.size() - 1] != 0xaf)))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "decipherData failed.");

		return std::vector<unsigned char>(result.begin(), result.end() - 2);
	}

	std::vector<unsigned char> SAMAV1ISO7816Commands::encipherData(std::vector<unsigned char> data, bool islastdata)
	{
		std::vector<unsigned char> result;
		unsigned char p1 = 0x00;

		if (!islastdata)
			p1 = 0xaf;
		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xed, p1, 0x00, (unsigned char)(data.size()), data, 0x00);

		if (result.size() >= 2 &&  result[result.size() - 2] != 0x90 &&
			((p1 == 0x00 && result[result.size() - 1] != 0x00) || (p1 == 0xaf && result[result.size() - 1] != 0xaf)))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "encipherData failed.");

		return std::vector<unsigned char>(result.begin(), result.end() - 2);
	}


	std::vector<unsigned char> SAMAV1ISO7816Commands::changeKeyPICC(const ChangeKeyInfo& info)
	{
		//if (d_crypto->d_sessionKey.size() == 0)
		//	THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: Authentification have to be done before using such command.");

		unsigned char keyCompMeth = 0;
		std::vector<unsigned char> result;

		if (!info.oldKeyInvolvement)
			keyCompMeth = 1;

		unsigned char cfg = info.desfireNumber & 0xf;
		if (info.isMasterKey)
			cfg |= 0x10;
		std::vector<unsigned char> data(4);
		data[0] = info.currentKeyNo;
		data[1] = info.currentKeyV;
		data[2] = info.newKeyNo;
		data[3] = info.newKeyV;
		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xc4, keyCompMeth, cfg, (unsigned char)(data.size()), data, 0x00);

		if (result.size() >= 2 &&  (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
		{
			char tmp[64];
			sprintf(tmp, "changeKeyPICC failed (%x %x).", result[result.size() - 2], result[result.size() - 1]);
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmp);
		}

		return std::vector<unsigned char>(result.begin(), result.end() - 2);
	}
}
