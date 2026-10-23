#include "PeerBindingHostService.h"

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include <esp_system.h>

#include "RAMUPairingProtocol.h"

namespace
{
    constexpr uint32_t SCAN_SECONDS = 5;
    constexpr uint32_t RESULT_HOLD_MS = 3500;
    constexpr uint32_t CONFIRM_TIMEOUT_MS = 20000;

    constexpr const char* WORD_A[] = {
        "amber", "brisk", "calm", "copper",
        "gentle", "hidden", "indigo", "lunar",
        "misty", "quiet", "silver", "steady"
    };

    constexpr const char* WORD_B[] = {
        "cedar", "fern", "harbor", "lake",
        "maple", "meadow", "orbit", "pine",
        "river", "stone", "willow", "garden"
    };

    constexpr const char* WORD_C[] = {
        "bee", "comet", "finch", "fox",
        "moth", "owl", "star", "wren",
        "leaf", "moon", "seed", "wave"
    };

    template <typename T, size_t N>
    constexpr size_t countOf(
        const T (&)[N]
    )
    {
        return N;
    }

    String toArduinoString(
        const std::string& value
    )
    {
        return String(
            value.c_str()
        );
    }
}

PeerBindingHostService::PeerBindingHostService(
    KSJ::SSD1306Display& display
)
    : _display(display)
{
}

bool PeerBindingHostService::begin()
{
    if (!_prefs.begin(
        "peer-host",
        false
    ))
    {
        return false;
    }

    ensureHostIdentity();
    loadBinding();

    BLEDevice::init(
        _hostId.c_str()
    );

    Serial.println();
    Serial.println("RAMU Peer Binding Host");
    Serial.println("----------------------");
    Serial.print("Host ID: ");
    Serial.println(_hostId);
    Serial.println(
        "Pairing transport: BLE (temporary only)"
    );
    Serial.println(
        "HMI: Settings > Pair Peer"
    );

    printBinding();
    printHelp();

    return true;
}

void PeerBindingHostService::update(
    uint32_t nowMs
)
{
    if (
        _hmiState == HMIState::Confirm &&
        nowMs - _screenSinceMs >=
            CONFIRM_TIMEOUT_MS
    )
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = nowMs;

        renderFailure(
            "CONFIRM TIMEOUT"
        );

        return;
    }

    if (
        (_hmiState == HMIState::Success ||
         _hmiState == HMIState::Failure) &&
        nowMs - _screenSinceMs >=
            RESULT_HOLD_MS
    )
    {
        releaseDisplay();
    }
}

void PeerBindingHostService::startPairing(
    uint32_t nowMs
)
{
    if (_hmiState != HMIState::Idle)
    {
        return;
    }

    _candidateCode = "";
    _candidateAddress = "";

    _hmiState = HMIState::Scanning;
    _screenSinceMs = nowMs;

    renderScanning();

    if (findArmedPeer())
    {
        _hmiState = HMIState::Confirm;
        _screenSinceMs = millis();

        renderConfirm();
        return;
    }

    _hmiState = HMIState::Failure;
    _screenSinceMs = millis();

    renderFailure(
        "NO ARMED PEER"
    );
}

void PeerBindingHostService::handleInput(
    KSJ::InputEvent event,
    uint32_t nowMs
)
{
    if (_hmiState == HMIState::Confirm)
    {
        if (event == KSJ::InputEvent::ButtonDown)
        {
            _hmiState = HMIState::Working;
            _screenSinceMs = nowMs;

            renderLines(
                "PAIR PEER",
                "BINDING...",
                _candidateCode.c_str()
            );

            pairWithCandidate();
            return;
        }

        if (
            event == KSJ::InputEvent::RotateLeft ||
            event == KSJ::InputEvent::RotateRight
        )
        {
            releaseDisplay();
            return;
        }
    }

    if (
        _hmiState == HMIState::Failure ||
        _hmiState == HMIState::Success
    )
    {
        if (event == KSJ::InputEvent::ButtonDown)
        {
            releaseDisplay();
        }
    }
}

bool PeerBindingHostService::ownsDisplay() const
{
    return _hmiState != HMIState::Idle;
}

bool PeerBindingHostService::consumeDisplayReleased()
{
    const bool released =
        _displayReleased;

    _displayReleased = false;

    return released;
}

void PeerBindingHostService::releaseDisplay()
{
    _hmiState = HMIState::Idle;
    _candidateCode = "";
    _candidateAddress = "";
    _displayReleased = true;
}

