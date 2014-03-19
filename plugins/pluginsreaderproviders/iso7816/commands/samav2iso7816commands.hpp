/**
 * \file SAMAV2ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP

#include "samcommands.hpp"
#include "samav1iso7816commands.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samcrypto.hpp"
#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{
	/**
	 * \brief The DESFire base commands class.
	 */
	class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMISO7816Commands<KeyEntryAV2Information, SETAV2>
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMAV2ISO7816Commands();

			/**
			 * \brief Destructor.
			 */
			virtual ~SAMAV2ISO7816Commands();

			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno);

			virtual boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > getKeyEntry(unsigned char keyno);

			virtual void changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> keyentry, boost::shared_ptr<DESFireKey> key);
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry, boost::shared_ptr<DESFireKey> key);

			virtual std::vector<unsigned char> decipherData(std::vector<unsigned char> data, bool islastdata);
			virtual std::vector<unsigned char> encipherData(std::vector<unsigned char> data, bool islastdata);
			virtual std::vector<unsigned char> changeKeyPICC(const ChangeKeyInfo& info);

		protected:

			void generateSessionKey(std::vector<unsigned char> rnd1, std::vector<unsigned char> rnd2);

			std::vector<unsigned char> fullProtectionCmd(std::vector<unsigned char> cmd, std::vector<unsigned char> data);

			std::vector<unsigned char> d_macSessionKey;

			std::vector<unsigned char> d_lastIV;

			unsigned int d_cmdCtr;
	};
}

#endif /* LOGICALACCESS_SAMAV2ISO7816COMMANDS_HPP */

