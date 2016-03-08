#pragma once

#include "../iso7816/iso7816chip.hpp"
#include "epass_command.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API EPassChip : public ISO7816Chip
{
  public:
    EPassChip();
    virtual ~EPassChip() = default;

    std::shared_ptr<EPassCommand> getEPassCommand();

    virtual std::shared_ptr<CardService>
    getService(CardServiceType serviceType) override;
};
}
