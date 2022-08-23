#include "layline/network/GNetworkAddress.h"

namespace rev {

bool NetworkAddress::isAny() const
{
    if (isIpv4()) {
        return *as<NetworkAddressV4>() == NetworkAddressV4::Any();
    }
    else {
        return *as<NetworkAddressV6>() == NetworkAddressV6::Any();
    }
}

} // End namespace rev