#pragma once

#include <chrono>
#include <sstream>

#include <kx/kx.h>

#include "untyped_span.h"
#include "adapter.h"

namespace qbind 
{

/**
 * @brief K type used to map type codes to underlying types.
 * 
 * @tparam TypeCode : K type code
 * @tparam UnderlierType : Underlying type, as specified by k0 struct.
 * @tparam DefaultAdapter : Default convert from underlying to useful type and back.
 */
template<size_t TypeCode, class UnderlierType, class DefaultTarget = UnderlierType>
struct Type
{
    static constexpr size_t Code = TypeCode;
    static constexpr size_t Size = sizeof(UnderlierType);
    using Underlier = UnderlierType;
    using Adapter = Adapter<UnderlierType, DefaultTarget>;
};

using Mixed =       Type<0, K, Adapter<K, UntypedSpan>>;
using Boolean =     Type<KB, bool>;
using Guid =        Type<UU, U, std::array<uint8_t, 16>>;
using Byte =        Type<KG, uint8_t>;
using Short =       Type<KH, int16_t>;
using Integer =     Type<KI, int32_t>;
using Long =        Type<KJ, int64_t>;
using Real =        Type<KE, float>;
using Float =       Type<KF, double>;
using Char =        Type<KC, char>;
using Symbol =      Type<KS, char *, std::string_view>;

using Timestamp =   Type<KP, int64_t, chrono::Timestamp>;
using Month =       Type<KM, int32_t, chrono::Month>;
using Date =        Type<KD, int32_t, chrono::Date>;

using Timespan =    Type<KN, int64_t, chrono::Timespan>;
using Minute =      Type<KU, int32_t, chrono::Minute>;
using Second =      Type<KV, int32_t, chrono::Second>;
using Time =        Type<KT, int32_t, chrono::Time>;

// TODO: Find a better default
using Datetime =    Type<KZ, double>;

using Error =       Type<128, char *, std::runtime_error>;

}