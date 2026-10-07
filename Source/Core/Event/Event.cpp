#include "Event.hpp"

namespace adh {
    event::Bus& EventBus() noexcept {
        static event::Bus bus;
        return bus;
    }
} // namespace adh
