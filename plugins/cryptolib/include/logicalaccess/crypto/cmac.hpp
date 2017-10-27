#ifndef CMAC_HPP__
#define CMAC_HPP__

#include <vector>
#include <memory>
#include "logicalaccess/crypto/openssl.hpp"
#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"
#include "logicalaccess/lla_fwd.hpp"

namespace logicalaccess
{
namespace openssl
{
class CMACCrypto
{
  public:
    CMACCrypto()
    {
        OpenSSLInitializer::GetInstance();
    }
    ~CMACCrypto()
    {
    }

    /*
     * \brief Calculate CMAC for a crypto algorithm and key.
     * \param key The key to use.
     * \param crypto The crypto algorithm (3DES, AES, ...).
     * \param data The data buffer to calculate CMAC.
     * \param padding_size The padding size.
     * \return The CMAC result for the message.
     */
    static ByteVector cmac(const ByteVector &key, std::string crypto,
                           const ByteVector &data, const ByteVector &iv = {},
                           int padding_size = 0);

    /**
     * \brief  Return data part for the encrypted communication mode.
     * \param key The key to use, shall be the session key from the previous
     * authentication
     * \param cipher The cypher to use
     * \param block_size The bloc size
     * \param data The data source buffer to calculate MAC
     * \param lastIV The last initialisation vector
     * \return The MAC result for the message.
     */
    static ByteVector cmac(const ByteVector &key,
                           std::shared_ptr<OpenSSLSymmetricCipher> cipherMAC,
                           unsigned int block_size, const ByteVector &data,
                           ByteVector lastIV, unsigned int padding_size,
                           bool forceK2Use = false);

    /**
     * \brief Shift a string.
     * \param buf The buffer string
     * \param xorparam The optional xor for the string.
     */
    static ByteVector shift_string(const ByteVector &buf, unsigned char xorparam = 0x00);
};
}
}

#endif