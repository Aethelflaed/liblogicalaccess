/**
 * \file desfirecrypto.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire cryptographic functions.
 */

#include "desfirecommands.hpp"
#include "desfirecrypto.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <zlib.h>
#include <ctime>
#include <cstdlib>

#include <cstring>

#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"


namespace logicalaccess
{		
	DESFireCrypto::DESFireCrypto()
	{
		d_auth_method = CM_LEGACY;
		d_currentAid = 0;
		d_currentKeyNo = 0;
		d_mac_size = 4;
		d_block_size = 8;

		d_lastIV.clear();
		d_lastIV.resize(d_block_size, 0x00);
	}

	DESFireCrypto::~DESFireCrypto()
	{
		
	}

	void DESFireCrypto::decipherData1(size_t length, const std::vector<unsigned char>& data)
	{
		initBuf(length);
		d_buf.insert(d_buf.end(), data.begin(), data.end());
	}

	void DESFireCrypto::decipherData2(const std::vector<unsigned char>& data)
	{
		d_buf.insert(d_buf.end(), data.begin(), data.end());
	}

	std::vector<unsigned char> DESFireCrypto::decipherData(size_t length)
	{
		std::vector<unsigned char> ret;

		if (d_auth_method == CM_LEGACY)
		{
			ret = desfire_decrypt(d_sessionKey, d_buf, length);
		}
		else
		{
			ret = desfire_iso_decrypt(d_sessionKey, d_buf, d_cipher, d_block_size, length);
		}

		return ret;
	}

	void DESFireCrypto::initBuf(size_t /*length*/)
	{
		d_buf.clear();
		d_last_left.clear();
		if (d_auth_method == CM_LEGACY)
		{
			d_lastIV.clear();
			d_lastIV.resize(d_block_size, 0x00);
		}
	}

	bool DESFireCrypto::verifyMAC(bool end, const std::vector<unsigned char>& data)
	{
		bool ret = false;
		d_buf.insert(d_buf.end(), data.begin(), data.end());
		
		if (end)
		{
			if (d_auth_method == CM_LEGACY)	// Native DESFire mode
			{				
				std::vector<unsigned char> mac;
				mac.insert(mac.end(), d_buf.end() - 4, d_buf.end());
				std::vector<unsigned char> ourMacBuf;
				ourMacBuf.insert(ourMacBuf.end(), d_buf.begin(), d_buf.end() - 4);
				std::vector<unsigned char> ourMac = desfire_mac(d_sessionKey, ourMacBuf);
				ret = (mac == ourMac);
			}
			else
			{
				std::vector<unsigned char> mac;
				mac.insert(mac.end(), d_buf.end() - 8, d_buf.end());
				std::vector<unsigned char> ourMacBuf;
				ourMacBuf.insert(ourMacBuf.end(), d_buf.begin(), d_buf.end() - 8);
				ourMacBuf.push_back(0x00); // SW_OPERATION_OK
				std::vector<unsigned char> ourMac = desfire_cmac(d_sessionKey, d_cipher, d_block_size, ourMacBuf);
				ret = (mac == ourMac);
			}
			
			d_buf.clear();
		}	
		else
		{
			ret = true;
		}

		return ret;
	}

	void DESFireCrypto::bufferingForGenerateMAC(const std::vector<unsigned char>& data)
	{
		d_buf.insert(d_buf.end(), data.begin(), data.end());
	}

