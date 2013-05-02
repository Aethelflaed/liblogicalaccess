#include <string>
#include <boost/shared_ptr.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "EM4135Chip.h"

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this macro, to avoid MSVC specific warnings pragma */
#endif
#endif

extern "C"
{
	LIBLOGICALACCESS_API char *getLibraryName()
	{
		return (char *)"EM4135";
	}
	
	LIBLOGICALACCESS_API void getEM4135Chip(boost::shared_ptr<LOGICALACCESS::Chip>* chip)
	{
		if (chip != NULL)
		{
			*chip = boost::shared_ptr<LOGICALACCESS::EM4135Chip>(new LOGICALACCESS::EM4135Chip());
		}
	}
}