bool PeerBindingHostService::findArmedPeer()
{
    Serial.println();
    Serial.println(
        "PAIR HMI: intentional one-shot BLE scan..."
    );

    BLEScan* scan =
        BLEDevice::getScan();

    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(80);

    BLEScanResults found =
        scan->start(
            SCAN_SECONDS,
            false
        );


    for (
        int i = 0;
        i < found.getCount();
        ++i
    )
    {
        BLEAdvertisedDevice device =
            found.getDevice(i);

        if (
            !device.haveManufacturerData()
        )
        {
            continue;
        }

        const String manufacturerData =
            String(
                device
                    .getManufacturerData()
                    .c_str()
            );

        const String prefix =
            RAMUPairingProtocol::
                ADVERTISEMENT_PREFIX;

        if (
            !manufacturerData.startsWith(
                prefix
            )
        )
        {
            continue;
        }

        const String code =
            manufacturerData.substring(
                prefix.length()
            );

        if (code.length() != 6)
        {
            continue;
        }

        _candidateCode =
            code;

        _candidateAddress =
            String(
                device
                    .getAddress()
                    .toString()
                    .c_str()
            );

        Serial.print(
            "PAIR HMI: armed peer found at "
        );

        Serial.println(
            _candidateAddress
        );

        Serial.print(
            "PAIR HMI: verification code "
        );

        Serial.println(
            _candidateCode
        );

        scan->clearResults();

        return true;
    }

    scan->clearResults();

    Serial.println(
        "PAIR HMI: no armed RAMU peer found."
    );

    return false;
}

void PeerBindingHostService::pairWithCandidate()
{
    pairWithCode(
        _candidateCode,
        _candidateAddress
    );
}

void PeerBindingHostService::pairWithCode(
    const String& code,
    const String& address
)
{
    if (code.length() != 6)
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            "BAD CODE"
        );

        return;
    }

    BLEScan* scan =
        BLEDevice::getScan();

    scan->setActiveScan(true);

    BLEAdvertisedDevice* target =
        nullptr;

    BLEUUID serviceUuid(
        RAMUPairingProtocol::
            SERVICE_UUID
    );

    BLEScanResults found =
        scan->start(
            SCAN_SECONDS,
            false
        );

    for (
        int i = 0;
        i < found.getCount();
        ++i
    )
    {
        BLEAdvertisedDevice device =
            found.getDevice(i);

        if (
            !device.haveManufacturerData()
        )
        {
            continue;
        }

        const String manufacturerData =
            String(
                device
                    .getManufacturerData()
                    .c_str()
            );

        const String prefix =
            RAMUPairingProtocol::
                ADVERTISEMENT_PREFIX;

        if (
            !manufacturerData.startsWith(
                prefix
            )
        )
        {
            continue;
        }

        const String foundAddress =
            String(
                device
                    .getAddress()
                    .toString()
                    .c_str()
            );

        if (
            address.length() == 0 ||
            foundAddress == address
        )
        {
            target =
                new BLEAdvertisedDevice(
                    device
                );

            break;
        }
    }

    scan->clearResults();

    if (target == nullptr)
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            "PEER VANISHED"
        );

        return;
    }

    BLEClient* client =
        BLEDevice::createClient();

    if (!client->connect(target))
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            "CONNECT FAIL"
        );

        delete target;
        return;
    }

    BLERemoteService* service =
        client->getService(
            serviceUuid
        );

    if (service == nullptr)
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            "SERVICE LOST"
        );

        client->disconnect();
        delete target;
        return;
    }

    BLERemoteCharacteristic* request =
        service->getCharacteristic(
            BLEUUID(
                RAMUPairingProtocol::
                    REQUEST_UUID
            )
        );

    BLERemoteCharacteristic* result =
        service->getCharacteristic(
            BLEUUID(
                RAMUPairingProtocol::
                    RESULT_UUID
            )
        );

    BLERemoteCharacteristic* identity =
        service->getCharacteristic(
            BLEUUID(
                RAMUPairingProtocol::
                    IDENTITY_UUID
            )
        );

    BLERemoteCharacteristic* capabilities =
        service->getCharacteristic(
            BLEUUID(
                RAMUPairingProtocol::
                    CAPABILITIES_UUID
            )
        );

    if (
        request == nullptr ||
        result == nullptr ||
        identity == nullptr ||
        capabilities == nullptr
    )
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            "PROTO ERROR"
        );

        client->disconnect();
        delete target;
        return;
    }

    const String newBindingId =
        createThreeWordId();

    String payload;

    payload +=
        RAMUPairingProtocol::
            PROTOCOL_VERSION;

    payload += "|";
    payload += code;
    payload += "|";
    payload += _hostId;
    payload += "|";
    payload += _hostFingerprint;
    payload += "|";
    payload += newBindingId;

    request->writeValue(
        std::string(
            payload.c_str()
        ),
        true
    );

    delay(120);

    const String resultValue =
        toArduinoString(
            result->readValue()
        );

    if (resultValue != "OK")
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            resultValue.c_str()
        );

        client->disconnect();
        delete target;
        return;
    }

    const String identityValue =
        toArduinoString(
            identity->readValue()
        );

    const int split =
        identityValue.indexOf('|');

    if (split <= 0)
    {
        _hmiState = HMIState::Failure;
        _screenSinceMs = millis();

        renderFailure(
            "IDENTITY ERROR"
        );

        client->disconnect();
        delete target;
        return;
    }

    _peerId =
        identityValue.substring(
            0,
            split
        );

    _peerFingerprint =
        identityValue.substring(
            split + 1
        );

    _capabilities =
        toArduinoString(
            capabilities->readValue()
        );

    _bindingId = newBindingId;
    _hasBinding = true;

    saveBinding();

    Serial.println();
    Serial.println(
        "=== RAMU BINDING ESTABLISHED ==="
    );

    Serial.print("Peer: ");
    Serial.println(_peerId);

    Serial.print("Binding: ");
    Serial.println(_bindingId);

    Serial.print("Capabilities: ");
    Serial.println(_capabilities);

    Serial.println(
        "================================"
    );

    _hmiState = HMIState::Success;
    _screenSinceMs = millis();

    renderSuccess();

    client->disconnect();
    delete target;
}

