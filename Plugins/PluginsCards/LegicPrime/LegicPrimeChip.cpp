/**
 * \file LegicPrimeChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Legic Prime chip.
 */

#include "LegicPrimeChip.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

namespace LOGICALACCESS
{
	LegicPrimeChip::LegicPrimeChip()
		: Chip("LegicPrime")
	{
	}

	LegicPrimeChip::~LegicPrimeChip()
	{
	}

	boost::shared_ptr<LocationNode> LegicPrimeChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("Legic Prime");

		return rootNode;
	}
}
