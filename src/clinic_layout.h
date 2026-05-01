#pragma once

#include <array>
#include <string>

#include "components.h"

struct ClinicLayoutRoom {
    const char* label = "";
    const char* access = "";
    const char* detail = "";
    TransformComponent local_bounds{};
};

inline constexpr std::array<ClinicLayoutRoom, 4> kClinicLayoutRooms{{
    ClinicLayoutRoom{
        "INTAKE",
        "PUBLIC",
        "front triage desk",
        TransformComponent{-28.0f, 18.0f, 34.0f, 18.0f}
    },
    ClinicLayoutRoom{
        "TREATMENT",
        "PUBLIC",
        "partitioned care bay",
        TransformComponent{18.0f, 14.0f, 42.0f, 22.0f}
    },
    ClinicLayoutRoom{
        "RECORDS",
        "STAFF",
        "ledger archive",
        TransformComponent{-24.0f, -18.0f, 36.0f, 20.0f}
    },
    ClinicLayoutRoom{
        "SERVICE",
        "STAFF",
        "supply and staff corridor",
        TransformComponent{24.0f, -20.0f, 34.0f, 18.0f}
    }
}};

inline std::string clinicLayoutReadout() {
    return "CLINIC LAYOUT: INTAKE PUBLIC; TREATMENT PUBLIC; RECORDS STAFF; SERVICE STAFF";
}

inline std::string clinicRestrictedBoundaryBaseReadout() {
    return "CLINIC BOUNDARY: RECORDS STAFF ONLY";
}
