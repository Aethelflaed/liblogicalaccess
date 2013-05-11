/**
 * \file DataRepresentation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A Data Representation.
 */

#include "logicalaccess/Services/AccessControl/Encodings/DataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Encodings/LittleEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Encodings/NoDataRepresentation.h"

namespace logicalaccess
{
	DataRepresentation* DataRepresentation::getByEncodingType(EncodingType type)
	{
		switch (type)
		{
		case ET_BIGENDIAN:
			return new BigEndianDataRepresentation();
			break;

		case ET_LITTLEENDIAN:
			return new LittleEndianDataRepresentation();
			break;

		case ET_NOENCODING:
			return new NoDataRepresentation();
			break;

		default:
			return NULL;
			break;
		}
	}
}