	std::vector<unsigned char> DESFireCrypto::generateMAC(const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> ret;
		bufferingForGenerateMAC(data);
		if (d_auth_method == CM_LEGACY)
		{
			ret = desfire_mac(d_sessionKey, d_buf);
		}
		else
		{
			ret = desfire_cmac(d_sessionKey, d_cipher, d_block_size, d_buf);
		}
		d_buf.clear();

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::desfireEncrypt(const std::vector<unsigned char>& data, const std::vector<unsigned char>& param)
	{
		std::vector<unsigned char> ret;

		if (d_auth_method == CM_LEGACY)
		{
			ret = desfire_encrypt(d_sessionKey, data);
		}
		else
		{
			ret = desfire_iso_encrypt(d_sessionKey, data, d_cipher, d_block_size, param);
		}

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::encipherData(bool end, std::vector<unsigned char> data)
	{
		std::vector<unsigned char> ret;
		d_buf.insert(d_buf.end(), data.begin(), data.end());

		data.insert(data.begin(), d_last_left.begin(), d_last_left.end());

		int leave = data.size() % 8;
		if (leave > 0)
		{
			d_last_left.clear();
			d_last_left.insert(d_last_left.end(), data.end() - leave,  data.end());
			data.resize(data.size() - leave);
		}
		else
		{
			d_last_left.clear();
		}

		if (data.size() > 0)
		{
			ret = desfire_CBC_send(d_sessionKey, d_lastIV, data);
			d_lastIV.clear();
			d_lastIV.insert(d_lastIV.end(), ret.end() - 8, ret.end());
		}
		

		if (end)
		{
			std::vector<unsigned char> fdata;
			fdata.insert(fdata.end(), d_last_left.begin(), d_last_left.end());
			d_last_left.clear();
			short crc = desfire_crc16(&d_buf[0], d_buf.size());
			fdata.push_back(static_cast<unsigned char>(crc & 0xff));
			fdata.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));

			int pad = (8 - (fdata.size() % 8)) % 8;
			for (int i = 0; i < pad; ++i)
			{
				fdata.push_back(0x00);
			}

			std::vector<unsigned char> ret2 = desfire_CBC_send(d_sessionKey, d_lastIV, fdata);
			ret.insert(ret.end(), ret2.begin(), ret2.end());
		}

		return ret;
	}

	short DESFireCrypto::desfire_crc16(const void* data, size_t dataLength)
	{
		unsigned char first, second;

		ComputeCrc(CRC_A, reinterpret_cast<const unsigned char*>(data), dataLength, &first, &second);

		return ((second << 8) | first);
	}

