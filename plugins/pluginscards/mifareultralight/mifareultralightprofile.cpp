/**
 * \file mifareultralightprofile.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight card profiles.
 */

#include "mifareultralightprofile.hpp"

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "mifareultralightlocation.hpp"
#include "mifareultralightaccessinfo.hpp"

namespace logicalaccess
{
    MifareUltralightProfile::MifareUltralightProfile()
        : Profile()
    {
    }

    MifareUltralightProfile::~MifareUltralightProfile()
    {
    }

    void MifareUltralightProfile::setDefaultKeys()
    {
    }

    void MifareUltralightProfile::setDefaultKeysAt(boost::shared_ptr<Location> /*location*/)
    {
    }

    void MifareUltralightProfile::clearKeys()
    {
    }

    void MifareUltralightProfile::setKeyAt(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*AccessInfo*/)
    {
    }

    boost::shared_ptr<AccessInfo> MifareUltralightProfile::createAccessInfo() const
    {
        boost::shared_ptr<MifareUltralightAccessInfo> ret;
        ret.reset(new MifareUltralightAccessInfo());
        return ret;
    }

    boost::shared_ptr<Location> MifareUltralightProfile::createLocation() const
    {
        boost::shared_ptr<MifareUltralightLocation> ret;
        ret.reset(new MifareUltralightLocation());
        return ret;
    }

    FormatList MifareUltralightProfile::getSupportedFormatList()
    {
        FormatList list = Profile::getHIDWiegandFormatList();
        return list;
    }
}