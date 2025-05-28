#pragma once

#include <library/cpp/actors/core/events.h>

namespace TEvents {
    struct TEvDone : NActors::TEventLocal<TEvDone, 1> {};

    struct TEvWriteValueRequest : NActors::TEventLocal<TEvWriteValueRequest, 2> {
        int64_t Value;
        TEvWriteValueRequest(int64_t value) : Value(value) {}
    };
}