	long DESFireCrypto::desfire_crc32(const void* data, size_t dataLength)
	{
		long crc = crc32(0L, Z_NULL, 0);
		crc = crc32(crc, reinterpret_cast<const Bytef*>(data), static_cast<uInt>(dataLength));
		crc ^= 0xffffffff;
		if (crc < 0)
		{
			crc = (-1 * (crc^0xffffffff)) - 1;
		}

		return crc;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		symmetric_key skey;
		unsigned char in[8], out[8], in2[8];
		
		unsigned int i=0;
		unsigned int j=0;

		std::vector<unsigned char> ret;

		bool is3des = false;
		if (memcmp(&key[0], &key[8], 8))
		{
			is3des = true;
		}

		// Set encryption keys
		if (is3des)
		{
			des3_setup(&key[0], 16, 0, &skey);
		}
		else
		{
			des_setup(&key[0], 8, 0, &skey);
		}
	

		// clear buffers
		memset(in, 0x00, 8);
		memset(out, 0x00, 8);
		memset(in2, 0x00, 8);

		// do for each 8 byte block of input
		for (i=0; i<data.size()/8; i++)
		{
			// copy 8 bytes from input buffer to in
			memcpy(in, &data[i*8], 8);

			if (i == 0)
			{
				if (iv.size() >= 8)
				{
					for (j=0;j<8;j++)
					{
						in[j] ^=  iv[j];
					}
				}
			}
			else
			{
				for (j=0;j<8;j++)
				{
					in[j] ^= in2[j];
				}
			}
			
			// encryption
			if (is3des)
			{
				des3_ecb_decrypt(in, out, &skey);			
			}
			else
			{
				des_ecb_decrypt(in, out, &skey);
			}

			memcpy(in2, out, 8);

			// copy decrypted block to output
			ret.insert(ret.end(), out, out + 8);
		}

		if (is3des)
		{
			des3_done(&skey);		
		}
		else
		{
			des_done(&skey);
		}

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_CBC_receive(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		symmetric_key skey;
		unsigned char in[8],out[8], in2[8];
		
		unsigned int i=0;
		unsigned int j=0;

		std::vector<unsigned char> ret;

		// Set encryption keys
		des3_setup(&key[0], 16, 0, &skey);

		// clear buffers
		memset(in, 0x00, 8);
		memset(out, 0x00, 8);
		memset(in2, 0x00, 8);

		// do for each 8 byte block of input
		for (i=0;i<data.size()/8;i++)
		{
			// copy 8 bytes from input buffer to in
			memcpy(in, &data[i*8], 8);
			
			// 3DES encryption
			des3_ecb_decrypt(in, out, &skey);

			if (i == 0)
			{
				if (iv.size() >= 8)
				{
					for (j=0;j<8;j++)
					{
						out[j] ^= iv[j];
					}
				}
			}
			else
			{
				for (j=0;j<8;j++)
				{
					out[j] ^= in2[j];
				}
			}

			memcpy(in2, &data[i*8], 8);

			// copy decrypted block to output
			ret.insert(ret.end(), out, out + 8);
		}

		des3_done(&skey);

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_CBC_mac(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> ret = sam_CBC_send(key, iv, data);
		return std::vector<unsigned char>(ret.end() - 8, ret.end() - 4);
	}

	std::vector<unsigned char> DESFireCrypto::sam_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		unsigned char in[8], out[8];
		symmetric_key skey;
		unsigned int i=0;
		unsigned int j=0;

		std::vector<unsigned char> ret;

		des3_setup(&key[0], 16, 0, &skey);

		// clear buffers
		memset(in, 0x00, 8);
		memset(out, 0x00, 8);

		// do for each 8 byte block of input
		for (i=0; i < data.size()/8; i++)
		{
			// copy 8 bytes from input buffer to in
			memcpy(in, &data[i*8], 8);

			if (i == 0)
			{
				if (iv.size() >= 8)
				{
					for (j=0;j<8;j++)
					{
						in[j] ^= iv[j];
					}
				}
			}
			else
			{
				for (j=0;j<8;j++)
				{
					in[j] = in[j]^out[j];					
				}
			}
			
			// 3DES encryption
			des3_ecb_encrypt(in, out, &skey);

			// copy encrypted block to output
			ret.insert(ret.end(), out, out + 8);
		}

		des3_done(&skey);

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_mac(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
	{
		int pad = (8 - (data.size() % 8)) % 8;
		for (int i = 0; i < pad; ++i)
		{
			data.push_back(0x00);
		}

		return desfire_CBC_mac(key, std::vector<unsigned char>(), data);
	}

	std::vector<unsigned char> DESFireCrypto::desfire_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
	{
		int pad = (8 - ((data.size() + 2) % 8)) % 8;

		short crc = desfire_crc16(&data[0], data.size());
		data.push_back(static_cast<unsigned char>(crc & 0xff));
		data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
		for (int i = 0; i < pad; ++i)
		{
			data.push_back(0x00);
		}

		return desfire_CBC_send(key, std::vector<unsigned char>(), data);
	}

	std::vector<unsigned char> DESFireCrypto::sam_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
	{
		int pad = (8 - ((data.size() + 2) % 8)) % 8;		

		short crc = desfire_crc16(&data[0], data.size());
		data.push_back(static_cast<unsigned char>(crc & 0xff));
		data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
		for (int i = 0; i < pad; ++i)
		{
			data.push_back(0x00);
		}			

		return sam_CBC_send(key, std::vector<unsigned char>(), data);
	}

	std::vector<unsigned char> DESFireCrypto::desfire_decrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, size_t datalen)
	{
		std::vector<unsigned char> ret;
		size_t ll = 0;

		ret = desfire_CBC_receive(key, std::vector<unsigned char>(), data);

		if (datalen == 0)
		{
			ll = ret.size() - 1;

			while (ret[ll] == 0x00)
			{
				ll--;
			}

			EXCEPTION_ASSERT_WITH_LOG(ret[ll] == 0x80, LibLogicalAccessException, "Incorrect FLT result");

			ll -= 2;
		}
		else
		{
			ll = datalen;
		}

		unsigned short crc1 = desfire_crc16(&ret[0], ll);
		unsigned short crc2 = static_cast<unsigned short>(ret[ll] | (ret[ll+1] << 8));

		char computationError[128];
		memset(computationError, 0x00, 128);
		sprintf(computationError, "Error in crc computation: (crc1: %04x, crc2: %04x)", crc1, crc2);
		EXCEPTION_ASSERT_WITH_LOG(crc1 == crc2, LibLogicalAccessException, computationError);

		ret.resize(ll);
		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::authenticate_PICC1(unsigned char keyno, unsigned char *diversify, const std::vector<unsigned char>& encRndB)
	{
		d_sessionKey.clear();
		d_authkey.resize(16);
		d_profile->getKey(d_currentAid, keyno, diversify, d_authkey);
		d_rndB = desfire_CBC_send(d_authkey, std::vector<unsigned char>(), encRndB);
		
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 8);
		rndB1.push_back(d_rndB[0]);

		RAND_seed(&keyno, sizeof(keyno));

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		d_rndA.clear();
		d_rndA.resize(8);
		if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> rndAB;
		rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		return desfire_CBC_send(d_authkey, std::vector<unsigned char>(), rndAB);
	}

	void DESFireCrypto::authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1)
	{
		std::vector<unsigned char> rndA = desfire_CBC_send(d_authkey, std::vector<unsigned char>(), encRndA1);
		std::vector<unsigned char> checkRndA;

		d_sessionKey.clear();
		checkRndA.push_back(rndA[7]);
		checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 7);

		if (d_rndA == checkRndA)
		{
			// DES
			if (!memcmp(&d_authkey[0], &d_authkey[8], 8))
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
			}
			// 3DES
			else
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4, d_rndA.begin() + 8);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4, d_rndB.begin() + 8);
			}

			d_currentKeyNo = keyno;
		}

		d_auth_method = CM_LEGACY;
		d_mac_size = 4;
	}

	void DESFireCrypto::getKey(boost::shared_ptr<DESFireKey> key, unsigned char* diversify, std::vector<unsigned char>& keydiv)
	{
		keydiv.clear();
		if (key->getDiversify() && diversify != NULL)
		{
			if (key->getKeyType() != DF_KEY_AES)
			{
				if (key->getLength() == 16)
				{
					// Sagem diversification algo. Should be an option with SAM diversifiction soon...
					std::vector<unsigned char> iv;
					// Two time, to have ECB and not CBC mode (laazzyyy to create new function :))
					unsigned char* keydata = key->getData();
					std::vector<unsigned char> r = sam_CBC_send(std::vector<unsigned char>(keydata, keydata + key->getLength()), iv, std::vector<unsigned char>(diversify, diversify + 8));
					keydiv.insert(keydiv.end(), r.begin(), r.end());
					keydata = key->getData();
					std::vector<unsigned char> r2 = sam_CBC_send(std::vector<unsigned char>(keydata, keydata + key->getLength()), iv, std::vector<unsigned char>(diversify + 8, diversify + 16));
					keydiv.insert(keydiv.end(), r2.begin(), r2.end());
				}
				else
				{
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Diversification for this key type is not implemented yet.");
				}
			}
			else
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Diversification for this key type is not implemented yet.");
			}
		}
		else
		{
			unsigned char* keydata = key->getData();
			keydiv.insert(keydiv.end(), keydata, keydata + key->getLength());
		}

		if (key->getKeyType() != DF_KEY_AES)
		{
			unsigned char* keyversioned = new unsigned char[key->getLength()];
			memset(keyversioned, 0x00, key->getLength());
			unsigned char version = key->getKeyVersion();
			// set the key version
			for (unsigned char i = 0; i < 8; i++)
			{
				keyversioned[7 - i] = (unsigned char)((unsigned char)(keydiv[7 - i] & 0xFE) | (unsigned char)((version >> i) & 0x01));
				keyversioned[i + 8] = ((unsigned char)(keydiv[i + 8] & 0xFE));
				if (key->getLength() == 24)
				{
					keyversioned[i + 16] = ((unsigned char)(keydiv[i + 16] & 0xFE));
				}
			}
			keydiv.clear();
			keydiv.insert(keydiv.end(), keyversioned, keyversioned + key->getLength());
			delete[] keyversioned;
		}
	}

	void DESFireCrypto::selectApplication(size_t aid)
	{
		d_currentAid = static_cast<int>(aid);

		d_auth_method = CM_LEGACY;
		d_cipher.reset();
		d_currentKeyNo = 0;
		d_sessionKey.clear();
	}

	std::vector<unsigned char> DESFireCrypto::changeKey_PICC(unsigned char keyno, boost::shared_ptr<DESFireKey> newkey, unsigned char* diversify)
	{
		std::vector<unsigned char> cryptogram;
		std::vector<unsigned char> oldkeydiv, newkeydiv;
		oldkeydiv.resize(16, 0x00);
		newkeydiv.resize(16, 0x00);
		d_profile->getKey(d_currentAid, keyno, diversify, oldkeydiv);
		getKey(newkey, diversify, newkeydiv);		

		std::vector<unsigned char> encCryptogram;

		if (d_auth_method == CM_LEGACY) // Native DESFire
		{
			if (keyno != d_currentKeyNo)
			{
				short crc;
				for (unsigned int i = 0; i < newkeydiv.size(); ++i)
				{
					encCryptogram.push_back(static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
				}
				crc = desfire_crc16(&encCryptogram[0], newkeydiv.size());
				encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
				crc = desfire_crc16(&newkeydiv[0], newkeydiv.size());
				encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
				// Pad
				for (int i = 0; i < 4; ++i)
				{
					encCryptogram.push_back(0x00);
				}
				cryptogram = desfire_CBC_send(d_sessionKey, std::vector<unsigned char>(), encCryptogram);
			}
			else
			{
				cryptogram = desfire_encrypt(d_sessionKey, newkeydiv);
			}
		}
		else
		{
			if (keyno != d_currentKeyNo)
			{
				long crc;
				for (unsigned int i = 0; i < newkeydiv.size(); ++i)
				{
					encCryptogram.push_back(static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
				}

				boost::shared_ptr<openssl::SymmetricKey> sessionkey;
				boost::shared_ptr<openssl::InitializationVector> iv;

				if (boost::dynamic_pointer_cast<openssl::AESCipher>(d_cipher))
				{
					// For AES, add key version.
					encCryptogram.push_back(newkey->getKeyVersion());

					sessionkey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_sessionKey)));
					iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
				}
				else
				{
					sessionkey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(d_sessionKey)));
					iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
				}
				std::vector<unsigned char> crcoldkxor;
				crcoldkxor.push_back(DF_INS_CHANGE_KEY);
				crcoldkxor.push_back(keyno);
				crcoldkxor.insert(crcoldkxor.end(), encCryptogram.begin(), encCryptogram.end());
				crc = desfire_crc32(&crcoldkxor[0], crcoldkxor.size());
				encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
				crc = desfire_crc32(&newkeydiv[0], newkeydiv.size());
				encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
				encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));

				int pad = (d_block_size - (encCryptogram.size() % d_block_size)) % d_block_size;
				for (int i = 0; i < pad; ++i)
				{
					encCryptogram.push_back(0x00);
				}
				
				d_cipher->cipher(encCryptogram, cryptogram, *sessionkey, *iv, false);
				d_lastIV = std::vector<unsigned char>(cryptogram.end() - d_block_size, cryptogram.end());
			}
			else
			{
				if (boost::dynamic_pointer_cast<openssl::AESCipher>(d_cipher))
				{
					// For AES, add key version.
					newkeydiv.push_back(newkey->getKeyVersion());
				}

				encCryptogram.push_back(DF_INS_CHANGE_KEY);
				encCryptogram.push_back(keyno);
				cryptogram = desfire_iso_encrypt(d_sessionKey, newkeydiv, d_cipher, d_block_size, encCryptogram);
			}
		}

		return cryptogram;
	}	

	std::vector<unsigned char> DESFireCrypto::iso_authenticate_PICC1(unsigned char keyno, unsigned char *diversify, const std::vector<unsigned char>& encRndB, unsigned int randomlen)
	{
		d_sessionKey.clear();		
		d_profile->getKey(d_currentAid, keyno, diversify, d_authkey);
		d_cipher.reset(new openssl::DESCipher());
		openssl::DESSymmetricKey deskey = openssl::DESSymmetricKey::createFromData(d_authkey);
		openssl::DESInitializationVector iv = openssl::DESInitializationVector::createNull();
		d_rndB.clear();		
		d_cipher->decipher(encRndB, d_rndB, deskey, iv, false);
		d_lastIV = std::vector<unsigned char>(encRndB.end() - randomlen, encRndB.end());
		
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + randomlen);
		rndB1.push_back(d_rndB[0]);

		RAND_seed(&keyno, sizeof(keyno));

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		d_rndA.clear();
		d_rndA.resize(randomlen);
		if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> rndAB;
		rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		std::vector<unsigned char> ret;
		d_cipher->cipher(rndAB, ret, deskey, iv, false);
		d_lastIV = std::vector<unsigned char>(ret.end() - randomlen, ret.end());

		return ret;
	}

	void DESFireCrypto::iso_authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1, unsigned int randomlen)
	{
		std::vector<unsigned char> checkRndA;
		std::vector<unsigned char> rndA;
		openssl::DESSymmetricKey deskey = openssl::DESSymmetricKey::createFromData(d_authkey);
		openssl::DESInitializationVector iv = openssl::DESInitializationVector::createFromData(d_lastIV);
		d_cipher->decipher(encRndA1, rndA, deskey, iv, false);

		d_sessionKey.clear();
		checkRndA.push_back(rndA[randomlen-1]);
		checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + randomlen - 1);

		if (d_rndA == checkRndA)
		{
			// 2K3DES
			if (d_authkey.size() == 16)
			{
				// DES
				if (!memcmp(&d_authkey[0], &d_authkey[8], 8))
				{
					d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
					d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
					d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
					d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				}
				// 3DES
				else
				{
					d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
					d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
					d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4, d_rndA.begin() + 8);
					d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4, d_rndB.begin() + 8);
				}
			}
			// 3K3DES
			else if (d_authkey.size() == 24)
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 6, d_rndA.begin() + 10);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 6, d_rndB.begin() + 10);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndB.begin() + 16);
			}

			d_currentKeyNo = keyno;
		}

		d_cipher.reset(new openssl::DESCipher());		
		d_auth_method = CM_ISO;
		d_block_size = 8;
		d_mac_size = 8;
		d_lastIV.clear();
		d_lastIV.resize(d_block_size, 0x00);
	}

	std::vector<unsigned char> DESFireCrypto::aes_authenticate_PICC1(unsigned char keyno, unsigned char *diversify, const std::vector<unsigned char>& encRndB)
	{
		d_sessionKey.clear();		
		d_profile->getKey(d_currentAid, keyno, diversify, d_authkey);
		d_cipher.reset(new openssl::AESCipher());
		openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(d_authkey);
		openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
		d_rndB.clear();		
		d_cipher->decipher(encRndB, d_rndB, aeskey, iv, false);
		d_lastIV = std::vector<unsigned char>(encRndB.end() - 16, encRndB.end());
		
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 1 + 15);
		rndB1.push_back(d_rndB[0]);

		RAND_seed(&keyno, sizeof(keyno));

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		d_rndA.clear();
		d_rndA.resize(16);
		if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> rndAB;
		rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		std::vector<unsigned char> ret;
		d_cipher->cipher(rndAB, ret, aeskey, iv, false);
		d_lastIV = std::vector<unsigned char>(ret.end() - 16, ret.end());

		return ret;
	}

	void DESFireCrypto::aes_authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1)
	{
		std::vector<unsigned char> checkRndA;
		std::vector<unsigned char> rndA;
		openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(d_authkey);
		openssl::AESInitializationVector iv = openssl::AESInitializationVector::createFromData(d_lastIV);
		d_cipher->decipher(encRndA1, rndA, aeskey, iv, false);

		d_sessionKey.clear();
		checkRndA.push_back(rndA[15]);
		checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 15);

		if (d_rndA == checkRndA)
		{
			d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
			d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndA.begin() + 4);
			d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
			d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndA.begin() + 16);

			d_currentKeyNo = keyno;
		}

		d_cipher.reset(new openssl::AESCipher());
		d_auth_method = CM_ISO;
		d_block_size = 16;
		d_mac_size = 8;
		d_lastIV.clear();
		d_lastIV.resize(d_block_size, 0x00);
	}

	std::vector<unsigned char> DESFireCrypto::desfire_cmac(const std::vector<unsigned char>& data)
	{
		return desfire_cmac(d_sessionKey, d_cipher, d_block_size, data);
	}

	std::vector<unsigned char> DESFireCrypto::desfire_cmac_generic(const std::vector<unsigned char>& key, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, unsigned int block_size, const std::vector<unsigned char>& data, std::vector<unsigned char> lastIV)
	{
		boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherK1K2;

		boost::shared_ptr<openssl::SymmetricKey> symkey;
		boost::shared_ptr<openssl::InitializationVector> iv;
		// 3DES
		if (boost::dynamic_pointer_cast<openssl::DESCipher>(cipherMAC))
		{
			cipherK1K2.reset(new openssl::DESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB));

			symkey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(key)));
			iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createNull()));
		}
		// AES
		else
		{
			cipherK1K2.reset(new openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB));

			symkey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(key)));
			iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createNull()));
		}

		unsigned char Rb;

		// TDES
		if (block_size == 8)
		{
			Rb = 0x1b;
		}
		// AES
		else
		{
			Rb = 0x87;
		}

		std::vector<unsigned char> blankbuf;
		blankbuf.resize(block_size, 0x00);
		std::vector<unsigned char> L;
		cipherK1K2->cipher(blankbuf, L, *symkey.get(), *iv.get(), false);

		std::vector<unsigned char> K1;
		if ((L[0] & 0x80) == 0x00)
		{
			K1 = shift_string(L);
		}
		else
		{
			K1 = shift_string(L, Rb);
		}

		std::vector<unsigned char> K2;
		if ((K1[0] & 0x80) == 0x00)
		{
			K2 = shift_string(K1);
		}
		else
		{
			K2 = shift_string(K1, Rb);
		}

		int pad = (block_size - (data.size() % block_size)) % block_size;
		std::vector<unsigned char> padded_data = data;
		if (pad > 0)
		{
			padded_data.push_back(0x80);
			if (pad > 1)
			{
				for (int i = 0; i < (pad - 1); ++i)
				{
					padded_data.push_back(0x00);
				}
			}
		}

		// XOR with K1
		if (pad == 0)
		{
			for (unsigned int i = 0; i < K1.size(); ++i)
			{
				padded_data[padded_data.size() - K1.size() + i] = static_cast<unsigned char>(padded_data[padded_data.size() - K1.size() + i] ^ K1[i]);
			}
		}
		// XOR with K2
		else
		{
			for (unsigned int i = 0; i < K2.size(); ++i)
			{
				padded_data[padded_data.size() - K2.size() + i] = static_cast<unsigned char>(padded_data[padded_data.size() - K2.size() + i] ^ K2[i]);
			}
		}

		std::vector<unsigned char> ret;
		// 3DES
		if (boost::dynamic_pointer_cast<openssl::DESCipher>(cipherMAC))
		{
			iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(lastIV)));
		}
		// AES
		else
		{
			iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(lastIV)));
		}
		cipherMAC->cipher(padded_data, ret, *symkey.get(), *iv.get(), false);

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_cmac(const std::vector<unsigned char>& key, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, unsigned int block_size, const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> ret = desfire_cmac_generic(key, cipherMAC, block_size, data, d_lastIV);

		if (cipherMAC == d_cipher)
		{
			d_lastIV = std::vector<unsigned char>(ret.end() - block_size, ret.end());
		}

		// DES
		if (block_size == 8)
		{
			ret = std::vector<unsigned char>(ret.end() - 8, ret.end());
		}
		else
		{
			ret = std::vector<unsigned char>(ret.end() - 16, ret.end() - 8);
		}

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::shift_string(const std::vector<unsigned char>& buf, unsigned char xorparam)
	{
		std::vector<unsigned char> ret = buf;

		unsigned int i;
		for (i = 0; i < ret.size() - 1; ++i)
		{
			ret[i] = ret[i] << 1;
			// add the carry over bit
			ret[i] = ret[i] | ((ret[i + 1] & 0x80) ? 0x01 : 0x00);
		}

		ret[ret.size()-1] = ret[ret.size()-1] << 1;

		if (xorparam != 0x00)
		{
			ret[ret.size()-1] = static_cast<unsigned char>(ret[ret.size()-1] ^ xorparam);
		}

		return ret;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_iso_decrypt(const std::vector<unsigned char>& data, size_t length)
	{
		return desfire_iso_decrypt(d_sessionKey, data, d_cipher, d_block_size, length);
	}

	std::vector<unsigned char> DESFireCrypto::desfire_iso_decrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher, unsigned int block_size, size_t datalen)
	{
		std::vector<unsigned char> decdata;
		boost::shared_ptr<openssl::SymmetricKey> isokey;
		boost::shared_ptr<openssl::InitializationVector> iv;
		if (boost::dynamic_pointer_cast<openssl::AESCipher>(cipher))
		{
			isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(key)));
			iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
		}
		else
		{
			isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(key)));
			iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
		}

		cipher->decipher(data, decdata, *isokey, *iv, false);
		if (cipher == d_cipher)
		{
			d_lastIV = std::vector<unsigned char>(data.end() - block_size, data.end());
		}

		size_t ll = 0;

		if (datalen == 0)
		{
			ll = decdata.size() - 1;

			while (decdata[ll] == 0x00)
			{
				ll--;
			}

			EXCEPTION_ASSERT_WITH_LOG(decdata[ll] == 0x80, LibLogicalAccessException, "Incorrect FLT result");

			ll -= 4;
		}
		else
		{
			ll = datalen;
		}

		std::vector<unsigned char> crcbuf = std::vector<unsigned char>(decdata.begin(), decdata.begin() + ll);
		crcbuf.push_back(0x00);	// SW_OPERATION_OK
		long crc1 = desfire_crc32(&crcbuf[0], crcbuf.size());
		long crc2 = static_cast<long>(decdata[ll] | (decdata[ll+1] << 8) | (decdata[ll+2] << 16) | (decdata[ll+3] << 24));
		size_t pad = decdata.size() - ll - 4;
		std::vector<unsigned char> padding = std::vector<unsigned char>(decdata.begin() + ll + 4, decdata.begin() + ll + 4 + pad);
		std::vector<unsigned char> padding1;
		for (size_t i = 0; i < pad; ++i)
		{
			padding1.push_back(0x00);
		}
		char computationError[128];
		memset(computationError, 0x00, 128);
		sprintf(computationError, "Error in crc computation: (crc1: %04x, crc2: %04x) or padding", static_cast<unsigned int>(crc1), static_cast<unsigned int>(crc2));
		EXCEPTION_ASSERT_WITH_LOG(crc1 == crc2 && padding == padding1, LibLogicalAccessException, computationError);

		decdata.resize(ll);
		return decdata;
	}

	std::vector<unsigned char> DESFireCrypto::desfire_iso_encrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher, unsigned int block_size, const std::vector<unsigned char>& param)
	{		
		std::vector<unsigned char> encdata;
		std::vector<unsigned char> decdata = data;		
		std::vector<unsigned char> calconbuf = param;
		calconbuf.insert(calconbuf.end(), data.begin(), data.end());
		long crc = desfire_crc32(&calconbuf[0], calconbuf.size());
		decdata.push_back(static_cast<unsigned char>(crc & 0xff));
		decdata.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
		decdata.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
		decdata.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));		
		int pad = (block_size - (decdata.size() % block_size)) % block_size;
		for (int i = 0; i < pad; ++i)
		{
			decdata.push_back(0x00);
		}

		boost::shared_ptr<openssl::SymmetricKey> isokey;
		boost::shared_ptr<openssl::InitializationVector> iv;
		if (boost::dynamic_pointer_cast<openssl::AESCipher>(cipher))
		{
			isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(key)));
			//iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createNull()));
			iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
		}
		else
		{
			isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(key)));
			//iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createNull()));
			iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
		}
		cipher->cipher(decdata, encdata, *isokey, *iv, false);
		if (cipher == d_cipher)
		{
			d_lastIV = std::vector<unsigned char>(encdata.end() - block_size, encdata.end());
		}

		return encdata;
	}

	bool DESFireCrypto::getDiversify(unsigned char* diversify)
	{
		diversify[0] = 0xFF;
		memcpy(diversify + 1, &d_identifier[0], d_identifier.size());
		diversify[8] = 0x00;
		memcpy(diversify + 9, &d_identifier[0], d_identifier.size());
		for (unsigned char i = 0; i < 7; ++i)
		{
			diversify[9 + i] ^= 0xFF;
		}

		return true;
	}

	void DESFireCrypto::setCryptoContext(boost::shared_ptr<DESFireProfile> profile, std::vector<unsigned char> identifier)
	{
		d_profile = profile;
		d_identifier = identifier;
	}

	boost::shared_ptr<DESFireKey> DESFireCrypto::getKey(unsigned char keyno)
	{
		return d_profile->getKey(d_currentAid, keyno);
	}

	void DESFireCrypto::createApplication(int aid, size_t maxNbKeys, DESFireKeyType cryptoMethod)
	{
		for (unsigned char i = 0; i < maxNbKeys; ++i)
		{
			d_profile->setKey(aid, i, d_profile->getDefaultKey(cryptoMethod));
		}
	}
}

