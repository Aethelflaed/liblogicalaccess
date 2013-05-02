/**
 * \file SCMReaderUnit.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief SCM Reader unit.
 */

#ifndef LOGICALACCESS_SCMREADERUNIT_H
#define LOGICALACCESS_SCMREADERUNIT_H

#include "../PCSCReaderUnit.h"

#include <string>
#include <vector>
#include <iostream>

namespace LOGICALACCESS
{
	/**
	 * \brief The SCM reader unit class.
	 */
	class LIBLOGICALACCESS_API SCMReaderUnit : public PCSCReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			SCMReaderUnit(const std::string& name);

			/**
			 * \brief Destructor.
			 */
			virtual ~SCMReaderUnit();			

			/**
			 * \brief Get the PC/SC reader unit type.
			 * \return The PC/SC reader unit type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const;


		protected:
			
	};
}

#endif
