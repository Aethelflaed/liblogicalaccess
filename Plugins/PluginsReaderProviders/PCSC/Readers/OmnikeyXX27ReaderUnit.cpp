/**
 * \file OmnikeyXX27ReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Omnikey XX27 reader unit.
 */

#include "../Readers/OmnikeyXX27ReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>


namespace LOGICALACCESS
{
	OmnikeyXX27ReaderUnit::OmnikeyXX27ReaderUnit(const std::string& name)
		: OmnikeyReaderUnit(name)
	{
		
	}

	OmnikeyXX27ReaderUnit::~OmnikeyXX27ReaderUnit()
	{
		
	}	

	PCSCReaderUnitType OmnikeyXX27ReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_OMNIKEY_XX27;
	}

	boost::shared_ptr<ReaderCardAdapter> OmnikeyXX27ReaderUnit::getReaderCardAdapter(std::string type)
	{
		return getDefaultReaderCardAdapter();
	}
}
