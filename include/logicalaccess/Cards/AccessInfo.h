/**
 * \file AccessInfo.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AccessInfo.
 */

#ifndef LOGICALACCESS_ACCESSINFO_H
#define LOGICALACCESS_ACCESSINFO_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/linearizable.h"
#include "logicalaccess/Key.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief A Access informations. Describe key used of authentication and access rights for a specific or a group of operati
	 * \remarks Linearizable is depreciated, shouldn't be used anymore in further versions.
	 */
	class LIBLOGICALACCESS_API AccessInfo : public Linearizable, public XmlSerializable
	{
		public:

			/**
			 * \brief Constructor.
			 */
			AccessInfo();

			/**
			 * \brief Generate pseudo-random access informations.
			 */
			virtual void generateInfos() = 0;

			/**
			 * \brief Generate pseudo-random access informations for a given card serial number.
			 * \param csn The card serial number, as a salt (no diversification here).
			 */
			virtual void generateInfos(const string& csn) = 0;

			/**
			 * \brief Access informations data size.
			 */
			virtual size_t getDataSize() = 0;

			/**
			 * \brief Get the card type for this access infos.
			 * \return The card type.
			 */
			virtual std::string getCardType() = 0;

			/**
			 * \brief Generate a key data.
			 * \param seed Seed used for random numbers.
			 * \param keySize The key size.
			 * \return A generated pseudo-random key.
			 */
			static std::string generateSimpleKey(long seed, size_t keySize);

			/**
			 * \brief Generate a DES key data.
			 * \param seed Seed used for random numbers.
			 * \param keySize The key size.
			 * \return A generated pseudo-random key.
			 */
			static std::string generateSimpleDESKey(long seed, size_t keySize);

			/**
			 * \brief Equality operator
			 * \param ai Access infos to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const AccessInfo& ai) const;

			/**
			 * \brief Inequality operator
			 * \param ai Access infos to compare.
			 * \return True if inequals, false otherwise.
			 */
			inline bool operator!=(const AccessInfo& ai) const { return !operator==(ai); };

	};	
}

#endif /* LOGICALACCESS_ACCESSINFO_H */