void PeerBindingHostService::updateSerial()
{
    while (Serial.available() > 0)
    {
        const char c =
            static_cast<char>(
                Serial.read()
            );

        if (
            c == '\r' ||
            c == '\n'
        )
        {
            if (_serialBuffer.length() > 0)
            {
                const String command =
                    _serialBuffer;

                _serialBuffer = "";

                handleCommand(
                    command
                );
            }

            continue;
        }

        if (_serialBuffer.length() < 120)
        {
            _serialBuffer += c;
        }
    }
}

void PeerBindingHostService::handleCommand(
    const String& command
)
{
    if (command == "peers")
    {
        printBinding();
        return;
    }

    if (command == "unpair")
    {
        clearBinding();

        Serial.println(
            "Binding authorization removed locally."
        );

        return;
    }

    if (command == "pair-help")
    {
        printHelp();
        return;
    }

    if (command.startsWith("pair "))
    {
        String code =
            command.substring(5);

        code.trim();

        pairWithCode(code);
    }
}

void PeerBindingHostService::printBinding() const
{
    Serial.println();

    if (!_hasBinding)
    {
        Serial.println(
            "Bound peer: none"
        );
        return;
    }

    Serial.println("Bound peer:");

    Serial.print("  peer_id: ");
    Serial.println(_peerId);

    Serial.print("  peer_fingerprint: ");
    Serial.println(_peerFingerprint);

    Serial.print("  binding_id: ");
    Serial.println(_bindingId);

    Serial.print("  capabilities: ");
    Serial.println(_capabilities);
}

void PeerBindingHostService::printHelp() const
{
    Serial.println();
    Serial.println(
        "Peer-binding lab:"
    );

    Serial.println(
        "  HMI: Settings > Pair Peer"
    );

    Serial.println(
        "  peers          show remembered binding"
    );

    Serial.println(
        "  unpair         remove local binding"
    );

    Serial.println(
        "  pair NNNNNN    Serial fallback"
    );
}

void PeerBindingHostService::renderScanning()
{
    renderLines(
        "PAIR PEER",
        "SCANNING...",
        "Arm peer first"
    );
}

void PeerBindingHostService::renderConfirm()
{
    renderLines(
        "PAIR PEER",
        "VERIFY CODE",
        _candidateCode.c_str(),
        "Press=Bind Rot=Cancel"
    );
}

void PeerBindingHostService::renderSuccess()
{
    renderLines(
        "BOUND",
        _peerId.c_str(),
        _bindingId.c_str(),
        "Press=Done"
    );
}

void PeerBindingHostService::renderFailure(
    const char* line2
)
{
    renderLines(
        "PAIR FAILED",
        line2,
        "Returns to Settings"
    );
}

void PeerBindingHostService::renderLines(
    const char* title,
    const char* line2,
    const char* line3,
    const char* line4
)
{
    _display.clear();
    _display.setTextSize(1);

    _display.print(
        0,
        0,
        title
    );

    if (line2 != nullptr)
    {
        _display.print(
            0,
            16,
            line2
        );
    }

    if (line3 != nullptr)
    {
        _display.print(
            0,
            32,
            line3
        );
    }

    if (line4 != nullptr)
    {
        _display.print(
            0,
            48,
            line4
        );
    }

    _display.update();
}

