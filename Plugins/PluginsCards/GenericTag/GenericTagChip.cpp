/**
 * \file GenericTagChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Generic tag chip.
 */

#include "GenericTagChip.h"

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
	GenericTagChip::GenericTagChip()
		: Chip(CHIP_GENERICTAG)
	{
		d_tagIdBitsLength = 0;
	}

	GenericTagChip::~GenericTagChip()
	{
	}

	boost::shared_ptr<LocationNode> GenericTagChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());

		rootNode->setName("RFID Tag");

		return rootNode;
	}
}
