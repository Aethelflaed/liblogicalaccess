/**
 * \file IndalaChip.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Indala chip.
 */

#ifndef LOGICALACCESS_INDALACHIP_H
#define LOGICALACCESS_INDALACHIP_H

#include "logicalaccess/Cards/Chip.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	/**
	 * \brief The Indala base chip class.
	 */
	class LIBLOGICALACCESS_API IndalaChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			IndalaChip();

			/**
			 * \brief Destructor.
			 */
			virtual ~IndalaChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return "Indala"; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

		protected:

	};
}

#endif