void PeerBindingHostService::ensureHostIdentity()
{
    _hostId =
        _prefs.getString(
            "host_id",
            ""
        );

    _hostFingerprint =
        _prefs.getString(
            "host_fp",
            ""
        );

    if (_hostId.length() == 0)
    {
        _hostId =
            createThreeWordId();

        _prefs.putString(
            "host_id",
            _hostId
        );
    }

    if (_hostFingerprint.length() == 0)
    {
        _hostFingerprint =
            createFingerprint();

        _prefs.putString(
            "host_fp",
            _hostFingerprint
        );
    }
}

void PeerBindingHostService::loadBinding()
{
    _hasBinding =
        _prefs.getBool(
            "has_peer",
            false
        );

    if (!_hasBinding)
    {
        return;
    }

    _peerId =
        _prefs.getString(
            "peer_id",
            ""
        );

    _peerFingerprint =
        _prefs.getString(
            "peer_fp",
            ""
        );

    _bindingId =
        _prefs.getString(
            "binding_id",
            ""
        );

    _capabilities =
        _prefs.getString(
            "caps",
            ""
        );
}

void PeerBindingHostService::saveBinding()
{
    _prefs.putBool(
        "has_peer",
        true
    );

    _prefs.putString(
        "peer_id",
        _peerId
    );

    _prefs.putString(
        "peer_fp",
        _peerFingerprint
    );

    _prefs.putString(
        "binding_id",
        _bindingId
    );

    _prefs.putString(
        "caps",
        _capabilities
    );

    _changedPeerId = _peerId;
    _changedPeerFingerprint = _peerFingerprint;
    _changedBindingId = _bindingId;
    _changedCapabilities = _capabilities;
    _lastBindingChangeRemoved = false;
    _bindingChanged = true;
}

void PeerBindingHostService::clearBinding()
{
    _changedPeerId = _peerId;
    _changedPeerFingerprint = _peerFingerprint;
    _changedBindingId = _bindingId;
    _changedCapabilities = _capabilities;
    _lastBindingChangeRemoved = true;
    _bindingChanged = _hasBinding;

    _prefs.remove("peer_id");
    _prefs.remove("peer_fp");
    _prefs.remove("binding_id");
    _prefs.remove("caps");

    _prefs.putBool(
        "has_peer",
        false
    );

    _peerId = "";
    _peerFingerprint = "";
    _bindingId = "";
    _capabilities = "";
    _hasBinding = false;
}

bool PeerBindingHostService::hasBinding() const
{
    return _hasBinding;
}

const String& PeerBindingHostService::hostId() const
{
    return _hostId;
}

const String& PeerBindingHostService::peerId() const
{
    return _peerId;
}

const String& PeerBindingHostService::peerFingerprint() const
{
    return _peerFingerprint;
}

const String& PeerBindingHostService::bindingId() const
{
    return _bindingId;
}

const String& PeerBindingHostService::capabilities() const
{
    return _capabilities;
}

bool PeerBindingHostService::consumeBindingChanged()
{
    const bool changed = _bindingChanged;
    _bindingChanged = false;
    return changed;
}

bool PeerBindingHostService::lastBindingChangeRemoved() const
{
    return _lastBindingChangeRemoved;
}

const String& PeerBindingHostService::changedPeerId() const
{
    return _changedPeerId;
}

const String& PeerBindingHostService::changedPeerFingerprint() const
{
    return _changedPeerFingerprint;
}

const String& PeerBindingHostService::changedBindingId() const
{
    return _changedBindingId;
}

const String& PeerBindingHostService::changedCapabilities() const
{
    return _changedCapabilities;
}

void PeerBindingHostService::unbindLocal()
{
    clearBinding();
    Serial.println(
        "Binding authorization removed locally."
    );
}

String PeerBindingHostService::createThreeWordId() const
{
    String result;

    result +=
        WORD_A[
            esp_random() %
            countOf(WORD_A)
        ];

    result += "-";

    result +=
        WORD_B[
            esp_random() %
            countOf(WORD_B)
        ];

    result += "-";

    result +=
        WORD_C[
            esp_random() %
            countOf(WORD_C)
        ];

    return result;
}

String PeerBindingHostService::createFingerprint() const
{
    char buffer[33];

    snprintf(
        buffer,
        sizeof(buffer),
        "%08lx%08lx%08lx%08lx",
        static_cast<unsigned long>(
            esp_random()
        ),
        static_cast<unsigned long>(
            esp_random()
        ),
        static_cast<unsigned long>(
            esp_random()
        ),
        static_cast<unsigned long>(
            esp_random()
        )
    );

    return String(buffer);
}
