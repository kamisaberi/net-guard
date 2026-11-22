// File: /home/kami/net-guard/include/net_guard/defense/slice_airgap.hpp

#pragma once

#include <atomic>

namespace net_guard::defense {

class SliceAirGap {
public:
    void trip() noexcept { tripped_.store(true, std::memory_order_release); }
    void reset() noexcept { tripped_.store(false, std::memory_order_release); }
    [[nodiscard]] bool is_tripped() const noexcept { return tripped_.load(std::memory_order_acquire); }

private:
    std::atomic<bool> tripped_{false};
};

} // namespace net_guard::defense