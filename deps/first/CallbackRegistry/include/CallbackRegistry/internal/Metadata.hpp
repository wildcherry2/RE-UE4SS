#pragma once
#include <Helpers/String.hpp>
#include <CallbackRegistry/CallbackOptions.hpp>
#include <CallbackRegistry/GlobalCallbackId.hpp>

namespace RC
{
    // Metadata for registered callbacks
    struct CallbackDataBase
    {
        GlobalCallbackId id{};
        StringType owner_mod_name;
        StringType hook_name;

        // Packed state for executor-count + invalidation flag.
        // High bit: invalid (1 = invalid/removed)
        // Low 63 bits: number of threads currently executing this callback
        static constexpr uint64_t INVALID_BIT = 1ull << 63;
        static constexpr uint64_t EXECUTOR_MASK = ~INVALID_BIT;

        // Highest bit = invalid bit, all other bits = number of threads currently executing this callback
        mutable std::atomic<uint64_t> state{0};

        [[nodiscard]] bool IsInvalid(const std::memory_order order = std::memory_order_acquire) const noexcept
        {
            return (state.load(order) & INVALID_BIT) != 0;
        }

        [[nodiscard]] uint64_t GetExecutorCount(const std::memory_order order = std::memory_order_acquire) const noexcept
        {
            return state.load(order) & EXECUTOR_MASK;
        }

        // Attempts to begin executing this callback.
        // If bOnce is true, this also atomically claims the callback (sets invalid) so exactly one thread executes it.
        // Returns true iff the caller should proceed to execute the callback.
        [[nodiscard]] bool TryBeginExecution(const bool bOnce, bool& out_invalidated_by_once) noexcept
        {
            out_invalidated_by_once = false;

            uint64_t cur = state.load(std::memory_order_acquire);
            for (;;)
            {
                if (cur & INVALID_BIT) return false;

                const uint64_t exec = (cur & EXECUTOR_MASK);
                if (exec == EXECUTOR_MASK) [[unlikely]]
                    return false; // would overflow into INVALID_BIT

                uint64_t desired = cur + 1;
                if (bOnce) desired |= INVALID_BIT;

                if (state.compare_exchange_weak(cur, desired, std::memory_order_acq_rel, std::memory_order_acquire))
                {
                    out_invalidated_by_once = bOnce;
                    return true;
                }
            }
        }

        // Marks this callback as invalid. Returns true iff this call transitioned it from valid -> invalid.
        bool Invalidate() noexcept
        {
            return (state.fetch_or(INVALID_BIT, std::memory_order_acq_rel) & INVALID_BIT) == 0;
        }

        // Ends an execution region started by TryBeginExecution().
        void EndExecution() noexcept
        {
            const uint64_t prev = state.fetch_sub(1, std::memory_order_acq_rel);
            if ((prev & EXECUTOR_MASK) == 1)
            {
                state.notify_all();
            }
        }

        // Waits until all in-flight executions are finished.
        void WaitForExecutorsToFinish() const noexcept
        {
            uint64_t cur = state.load(std::memory_order_acquire);
            while ((cur & EXECUTOR_MASK) != 0)
            {
                state.wait(cur, std::memory_order_acquire);
                cur = state.load(std::memory_order_acquire);
            }
        }

        CallbackDataBase() = delete;

        [[nodiscard]] CallbackDataBase(const GlobalCallbackId id, StringType owner_mod_name, StringType hook_name)
            : id(id), owner_mod_name(std::move(owner_mod_name)), hook_name(std::move(hook_name))
        {
        }
    };

    // Metadata for registered callbacks + the actual typed callback
    template <typename CallbackType>
    struct CallbackData : CallbackDataBase
    {
        CallbackType Callback;

        CallbackData(const GlobalCallbackId id, CallbackType callback, CallbackOptions options) noexcept
            : CallbackDataBase(id, std::move(options.OwnerModName), std::move(options.HookName)), Callback(std::move(callback)) {}
    };
} // namespace RC