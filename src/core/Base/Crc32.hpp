/* **********************************************************************
 *
 * Copied from file of same name in Therminator 2 package.
 *
 * *********************************************************************/
#ifndef CAP__CRC32
#define CAP__CRC32
#include <string>

namespace CAP
{

//!
//! @brief 32-bit CRC (cyclic redundancy check) checksum.
//!
//! Uses the CRC-32-IEEE 802.3 polynomial
//! @f[ X^{32} + X^{26} + X^{23} + X^{22} + X^{16} + X^{12} + X^{11} + X^{10} + X^8 + X^7 + X^5 + X^4 + X^2 + X + 1 @f]
//! (see <a href=http://en.wikipedia.org/wiki/Cyclic_redundancy_check>CRC @ Wikipedia</a>).
//!
//! Data are fed with update(), possibly in several pieces; finish() then completes the checksum. The class is not part
//! of the Base library build (Crc32.cpp is not listed in CMakeLists.txt); the CAP binary format uses its own CRC
//! (BinaryIO.hpp).
//!
//! Example:
//! @code
//!   CAP::Crc32 crc("hello",5);
//!   unsigned int sum = crc.finish();
//! @endcode
//!
class Crc32
{
  public:
    //! @brief Default constructor: start a new checksum.
    Crc32();
    //! @brief Start a new checksum with a block of data. @param aData data @param aSize number of bytes
    Crc32(const char* aData, unsigned int aSize);
    //! @brief Destructor.
    ~Crc32();

    //! @brief Add a block of data to the checksum. @param aData data @param aSize number of bytes
    void	 update(const char* aData, unsigned int aSize);
    //! @brief Complete the checksum and return it.
    //! @warning Call it once: it inverts the stored value, so further update() or finish() calls give invalid results.
    //! @return checksum
    unsigned int finish() const;
    //! @brief Current value (the checksum, once finish() was called). @return value
    unsigned int value() const;
    //! @brief Current value as an 8-character upper-case hexadecimal string.
    //! @return string, valid until the next call of valueHex() or the destruction of this object
    const char*  valueHex() const;

  private:
    mutable unsigned int      mCrc32; //!< running value
    mutable std::string       mHex;   //!< storage backing the pointer returned by valueHex()
    static const unsigned int mCrc32Tab[]; //!< lookup table of the polynomial
};

} // namespace CAP

#endif
