#include "mifare_cl1356_commands.hpp"

namespace logicalaccess
{
void MifareCL1356Commands::increment(uint8_t blockno, uint32_t value)
{
    std::vector<unsigned char> buf;
    buf.push_back(0x01);
    // Documentation states that the reader expects a signed 4byte number.
    int32_t num = static_cast<int32_t>(value);

    buf.push_back(static_cast<unsigned char>((num >> 24) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>(num & 0xff));

    getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD7, 0x00, blockno, 0x05,
                                                buf);
}

void MifareCL1356Commands::decrement(uint8_t blockno, uint32_t value)
{
    std::vector<unsigned char> buf;
    buf.push_back(0x02);
    // Documentation states that the reader expects a signed 4byte number.
    int32_t num = static_cast<int32_t>(value);

    buf.push_back(static_cast<unsigned char>((num >> 24) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>(num & 0xff));

    getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD7, 0x00, blockno, 0x05,
                                                buf);
}

void MifareCL1356Commands::authenticate(unsigned char blockno,
                                        unsigned char keyno,
                                        MifareKeyType keytype)
{
	std::vector<unsigned char> buf;
	buf.push_back(0x01);
	buf.push_back(0x00); // MSB. But we have 1 byte, so MSB is 0.
	buf.push_back(blockno); // LSB
	buf.push_back(keytype);
	buf.push_back(keyno);

	getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x86, 0x00, 0x00, 0x05,
		buf);
}
}
