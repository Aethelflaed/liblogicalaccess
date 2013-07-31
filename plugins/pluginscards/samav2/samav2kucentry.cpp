/**
 * \file desfirekey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire Key.
 */

#include "samav2kucentry.hpp"

namespace logicalaccess
{
	boost::shared_ptr<KucEntryUpdateSettings> SAMAV2KucEntry::getUpdateSettings()
	{
		KucEntryUpdateSettings *settings = new KucEntryUpdateSettings;

		bool *x = (bool*)settings;
		for (unsigned char i = 0; i < sizeof(*settings); ++i)
		{
			if ((d_updatemask & 0x80) == 0x80)
				x[i] = 1;
			else
				x[i] = 0;
			if (i + 1 < sizeof(*settings))
				d_updatemask = d_updatemask << 1;
		}
		return boost::shared_ptr<KucEntryUpdateSettings>(settings);
	}

	void	SAMAV2KucEntry::setUpdateSettings(boost::shared_ptr<KucEntryUpdateSettings> t)
	{
		bool *x = (bool*)&*t;
		d_updatemask = 0;
		for (unsigned char i = 0; i < sizeof(t); ++i)
		{
			d_updatemask += x[i];
			if (i + 1 < sizeof(t))
				d_updatemask = d_updatemask << 1;
		}
	}
}