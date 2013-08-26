/**
 * \file desfireev1iso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireEV1 ISO7816 commands.
 */

#include "../commands/desfireev1iso7816commands.hpp"
#include "desfirechip.hpp"
#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "desfireev1location.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"


namespace logicalaccess
{
	DESFireEV1ISO7816Commands::DESFireEV1ISO7816Commands()
		: DESFireISO7816Commands()
	{

	}

	DESFireEV1ISO7816Commands::~DESFireEV1ISO7816Commands()
	{

	}

	unsigned int DESFireEV1ISO7816Commands::getFreeMem()
	{
		unsigned int freemem = 0;
		std::vector<unsigned char> r = transmit(DFEV1_INS_FREE_MEM);
		r.resize(r.size() - 2);

		if (r.size() == 3)
		{
			freemem = (r[0] << 16)	|
					  (r[1] << 8)	|
					  r[2];
		}

		return freemem;
	}

	vector<DFName> DESFireEV1ISO7816Commands::getDFNames()
	{
		vector<DFName> dfnames;		
		unsigned char err;
		unsigned char cmd = DFEV1_INS_GET_DF_NAMES;

		do
		{
			std::vector<unsigned char> r = transmit(cmd);
			err = r[0];
			r.resize(r.size() - 2);
			cmd = DF_INS_ADDITIONAL_FRAME;

			DFName dfname;
			memset(&dfname, 0x00, sizeof(dfname));
			dfname.AID = DESFireLocation::convertAidToInt(&r[0]);
			dfname.FID = static_cast<unsigned short>((r[3] << 8) | r[4]);
			memcpy(dfname.DF_Name, &r[5], r.size() - 5);
			dfnames.push_back(dfname);

		} while (err == DF_INS_ADDITIONAL_FRAME);

		return dfnames;
	}

	vector<unsigned short> DESFireEV1ISO7816Commands::getISOFileIDs()
	{
		vector<unsigned short> fileids;		
		unsigned char err;
		unsigned char cmd = DFEV1_INS_GET_ISO_FILE_IDS;
		std::vector<unsigned char> fullr;

		do
		{
			std::vector<unsigned char> r = transmit(cmd);
			err = r[0];
			r.resize(r.size() - 2);
			cmd = DF_INS_ADDITIONAL_FRAME;
			
			fullr.insert(r.end(), r.begin(), r.end());

		} while (err == DF_INS_ADDITIONAL_FRAME);

		for (unsigned int i = 0; i < fullr.size(); i += 2)
		{
			unsigned short fid = static_cast<unsigned short>((fullr[i] << 8) | fullr[i+1]);
			fileids.push_back(fid);
		}

		return fileids;
	}

	bool DESFireEV1ISO7816Commands::createApplication(int aid, DESFireKeySettings settings, size_t maxNbKeys, FidSupport fidSupported, DESFireKeyType cryptoMethod, unsigned short isoFID, const char* isoDFName)
	{
		unsigned char command[128];
		unsigned int commandlen = 0;

		DESFireLocation::convertIntToAid(aid, command);
		commandlen += 3;
		command[commandlen++] = static_cast<unsigned char>(settings);
		command[commandlen++] = static_cast<unsigned char>((static_cast<unsigned char>(maxNbKeys) & 0x0f) | fidSupported | cryptoMethod);

		if (isoFID != 0x00)
		{
			command[commandlen++] = static_cast<unsigned char>(isoFID);
		}
		if (isoDFName != NULL)
		{
			memcpy(&command[commandlen++], isoDFName, strlen(isoDFName));
		}

		transmit(DF_INS_CREATE_APPLICATION, command, commandlen);
		d_crypto->createApplication(aid, maxNbKeys, cryptoMethod);
		
		return true;
	}

	void DESFireEV1ISO7816Commands::getKeySettings(DESFireKeySettings& settings, unsigned int& maxNbKeys, DESFireKeyType& keyType)
	{
		std::vector<unsigned char> r = transmit(DF_INS_GET_KEY_SETTINGS);
		r.resize(r.size() - 2);


		settings = static_cast<DESFireKeySettings>(r[0]);
		maxNbKeys = r[1] & 0x0F;
		
		if ((r[1] & DF_KEY_3K3DES) == DF_KEY_3K3DES)
		{
			keyType = DF_KEY_3K3DES;
		}
		else if ((r[1] & DF_KEY_AES) == DF_KEY_AES)
		{
			keyType = DF_KEY_AES;
		}
		else
		{
			keyType = DF_KEY_DES;
		}
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::getCardUID()
	{		
		std::vector<unsigned char> result = transmit_nomacv(DFEV1_INS_GET_CARD_UID);
		result.resize(result.size() - 2);

		return d_crypto->desfire_iso_decrypt(result, 7);
	}

	bool DESFireEV1ISO7816Commands::createStdDataFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, unsigned short isoFID)
	{
		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		if (isoFID != 0)
		{
			command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
			command.push_back(static_cast<unsigned char>(isoFID & 0xff));			
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

		DESFireISO7816Commands::transmit(DF_INS_CREATE_STD_DATA_FILE, command);

		return true;
	}

	bool DESFireEV1ISO7816Commands::createBackupFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, unsigned short isoFID)
	{
		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		if (isoFID != 0)
		{
			command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
			command.push_back(static_cast<unsigned char>(isoFID & 0xff));			
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));

