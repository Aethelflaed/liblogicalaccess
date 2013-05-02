/**
 * \file ISO15693Commands.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief ISO15693 commands.
 */

#ifndef LOGICALACCESS_ISO15693COMMANDS_H
#define LOGICALACCESS_ISO15693COMMANDS_H

#include "logicalaccess/Key.h"
#include "ISO15693Location.h"
#include "logicalaccess/Cards/Commands.h"

namespace LOGICALACCESS
{
	/**
	 * \brief The ISO15693 commands class.
	 */
	class LIBLOGICALACCESS_API ISO15693Commands : public Commands
	{
		public:

			/**
			 * \brief ISO15693 System information.
			 */
			struct SystemInformation
			{
				bool hasDSFID; /**< \brief The DSFID is supported */
				bool hasAFI; /**< \brief The AFI is supported */
				bool hasVICCMemorySize; /**< \brief The VICC memory size is supported */	
				bool hasICReference; /**< \brief The IC reference is supported */
				unsigned char ICReference; /**< \brief The IC reference */
				int blockSize; /**< \brief The block size in bytes */
				int nbBlocks; /**< \brief The number of blocks */				
				unsigned char AFI; /**< \brief The Application Family Identifier */
				unsigned char DSFID; /**< \brief The Data Storage Format Identifier */
			};

			virtual void stayQuiet() = 0;

			virtual bool readBlock(size_t block, void* data, size_t datalen, size_t le = 0) = 0;

			virtual bool writeBlock(size_t block, const void* data, size_t datalen) = 0;

			virtual void lockBlock(size_t block) = 0;

			virtual void writeAFI(size_t afi) = 0;

			virtual void lockAFI() = 0;

			virtual void writeDSFID(size_t dsfid) = 0;

			virtual void lockDSFID() = 0;

			virtual ISO15693Commands::SystemInformation getSystemInformation() = 0;

			virtual unsigned char getSecurityStatus(size_t block) = 0;
	};
}

#endif
