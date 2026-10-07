#pragma once
#include <Event/EventTypes.hpp>

#include <adh/event.hpp>

namespace adh {
    ADH_API event::Bus& EventBus() noexcept;
} // namespace adh
