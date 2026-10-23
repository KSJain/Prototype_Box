#pragma once

namespace RAMUPairingProtocol
{
    constexpr const char* SERVICE_UUID =
        "8e7c0001-5e4b-4c55-9f2e-52f4d2c10001";

    constexpr const char* REQUEST_UUID =
        "8e7c0002-5e4b-4c55-9f2e-52f4d2c10001";

    constexpr const char* RESULT_UUID =
        "8e7c0003-5e4b-4c55-9f2e-52f4d2c10001";

    constexpr const char* IDENTITY_UUID =
        "8e7c0004-5e4b-4c55-9f2e-52f4d2c10001";

    constexpr const char* CAPABILITIES_UUID =
        "8e7c0005-5e4b-4c55-9f2e-52f4d2c10001";

    constexpr const char* PROTOCOL_VERSION =
        "ramu-pairing-lab-v0.4";

    // Public human-verification material carried in the temporary
    // BLE advertisement. It is deliberately NOT a secret.
    constexpr const char* ADVERTISEMENT_PREFIX =
        "RAMU|";
}
