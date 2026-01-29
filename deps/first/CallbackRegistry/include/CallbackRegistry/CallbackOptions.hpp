#pragma once
#include <Helpers/String.hpp>

namespace RC
{
    // Options for registering callbacks. Default constructible, but highly recommended to set at least
    // OwnerModName and HookName for debugging and logging; this will make finding issues
    // and conflicts much easier, especially if an end user can supply a log.
    struct CallbackOptions
    {
        // Set to true to only execute this callback once, then automatically remove it.
        bool bOnce = false;

        // Set to true to ensure that this callback executes after all other callbacks of the same type
        // (meaning that if it's a posthook, it executes after all other posthook callbacks, and if it's
        // a prehook, it executes after all other prehook callbacks) and that it won't modify the return
        // value or try to prevent the original function call, or otherwise call any non-const function
        // of ICallbackIterationData
        bool bReadonly = false;

        // Set to the name of the registering mod for logging/debugging
        StringType OwnerModName;

        // Set the name of this particular callback for logging/debugging
        StringType HookName;
    };
}