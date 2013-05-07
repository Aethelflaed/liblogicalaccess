/**
 * \file AxessTMCLegicReaderProvider.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AxessTMCLegic card reader provider. 
 */

#ifndef LOGICALACCESS_READERAXESSTMCLEGIC_PROVIDER_H
#define LOGICALACCESS_READERAXESSTMCLEGIC_PROVIDER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "AxessTMCLegicReaderUnit.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{	
	#define READER_AXESSTMCLEGIC		"AxessTMCLegic"

	/**
	 * \brief AxessTMCLegic Reader Provider class.
	 */
	class LIBLOGICALACCESS_API AxessTMCLegicReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the AxessTMCLegicReaderProvider instance. Singleton because we can only have one COM communication, and AxessTMCLegic can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<AxessTMCLegicReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~AxessTMCLegicReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_AXESSTMCLEGIC; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "AxessTMC Legic"; };

			/**
			 * \brief List all readers of the system.
			 * \return True if the list was updated, false otherwise.
			 */
			virtual bool refreshReaderList();

			/**
			 * \brief Get reader list for this reader provider.
			 * \return The reader list.
			 */
			virtual const ReaderList& getReaderList() { return d_readers; };					
			
			/**
			 * \brief Create a new reader unit for the reader provider.
			 * \return A reader unit.
			 */
			virtual boost::shared_ptr<ReaderUnit> createReaderUnit();			

		protected:

			/**
			 * \brief Constructor.
			 */
			AxessTMCLegicReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERAXESSTMCLEGIC_PROVIDER_H */

