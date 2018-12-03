/**
 * \file samav2chip.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2Chip header.
 */

#ifndef LOGICALACCESS_SAMAV2CHIP_HPP
#define LOGICALACCESS_SAMAV2CHIP_HPP

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/samav2/samcommands.hpp>
#include <logicalaccess/plugins/cards/samav2/samchip.hpp>

namespace logicalaccess
{
#define CHIP_SAMAV2 "SAM_AV2"

/**
 * \brief The SAM chip class.
 */
class LLA_CARDS_SAMAV2_API SAMAV2Chip : public SAMChip
{
  public:
    /**
     * \brief Constructor.
     */
    SAMAV2Chip();

    explicit SAMAV2Chip(std::string ct);

    /**
     * \brief Destructor.
     */
    ~SAMAV2Chip();

    std::shared_ptr<SAMCommands<KeyEntryAV2Information, SETAV2>> getSAMAV2Commands() const
    {
        return std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(
            getCommands());
    }
};
}

#endif /* LOGICALACCESS_SAMAV2CHIP_HPP */