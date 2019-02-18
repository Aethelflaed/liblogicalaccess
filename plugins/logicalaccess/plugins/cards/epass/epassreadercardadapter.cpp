#include <logicalaccess/plugins/cards/epass/epassreadercardadapter.hpp>
#include <logicalaccess/plugins/cards/epass/epasscrypto.hpp>
#include <cassert>
using namespace logicalaccess;

EPassReaderCardAdapter::EPassReaderCardAdapter() {}

ByteVector EPassReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    ByteVector r;
    if (crypto_ && crypto_->secureMode())
        r = crypto_->decrypt_rapdu(answer);
    else
        r = answer;
    return r;
}

ByteVector EPassReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    if (crypto_ && crypto_->secureMode())
        return crypto_->encrypt_apdu(command);
    return command;
}

void EPassReaderCardAdapter::setEPassCrypto(std::shared_ptr<EPassCrypto> crypto)
{
    crypto_ = crypto;
}
