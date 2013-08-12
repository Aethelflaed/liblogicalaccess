/**
 * \file desfireiso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP

#include "samav2commands.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samcrypto.hpp"
#include "samav2keyentry.hpp"
#include "samcrypto.hpp"

#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{		

	struct SAMManufactureInformation
	{
		unsigned char	uniqueserialnumber[7];
		unsigned char	productionbatchnumber[5];
		unsigned char	dayofproduction;
		unsigned char	monthofproduction;
		unsigned char	yearofproduction;
		unsigned char	globalcryptosettings;
		unsigned char	modecompatibility;
	};

	struct SAMVersionInformation
	{
		unsigned char	vendorid;
		unsigned char	type;
		unsigned char	subtype;
		unsigned char	majorversion;
		unsigned char	minorversion;
		unsigned char	storagesize;
		unsigned char	protocoltype;
	};

	struct SAMVersion
	{
		SAMVersionInformation		hardware;
		SAMVersionInformation		software;
		SAMManufactureInformation	manufacture;
	};
	
	/**
	 * \brief The DESFire base commands class.
	 */
	class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMAV2Commands
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

			virtual void getVersion();

			virtual boost::shared_ptr<SAMAV2KeyEntry> getKeyEntry(unsigned int keyno);
			virtual boost::shared_ptr<SAMAV2KucEntry> getKUCEntry(unsigned int keyno);

			virtual void changeKUCEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KucEntry> keyentry, boost::shared_ptr<DESFireKey> key);
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KeyEntry> keyentry, boost::shared_ptr<DESFireKey> key);

			virtual void activeAV2Mode();
			virtual void selectApplication(unsigned char *aid);
			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			void authentificateHost_AES_3K3DES(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			void authentificateHostDES(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			virtual void disableKeyEntry(unsigned char keyno);
			virtual std::vector<unsigned char> dumpSessionKey();

			boost::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() { return boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter()); };
			virtual std::string getSAMTypeFromSAM();

			boost::shared_ptr<SAMDESfireCrypto> getCrypto() { return d_crypto; };
			void setCrypto(boost::shared_ptr<SAMDESfireCrypto> t) { d_crypto = t; };

			virtual std::vector<unsigned char> decipherData(std::vector<unsigned char> data, bool islastdata);
			virtual std::vector<unsigned char> encipherData(std::vector<unsigned char> data, bool islastdata);

		protected:
			boost::shared_ptr<SAMDESfireCrypto> d_crypto;
	};
}

#endif /* LOGICALACCESS_DESFIREISO7816COMMANDS_HPP */