		DESFireISO7816Commands::transmit(DF_INS_CREATE_BACKUP_DATA_FILE, command);

		return true;

	}

	bool DESFireEV1ISO7816Commands::createLinearRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords, unsigned short isoFID)
	{
		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		if (isoFID != 0)
		{
			command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
			command.push_back(static_cast<unsigned char>(isoFID & 0xff));			
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
		command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

		DESFireISO7816Commands::transmit(DF_INS_CREATE_LINEAR_RECORD_FILE, command);

		return true;
	}

	bool DESFireEV1ISO7816Commands::createCyclicRecordFile(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, int fileSize, int maxNumberOfRecords, unsigned short isoFID)
	{
		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);

		command.push_back(static_cast<unsigned char>(fileno));
		if (isoFID != 0)
		{
			command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(isoFID & 0xff00) >> 8));
			command.push_back(static_cast<unsigned char>(isoFID & 0xff));			
		}
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		command.push_back(static_cast<unsigned char>(fileSize & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fileSize & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(fileSize & 0xff0000) >> 16));
		command.push_back(static_cast<unsigned char>(maxNumberOfRecords & 0xff));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned short>(maxNumberOfRecords & 0xff00) >> 8));
		command.push_back(static_cast<unsigned char>(static_cast<unsigned int>(maxNumberOfRecords & 0xff0000) >> 16));

		DESFireISO7816Commands::transmit(DF_INS_CREATE_CYCLIC_RECORD_FILE, command[0]);

		return true;
	}

	void DESFireEV1ISO7816Commands::iso_selectFile(unsigned short fid)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(static_cast<unsigned short>(fid & 0xff00) >> 8));
		data.push_back(static_cast<unsigned char>(fid & 0xff));

		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, 0x00, 0x0C, static_cast<unsigned char>(data.size()), &data[0], data.size(), result, &resultlen);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::iso_readRecords(unsigned short fid, unsigned char start_record, DESFireRecords record_number)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char p1 = start_record;
		unsigned char p2 = record_number & 0x0f;
		if (fid != 0)
		{
			p2 += static_cast<unsigned char>((fid & 0xff) << 3);
		}

		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_READ_RECORDS, p1, p2, 0x00, result, &resultlen);

		std::vector<unsigned char> ret(result, result + resultlen - 2);
		return ret;
	}

	void DESFireEV1ISO7816Commands::iso_appendrecord(const std::vector<unsigned char>& data, unsigned short fid)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char p1 = 0x00;
		unsigned char p2 = 0x00;
		if (fid != 0)
		{
			p2 += static_cast<unsigned char>((fid & 0xff) << 3);
		}

		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_APPEND_RECORD, p1, p2, static_cast<unsigned char>(data.size()), &data[0], data.size(), result, &resultlen);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::iso_getChallenge(unsigned int length)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_CHALLENGE, 0x00, 0x00, static_cast<unsigned char>(length), result, &resultlen);

		std::vector<unsigned char> ret(result, result + resultlen - 2);

		return ret;
	}

	void DESFireEV1ISO7816Commands::iso_externalAuthenticate(DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno, const std::vector<unsigned char>& data)
	{
		unsigned char p1 = static_cast<unsigned char>(algorithm);
		unsigned char p2 = 0x00;
		if (!isMasterCardKey)
		{
			p2 = static_cast<unsigned char>(0x80 + (keyno & 0x0F));
		}

		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_EXTERNAL_AUTHENTICATE, p1, p2, static_cast<unsigned char>(data.size()), &data[0], static_cast<unsigned char>(data.size()));
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::iso_internalAuthenticate(DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno, const std::vector<unsigned char>& RPCD2, unsigned int length)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char p1 = static_cast<unsigned char>(algorithm);
		unsigned char p2 = 0x00;
		if (!isMasterCardKey)
		{
			p2 = static_cast<unsigned char>(0x80 + (keyno & 0x0F));
		}

		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_INTERNAL_AUTHENTICATE, p1, p2, static_cast<unsigned char>(RPCD2.size()), &RPCD2[0], RPCD2.size(), static_cast<unsigned char>(length), result, &resultlen);

		std::vector<unsigned char> ret(result, result + resultlen - 2);

		return ret;
	}	

	bool DESFireEV1ISO7816Commands::authenticate(unsigned char keyno)
	{
		// Get the appropriate authentification method and algorithm according to the key type (for 3DES we use legacy method instead of ISO).
		bool r = false;
		
		boost::shared_ptr<DESFireKey> key = d_crypto->getKey(keyno);

		if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && !getSAMChip())
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "SAMKeyStorage set on the key but not SAM reader has been set.");


		if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()) && key->getKeyType() != DF_KEY_DES)
		{
			if (key->getKeyType() == DF_KEY_3K3DES)
				sam_iso_authenticate(key, DF_ALG_3K3DES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
			else
				sam_iso_authenticate(key, DF_ALG_AES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
			return true;
		}

		switch (key->getKeyType())
		{
		case DF_KEY_DES:
			r = DESFireISO7816Commands::authenticate(keyno);
			break;

		case DF_KEY_3K3DES:
			iso_authenticate(key, DF_ALG_3K3DES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
			r = true;
			break;

		case DF_KEY_AES:
			iso_authenticate(key, DF_ALG_AES, (d_crypto->d_currentAid == 0 && keyno == 0), keyno);
			r = true;
			break;
		}

		return r;
	}

	void DESFireEV1ISO7816Commands::sam_iso_authenticate(boost::shared_ptr<DESFireKey> key, DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno)
	{
		boost::shared_ptr<SAMCommands> samcommands = boost::dynamic_pointer_cast<SAMCommands>(getSAMChip()->getCommands());
		boost::shared_ptr<ISO7816ReaderCardAdapter> readercardadapter = boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(samcommands->getReaderCardAdapter());
		unsigned char apduresult[255];
		size_t apduresultlen = sizeof(apduresult);

		std::vector<unsigned char> RPICC1 = iso_getChallenge(16);

		std::vector<unsigned char> data(2 + RPICC1.size());
		data[0] = keyno;
		data[1] = key->getKeyVersion();
		memcpy(&data[0] + 2, &RPICC1[0], RPICC1.size());

		readercardadapter->sendAPDUCommand(0x80, 0x8e, 0x02, 0x00, (unsigned char)(data.size()), &data[0], data.size(), 0x00, apduresult, &apduresultlen);
		if (apduresultlen <= 2 && apduresult[apduresultlen - 2] != 0x90 && apduresult[apduresultlen - 2] != 0xaf)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam_iso_authenticate P1 failed.");

		std::vector<unsigned char> encRPCD1RPICC1(apduresult, apduresult + apduresultlen - 2 - 16);
		std::vector<unsigned char> RPCD2(apduresult + apduresultlen - 16 - 2, apduresult + apduresultlen - 2);

		iso_externalAuthenticate(algorithm, isMasterCardKey, keyno, encRPCD1RPICC1);
		std::vector<unsigned char> encRPICC2RPCD2a = iso_internalAuthenticate(algorithm, isMasterCardKey, keyno, RPCD2, 2 * 16);

		readercardadapter->sendAPDUCommand(0x80, 0x8e, 0x00, 0x00, (unsigned char)(encRPICC2RPCD2a.size()), &encRPICC2RPCD2a[0], encRPICC2RPCD2a.size(), apduresult, &apduresultlen);
		if (apduresultlen <= 2 && apduresult[apduresultlen - 2] != 0x90 && apduresult[apduresultlen - 2] != 0x00)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "sam_iso_authenticate P2 failed.");

		d_crypto->d_currentKeyNo = keyno;
		d_crypto->d_sessionKey = samcommands->dumpSessionKey();
		d_crypto->d_auth_method = CM_ISO;

		INFO_("Session key length: %d", d_crypto->d_sessionKey.size());

		if (key->getKeyType() == DF_KEY_3K3DES)
		{
			d_crypto->d_cipher.reset(new openssl::DESCipher());
			d_crypto->d_block_size = 8;
			d_crypto->d_mac_size = 8;
		}
		else
		{
			d_crypto->d_cipher.reset(new openssl::AESCipher());
			d_crypto->d_block_size = 16;
			d_crypto->d_mac_size = 8;
		}
		d_crypto->d_lastIV.clear();
		d_crypto->d_lastIV.resize(d_crypto->d_block_size, 0x00);
	}

	void DESFireEV1ISO7816Commands::iso_authenticate(boost::shared_ptr<DESFireKey> key, DESFireISOAlgorithm algorithm, bool isMasterCardKey, unsigned char keyno)
	{
		unsigned char le;
		boost::shared_ptr<openssl::SymmetricCipher> cipher;
		unsigned char diversify[24];
		memset(diversify, 0x00, sizeof(diversify));
		d_crypto->getDiversify(diversify);
		std::vector<unsigned char> keydiv;
		d_crypto->getKey(key, diversify, keydiv);

		if (algorithm == DF_ALG_2K3DES)
		{
			le = 8;
			cipher.reset(new openssl::DESCipher());
		}
		else if (algorithm == DF_ALG_3K3DES)
		{
			le = 16;
			cipher.reset(new openssl::DESCipher());			
		}
		else
		{
			le = 16;
			cipher.reset(new openssl::AESCipher());
		}

		boost::shared_ptr<openssl::SymmetricKey> isokey;
		boost::shared_ptr<openssl::InitializationVector> iv;
		if (boost::dynamic_pointer_cast<openssl::AESCipher>(cipher))
		{
			isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(keydiv)));
			iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createNull()));
		}
		else
		{
			isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(keydiv)));
			iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createNull()));
		}

		std::vector<unsigned char> RPICC1 = iso_getChallenge(le);

		std::vector<unsigned char> RPCD1;
		RPCD1.resize(le);
		RAND_seed(&keyno, sizeof(keyno));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		if (RAND_bytes(&RPCD1[0], static_cast<int>(RPCD1.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}
		std::vector<unsigned char> makecrypt1;
		makecrypt1.insert(makecrypt1.end(), RPCD1.begin(), RPCD1.end());
		makecrypt1.insert(makecrypt1.end(), RPICC1.begin(), RPICC1.end());
		std::vector<unsigned char> cryptogram;
		cipher->cipher(makecrypt1, cryptogram, *isokey.get(), *iv.get(), false);
		if (boost::dynamic_pointer_cast<openssl::AESCipher>(cipher))
		{
			iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(std::vector<unsigned char>(cryptogram.end() - iv->data().size(), cryptogram.end()))));
		}
		else
		{
			iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(std::vector<unsigned char>(cryptogram.end() - iv->data().size(), cryptogram.end()))));
		}
		iso_externalAuthenticate(algorithm, isMasterCardKey, keyno, cryptogram);
		
		std::vector<unsigned char> RPCD2;
		RPCD2.resize(le);
		RAND_seed(&keyno, sizeof(keyno));
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		if (RAND_bytes(&RPCD2[0], static_cast<int>(RPCD2.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}
		cryptogram = iso_internalAuthenticate(algorithm, isMasterCardKey, keyno, RPCD2, 2*le);
		std::vector<unsigned char> response;
		cipher->decipher(cryptogram, response, *isokey.get(), *iv.get(), false);

		std::vector<unsigned char> RPICC2 = std::vector<unsigned char>(response.begin(), response.begin() + le);
		std::vector<unsigned char> RPCD2a = std::vector<unsigned char>(response.begin() + le, response.end());

		EXCEPTION_ASSERT_WITH_LOG(RPCD2 == RPCD2a, LibLogicalAccessException, "Integrity error : host part of mutual authentication");

		d_crypto->d_currentKeyNo = keyno;

		d_crypto->d_sessionKey.clear();
		d_crypto->d_auth_method = CM_ISO;
		if (algorithm == DF_ALG_2K3DES)
		{
			if (std::vector<unsigned char>(keydiv.begin(), keydiv.begin() + 8) == std::vector<unsigned char>(keydiv.begin() + 8, keydiv.begin() + 8 + 8))
			{
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPCD1.begin() + 4);
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPCD1.begin() + 4);
			}
			else
			{
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPCD1.begin() + 4);
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 4, RPCD1.begin() + 8);
				d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 4, RPCD1.begin() + 8);
			}

			d_crypto->d_cipher.reset(new openssl::DESCipher());
			d_crypto->d_authkey = keydiv;
			d_crypto->d_block_size = 8;
			d_crypto->d_mac_size = 8;			
		}
		else if (algorithm == DF_ALG_3K3DES)
		{
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPICC2.begin() + 4);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 6, RPCD1.begin() + 10);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 6, RPICC2.begin() + 10);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 12, RPCD1.begin() + 16);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 12, RPICC2.begin() + 16);

			d_crypto->d_cipher.reset(new openssl::DESCipher());
			d_crypto->d_authkey = keydiv;
			d_crypto->d_block_size = 8;
			d_crypto->d_mac_size = 8;
		}
		else
		{
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin(), RPCD1.begin() + 4);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin(), RPICC2.begin() + 4);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPCD1.begin() + 12, RPCD1.begin() + 16);
			d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RPICC2.begin() + 12, RPICC2.begin() + 16);

			d_crypto->d_cipher.reset(new openssl::AESCipher());
			d_crypto->d_authkey = keydiv;
			d_crypto->d_block_size = 16;
			d_crypto->d_mac_size = 8;
		}
		d_crypto->d_lastIV.clear();
		d_crypto->d_lastIV.resize(d_crypto->d_block_size, 0x00);
	}

	void DESFireEV1ISO7816Commands::authenticateISO(unsigned char keyno, DESFireISOAlgorithm algorithm)
	{
		d_crypto->d_auth_method = CM_LEGACY; // To prevent CMAC checking

		unsigned char random_len;
		if (algorithm == DF_ALG_2K3DES)
		{
			random_len = 8;
		}
		else
		{
			random_len = 16;
		}

		std::vector<unsigned char> data;
		data.push_back(keyno);

		unsigned char diversify[24];
		d_crypto->getDiversify(diversify);

		std::vector<unsigned char> encRndB = DESFireISO7816Commands::transmit(DFEV1_INS_AUTHENTICATE_ISO, data);
		unsigned char err = encRndB.back();
		encRndB.resize(encRndB.size() - 2);
		EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException, "No additional frame for ISO authentication.");

		std::vector<unsigned char> response = d_crypto->iso_authenticate_PICC1(keyno, diversify, encRndB, random_len);
		std::vector<unsigned char> encRndA1 = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, response);
		err = encRndA1.back();
		encRndA1.resize(encRndA1.size() - 2);

		d_crypto->iso_authenticate_PICC2(keyno, encRndA1, random_len);
	}

	void DESFireEV1ISO7816Commands::authenticateAES(unsigned char keyno)
	{
		d_crypto->d_auth_method = CM_LEGACY; // To prevent CMAC checking

		std::vector<unsigned char> data;
		data.push_back(keyno);

		unsigned char diversify[24];
		d_crypto->getDiversify(diversify);

		std::vector<unsigned char> encRndB = DESFireISO7816Commands::transmit(DFEV1_INS_AUTHENTICATE_AES, data);
		unsigned char err = encRndB.back();
		encRndB.resize(encRndB.size() - 2);
		EXCEPTION_ASSERT_WITH_LOG(err == DF_INS_ADDITIONAL_FRAME, LibLogicalAccessException, "No additional frame for ISO authentication.");

		std::vector<unsigned char> response = d_crypto->aes_authenticate_PICC1(keyno, diversify, encRndB);
		std::vector<unsigned char> encRndA1 = DESFireISO7816Commands::transmit(DF_INS_ADDITIONAL_FRAME, response);
		err = encRndA1.back();
		encRndA1.resize(encRndA1.size() - 2);

		d_crypto->aes_authenticate_PICC2(keyno, encRndA1);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::handleReadData(unsigned char err, const std::vector<unsigned char>& firstMsg, size_t length, EncryptionMode mode)
	{
		std::vector<unsigned char> ret, data;

		if ((err == DF_INS_ADDITIONAL_FRAME || err == 0x00))
		{
			if (mode == CM_ENCRYPT)
			{
				d_crypto->decipherData1(length, firstMsg);
			}
			else
			{
				ret = data = firstMsg;
				if (mode == CM_MAC)
				{
					d_crypto->initBuf(length + 4);
				}
			}
		}		
		
		while (err == DF_INS_ADDITIONAL_FRAME)
		{
			data = transmit_plain(DF_INS_ADDITIONAL_FRAME);
			err = data.back();
			data.resize(data.size() - 2);
			if (mode == CM_ENCRYPT)
			{
				d_crypto->decipherData2(data);
			}
			else
			{
				ret.insert(ret.end(), data.begin(), data.end());
			}
		}

		switch (mode)
		{
		case CM_PLAIN:
			{
				if (d_crypto->d_auth_method != CM_LEGACY)
				{
					if (!d_crypto->verifyMAC(true, ret))
					{
						THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC data doesn't match.");
					}
					ret.resize(ret.size() - d_crypto->d_mac_size);
				}
			}
			break;

		case CM_MAC:
			{
				if (!d_crypto->verifyMAC(true, ret))
				{
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC data doesn't match.");
				}
				ret.resize(ret.size() - d_crypto->d_mac_size);
			}
			break;
		case CM_ENCRYPT:
			{
				ret = d_crypto->decipherData(length);
			}
			break;
		case CM_UNKNOWN:
			{
				
			}
			break;
		}

		return ret;
	}

	bool DESFireEV1ISO7816Commands::handleWriteData(unsigned char cmd, void* parameters, size_t paramLength, const std::vector<unsigned char> data, EncryptionMode mode)
	{
		std::vector<unsigned char> edata;
		unsigned char command[64];
		size_t p = 0;
		size_t pos = 0;
		size_t pkSize = 0;
		bool ret = false;

		d_crypto->initBuf(data.size());

		if (data.size() <= DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8)
		{
			switch (mode)
			{
			case CM_PLAIN:
				{
					edata = data;
					if (d_crypto->d_auth_method != CM_LEGACY) // CMAC needs to be recalculated
					{
						std::vector<unsigned char> apdu_command;
						apdu_command.push_back(cmd);
						if (parameters != NULL)
						{
							apdu_command.insert(apdu_command.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
						}
						if (data.size() > 0)
						{
							apdu_command.insert(apdu_command.end(), data.begin(), data.end());
						}
						std::vector<unsigned char> CMAC = d_crypto->desfire_cmac(apdu_command);
					}
				}
				break;

			case CM_MAC:
				{
					std::vector<unsigned char> mac;
					if (d_crypto->d_auth_method == CM_LEGACY)
					{
						mac = d_crypto->generateMAC(data);
					}
					else
					{
						std::vector<unsigned char> gdata;
						gdata.push_back(cmd);
						if (parameters != NULL)
						{
							gdata.insert(gdata.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
						}
						if (data.size() > 0)
						{
							gdata.insert(gdata.end(), data.begin(), data.end());
						}
						mac = d_crypto->generateMAC(gdata);
					}
					edata = data;

					if (d_crypto->d_auth_method == CM_LEGACY) // Only in native mode mac needs to be added
					{
						edata.insert(edata.end(), mac.begin(), mac.end());
					}
				}
				break;

			case CM_ENCRYPT:
				{
					std::vector<unsigned char> encparameters;
					encparameters.push_back(cmd);
					if (parameters != NULL)
					{
						encparameters.insert(encparameters.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
					}
					edata = d_crypto->desfireEncrypt(data, encparameters);
				}
				break;

			default:
				return false;
			}			
		}
		else
		{
			switch (mode)
			{
			case CM_PLAIN:
				edata = std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8);
				if (d_crypto->d_auth_method != CM_LEGACY) // CMAC needs to be recalculated
				{
					std::vector<unsigned char> apdu_command;
					apdu_command.push_back(cmd);
					if (parameters != NULL)
					{
						apdu_command.insert(apdu_command.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
					}
					if (data.size() > 0)
					{
						apdu_command.insert(apdu_command.end(), data.begin(), data.end());
					}
					std::vector<unsigned char> CMAC = d_crypto->desfire_cmac(apdu_command);
				}
				break;

			case CM_MAC:
				edata = std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8);
				d_crypto->bufferingForGenerateMAC(edata);
				break;

			case CM_ENCRYPT:
				edata = d_crypto->encipherData(false, std::vector<unsigned char>(data.begin(), data.begin() + DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8));
				break;

			default:
				return false;
			}			
		}

		if (parameters != NULL && paramLength > 0)
		{
			memcpy(command, parameters, paramLength);
		}
		p += paramLength;
		memcpy(&command[p], &edata[0], edata.size());
		p += edata.size();
		command[p++] = 0x00;

		std::vector<unsigned char> result = transmit_plain(cmd, command, (paramLength + edata.size()));
		unsigned char err = result[result.size() - 1];
		if (data.size() > DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8)
		{
			ret = true;
			pos += DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8;
			while (ret && err == DF_INS_ADDITIONAL_FRAME) // && pos < dataLength
			{
				pkSize = ((data.size() - pos) >= DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8) ? DESFIRE_CLEAR_DATA_LENGTH_CHUNK - 8 : (data.size() - pos);

				switch (mode)
				{
				case CM_PLAIN:
					edata =  std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize);
					break;

				case CM_MAC:
					edata =  std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize);
					if (pos + pkSize == data.size())
					{
						std::vector<unsigned char> mac;
						if (d_crypto->d_auth_method == CM_LEGACY)
						{
							mac = d_crypto->generateMAC(edata);
						}
						else
						{
							std::vector<unsigned char> gdata;
							gdata.push_back(cmd);
							if (parameters != NULL)
							{
								gdata.insert(gdata.end(), (unsigned char*)parameters, (unsigned char*)parameters + paramLength);
							}
							gdata.insert(gdata.end(), edata.begin(), edata.end());
							mac = d_crypto->generateMAC(gdata);
						}

						if (d_crypto->d_auth_method == CM_LEGACY) // Only in native mode mac needs to be added
						{
							edata.insert(edata.begin(), mac.begin(), mac.end());
						}
					}
					else
					{
						d_crypto->bufferingForGenerateMAC(edata);
					}
					break;

				case CM_ENCRYPT:
					if (pos + pkSize == data.size())
					{
						edata = d_crypto->encipherData(true, std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize));
					}
					else
					{
						edata = d_crypto->encipherData(false, std::vector<unsigned char>(data.begin() + pos, data.begin() + pos + pkSize));
					}
					break;

				default:
					return false;
				}

				result = transmit_plain(DF_INS_ADDITIONAL_FRAME, edata);
				err = result.back();

				pos += pkSize;
			}			
		}

		ret = (err == 0x00);
		if (ret && result.size() > 2 && (mode == CM_MAC || mode == CM_ENCRYPT || d_crypto->d_auth_method != CM_LEGACY))
		{
			result.resize(result.size() - 2);
			ret = d_crypto->verifyMAC(true, result);
		}

		if (!ret)
		{
			COM_("Return an error: %x", err);
		}

		return ret;
	}

	size_t DESFireEV1ISO7816Commands::readData(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode)
	{
		size_t ret = 0;

		unsigned char command[7];

		command[0] = static_cast<unsigned char>(fileno);				

		// Currently we have some problems to read more than 253 bytes with an Omnikey Reader.
		// So the read command is separated to some commands, 8 bytes aligned.

		for (size_t i = 0; i < length; i += 248)
		{			
			size_t trunloffset = offset + i;
			size_t trunklength = ((length - i) > 248) ? 248 : (length - i);			
			memcpy(&command[1], &trunloffset, 3);
			memcpy(&command[4], &trunklength, 3);			

			std::vector<unsigned char> result = transmit_nomacv(DF_INS_READ_DATA, command, sizeof(command));
			unsigned char err = result.back();
			result.resize(result.size() - 2);
			result = handleReadData(err, result, trunklength, mode);
			memcpy(reinterpret_cast<unsigned char*>(data) + i, &result[0], result.size());
			ret += result.size();
		}

		return ret;
	}

	size_t DESFireEV1ISO7816Commands::readRecords(int fileno, size_t offset, size_t length, void* data, EncryptionMode mode)
	{
		unsigned char command[7];

		command[0] = static_cast<unsigned char>(fileno);
		memcpy(&command[1], &offset, 3);
		memcpy(&command[4], &length, 3);

		std::vector<unsigned char> result = transmit_nomacv(DF_INS_READ_RECORDS, command, sizeof(command));
		unsigned char err = result.back();
		result.resize(result.size() - 2);
		result = handleReadData(err, result, length, mode);
		memcpy(data, &result[0], result.size());
		
		return result.size();
	}

	bool DESFireEV1ISO7816Commands::changeFileSettings(int fileno, EncryptionMode comSettings, DESFireAccessRights accessRights, bool plain)
	{
		bool ret = false;
		std::vector<unsigned char> command;
		short ar = AccessRightsInMemory(accessRights);
		
		command.push_back(static_cast<unsigned char>(comSettings));
		BufferHelper::setUShort(command, ar);
		
		if (!plain)
		{
			std::vector<unsigned char> param;
			param.push_back(DF_INS_CHANGE_FILE_SETTINGS);
			param.push_back(static_cast<unsigned char>(fileno));
			command = d_crypto->desfireEncrypt(command, param);
		}
		unsigned char uf = static_cast<unsigned char>(fileno);
		command.insert(command.begin(), &uf, &uf + 1);

		if (d_crypto->d_auth_method == CM_LEGACY && plain)
		{
			DESFireISO7816Commands::transmit(DF_INS_CHANGE_FILE_SETTINGS, command);
			ret = true;
		}
		else
		{
			std::vector<unsigned char> dd = transmit_plain(DF_INS_CHANGE_FILE_SETTINGS, command);
			dd.resize(dd.size() - 2);
			ret = d_crypto->verifyMAC(true, dd);
		}

		return ret;
	}

	bool DESFireEV1ISO7816Commands::changeKeySettings(DESFireKeySettings settings)
	{
		bool ret = false;
		unsigned char command[1];
		command[0] = static_cast<unsigned char>(settings);

		unsigned char param[1];
		param[0] = DF_INS_CHANGE_KEY_SETTINGS;

		std::vector<unsigned char> cryptogram = d_crypto->desfireEncrypt(std::vector<unsigned char>(command, command + sizeof(command)), std::vector<unsigned char>(param, param + sizeof(param)));

		if (d_crypto->d_auth_method == CM_LEGACY)
		{
			DESFireISO7816Commands::transmit(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
			ret = true;
		}
		else
		{
			std::vector<unsigned char> r = transmit_plain(DF_INS_CHANGE_KEY_SETTINGS, cryptogram);
			r.resize(r.size() - 2);
			ret = d_crypto->verifyMAC(true, r);
		}
		
		return ret;
	}

	bool DESFireEV1ISO7816Commands::changeKey(unsigned char keyno, boost::shared_ptr<DESFireKey> key)
	{
		unsigned char diversify[24];
		d_crypto->getDiversify(diversify);
		std::vector<unsigned char> cryptogram;

		if (boost::dynamic_pointer_cast<SAMKeyStorage>(key->getKeyStorage()))
		{
			cryptogram = getChangeKeySAMCryptogram(keyno, key);
		}
		else
		{
			cryptogram = d_crypto->changeKey_PICC(keyno, key, diversify);
		}

		std::vector<unsigned char> data;
		unsigned char keynobyte = keyno;
		if (keyno == 0 && d_crypto->d_currentAid == 0)
		{
			keynobyte |= key->getKeyType();
		}
		data.push_back(keynobyte);
		data.insert(data.end(), cryptogram.begin(), cryptogram.end());
		if (d_crypto->d_auth_method == CM_LEGACY)
		{			
			DESFireISO7816Commands::transmit(DF_INS_CHANGE_KEY, data);
		}
		else
		{
			std::vector<unsigned char> dd = transmit_plain(DF_INS_CHANGE_KEY, data);
			unsigned char err = dd.back();
			dd.resize(dd.size() - 2);

			// Don't check CMAC if master key.
			if (dd.size() > 0 && keyno != 0)
			{
				std::vector<unsigned char> CMAC = d_crypto->desfire_cmac(std::vector<unsigned char>(&err, &err + 1));
				EXCEPTION_ASSERT_WITH_LOG(dd == CMAC, LibLogicalAccessException, "Wrong CMAC.")
			}
		}

		return true;
	}

	bool DESFireEV1ISO7816Commands::getVersion(DESFireCommands::DESFireCardVersion& dataVersion)
	{
		bool r = false;
		std::vector<unsigned char> result;
		std::vector<unsigned char> allresult;

		result = transmit_nomacv(DF_INS_GET_VERSION);

		if ((result.size() - 2) >= 7)
		{
			if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
			{
				memcpy(&dataVersion, &result[0], 7);
				allresult.insert(allresult.end(), result.begin(), result.end() - 2);

				result = transmit_plain(DF_INS_ADDITIONAL_FRAME);

				if (result.size()-2 >= 7)
				{
					if (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
					{
						memcpy(reinterpret_cast<char*>(&dataVersion) + 7, &result[0], 7);						
						allresult.insert(allresult.end(), result.begin(), result.end() - 2);

						result = transmit_plain(DF_INS_ADDITIONAL_FRAME);
						if (result.size()-2 >= 14)
						{
							if (result[result.size() - 1] == 0x00)
							{
								memcpy(reinterpret_cast<char*>(&dataVersion) + 14, &result[0], 14);
								allresult.insert(allresult.end(), result.begin(), result.end() - 2);

								if (d_crypto->d_auth_method != CM_LEGACY)
								{
									r = d_crypto->verifyMAC(true, allresult);
								}
								else
								{
									r = true;
								}
							}
						}
					}
				}
			}
		}

		return r;
	}

	std::vector<int> DESFireEV1ISO7816Commands::getApplicationIDs()
	{
		std::vector<int> aids;
		std::vector<unsigned char> result;

		result = transmit(DF_INS_GET_APPLICATION_IDS);

		while (result[result.size() - 1] == DF_INS_ADDITIONAL_FRAME)
		{
			for (size_t i = 0; i < result.size() - 2; i += 3)
			{
				aids.push_back(DESFireLocation::convertAidToInt(&result[i]));
			}

			result = transmit(DF_INS_ADDITIONAL_FRAME);
		}

		result.resize(result.size() - 2);
		if (d_crypto->d_auth_method != CM_LEGACY)
		{
			result.resize(result.size() - 8); // Remove CMAC
		}

		for (size_t i = 0; i < result.size(); i += 3)
		{
			aids.push_back(DESFireLocation::convertAidToInt(&result[i]));
		}

		return aids;
	}

	std::vector<int> DESFireEV1ISO7816Commands::getFileIDs()
	{
		std::vector<unsigned char> result = transmit(DF_INS_GET_FILE_IDS);
		result.resize(result.size() - 2);
		if (d_crypto->d_auth_method != CM_LEGACY)
		{
			result.resize(result.size() - 8); // Remove CMAC
		}

		std::vector<int> files;
		for (size_t i = 0; i < result.size(); ++i)
		{
			files.push_back(result[i]);
		}

		return files;
	}

	bool DESFireEV1ISO7816Commands::getValue(int fileno, EncryptionMode mode, int& value)
	{
		bool ret = false;

		unsigned char command[1];
		command[0] = static_cast<unsigned char>(fileno);

		std::vector<unsigned char> result = transmit_nomacv(DF_INS_GET_VALUE, command, sizeof(command));
		unsigned char err = result.back();
		result.resize(result.size() - 2);
		result = handleReadData(err, result, 4, mode);

		if (result.size() >= 4)
		{
			size_t offset = 0;
			value = BufferHelper::getInt32(result, offset);
			ret = true;
		}
		
		return ret;
	}

	void DESFireEV1ISO7816Commands::iso_selectApplication(std::vector<unsigned char> isoaid)
	{
		getISO7816ReaderCardAdapter()->sendAPDUCommand(DFEV1_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, SELECT_FILE_BY_AID, 0x00, static_cast<unsigned char>(isoaid.size()), &isoaid[0], static_cast<unsigned char>(isoaid.size()));
	}

	void DESFireEV1ISO7816Commands::setConfiguration(bool formatCardEnabled, bool randomIdEnabled)
	{
		unsigned char command[1];
		//command[0] = 0x00;
		command[0] = 0x00 | ((formatCardEnabled) ? 0x00 : 0x01) | ((randomIdEnabled) ? 0x02 : 0x00);

		d_crypto->initBuf(sizeof(command));
		std::vector<unsigned char> encBuffer = d_crypto->desfire_encrypt(d_crypto->d_sessionKey, std::vector<unsigned char>(command, command + sizeof(command)));
		std::vector<unsigned char> buf;
		buf.push_back(0x00);
		buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
		DESFireISO7816Commands::transmit(DFEV1_INS_SET_CONFIGURATION, buf);
	}

	void DESFireEV1ISO7816Commands::setConfiguration(boost::shared_ptr<DESFireKey> defaultKey)
	{
		if (defaultKey->getLength() < 24)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The default key length must be 24-byte long.");
		}

		unsigned char command[25];
		memcpy(&command[0], defaultKey->getData(), 24);
		command[24] = defaultKey->getKeyVersion();

		std::vector<unsigned char> encBuffer = d_crypto->desfireEncrypt(std::vector<unsigned char>(command, command + sizeof(command)));
		std::vector<unsigned char> buf;
		buf.push_back(0x02);
		buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
		DESFireISO7816Commands::transmit(DFEV1_INS_SET_CONFIGURATION, buf);
	}

	void DESFireEV1ISO7816Commands::setConfiguration(const std::vector<unsigned char>& ats)
	{
		std::vector<unsigned char> encBuffer = d_crypto->desfireEncrypt(ats);
		std::vector<unsigned char> buf;
		buf.push_back(0x02);
		buf.insert(buf.end(), encBuffer.begin(), encBuffer.end());
		DESFireISO7816Commands::transmit(DFEV1_INS_SET_CONFIGURATION, buf);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit(unsigned char cmd, const void* buf, size_t buflen, bool forceLc)
	{		
		std::vector<unsigned char> CMAC;

		if (d_crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME)
		{
			std::vector<unsigned char> apdu_command;
			apdu_command.push_back(cmd);
			if (buf != NULL && buflen > 0)
			{
				apdu_command.insert(apdu_command.end(), (unsigned char*)buf, (unsigned char*)buf + buflen);
			}
			CMAC = d_crypto->desfire_cmac(apdu_command);
		}

		std::vector<unsigned char> r = DESFireISO7816Commands::transmit(cmd, buf, buflen, forceLc);
		unsigned char err = r.back();
		if (d_crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME && r.size() > 8 && err != DF_INS_ADDITIONAL_FRAME)
		{
			std::vector<unsigned char> response = std::vector<unsigned char>(r.begin(), r.end() - 10);
			response.push_back(err);

			CMAC = d_crypto->desfire_cmac(response);

			EXCEPTION_ASSERT_WITH_LOG(std::vector<unsigned char>(r.end() - 10, r.end() - 2) == CMAC, LibLogicalAccessException, "Wrong CMAC.");
		}
		
		return r;
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd, unsigned char lc)
	{
		return transmit_plain(cmd, NULL, lc, true);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd, const std::vector<unsigned char>& buf, bool forceLc)
	{
		return transmit_plain(cmd, &buf[0], buf.size(), forceLc);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_plain(unsigned char cmd, const void* buf, size_t buflen, bool forceLc)
	{
		return DESFireISO7816Commands::transmit(cmd, buf, buflen, forceLc);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_nomacv(unsigned char cmd, unsigned char lc)
	{
		return transmit_nomacv(cmd, NULL, lc, true);
	}

	std::vector<unsigned char> DESFireEV1ISO7816Commands::transmit_nomacv(unsigned char cmd, const void* buf, size_t buflen, bool forceLc)
	{
		std::vector<unsigned char> CMAC;

		if (d_crypto->d_auth_method != CM_LEGACY && cmd != DF_INS_ADDITIONAL_FRAME)
		{
			std::vector<unsigned char> apdu_command;
			apdu_command.push_back(cmd);
			if (buf != NULL && buflen > 0)
			{
				apdu_command.insert(apdu_command.end(), (unsigned char*)buf, (unsigned char*)buf + buflen);
			}
			CMAC = d_crypto->desfire_cmac(apdu_command);
		}

		return DESFireISO7816Commands::transmit(cmd, buf, buflen, forceLc);
	}
}
