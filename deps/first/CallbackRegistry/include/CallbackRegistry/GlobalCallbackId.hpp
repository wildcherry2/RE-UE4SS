#pragma once
#include <cstdint>

namespace RC
{
    // ID type returned from registering a callback that can be used to unregister a callback via UnregisterCallback
    using GlobalCallbackId = uint64_t;

    // Error sentinel for invalid GlobalCallbackIds
    inline static constexpr GlobalCallbackId ERROR_ID = 0;
}
