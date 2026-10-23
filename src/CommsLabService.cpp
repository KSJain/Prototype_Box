#include "CommsLabService.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

namespace CommsLabConfig
{
    constexpr char NODE_ID[] = "prototype-box";
    constexpr char TARGET_ID[] = "lab-node-01";

    constexpr uint32_t RESPONSE_TIMEOUT_MS = 2000;
    constexpr uint32_t RELAY_PULSE_MS = 250;

    const uint8_t BROADCAST_MAC[6] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };
}

CommsLabService* CommsLabService::_instance = nullptr;

bool CommsLabService::begin(
    uint8_t offlineFallbackChannel
)
{
    _ready = false;
    _peerSeen = false;
    _awaitingResponse = false;
    _lastResponseOk = false;
    _remoteRelayClicksEnabled = false;
    _changed = false;

    // Wi-Fi STA mode is shared with NetworkService. Do not disconnect here.
    WiFi.mode(WIFI_STA);

    strlcpy(
        _localMac,
        WiFi.macAddress().c_str(),
        sizeof(_localMac)
    );

    // If Wi-Fi is not associated yet, give ESP-NOW a deterministic offline
    // channel. Once STA associates, the ESP32 radio follows the AP channel.
    if (
        WiFi.status() != WL_CONNECTED &&
        offlineFallbackChannel > 0
    )
    {
        if (
            esp_wifi_set_channel(
                offlineFallbackChannel,
                WIFI_SECOND_CHAN_NONE
            ) != ESP_OK
        )
        {
            strlcpy(_lastState, "RADIO FAIL", sizeof(_lastState));
            strlcpy(_lastReason, "channel", sizeof(_lastReason));
            return false;
        }
    }

    if (esp_now_init() != ESP_OK)
    {
        strlcpy(_lastState, "ESPNOW FAIL", sizeof(_lastState));
        strlcpy(_lastReason, "init", sizeof(_lastReason));
        return false;
    }

    esp_now_peer_info_t peer{};
    memcpy(
        peer.peer_addr,
        CommsLabConfig::BROADCAST_MAC,
        sizeof(CommsLabConfig::BROADCAST_MAC)
    );
    // 0 means use the current Wi-Fi radio channel.
    peer.channel = 0;
    peer.encrypt = false;

    if (
        !esp_now_is_peer_exist(
            CommsLabConfig::BROADCAST_MAC
        ) &&
        esp_now_add_peer(&peer) != ESP_OK
    )
    {
        strlcpy(_lastState, "PEER FAIL", sizeof(_lastState));
        strlcpy(_lastReason, "broadcast", sizeof(_lastReason));
        return false;
    }

    _instance = this;
    esp_now_register_recv_cb(onDataRecv);

    _ready = true;
    strlcpy(_lastState, "READY", sizeof(_lastState));
    strlcpy(_lastReason, "NONE", sizeof(_lastReason));
    setChanged();

    Serial.print("Comms Lab MAC: ");
    Serial.println(_localMac);

    Serial.print("Comms Lab channel: ");
    Serial.println(channel());

    return true;
}

void CommsLabService::update(
    uint32_t nowMs
)
{
    processIncoming(nowMs);

    if (
        _awaitingResponse &&
        nowMs - _requestSentAtMs >=
            CommsLabConfig::RESPONSE_TIMEOUT_MS
    )
    {
        _awaitingResponse = false;
        _lastResponseOk = false;
        strlcpy(_lastState, "TIMEOUT", sizeof(_lastState));
        strlcpy(_lastReason, "no_reply", sizeof(_lastReason));
        setChanged();
    }
}

bool CommsLabService::request(
    CommsLabAction action,
    uint32_t nowMs
)
{
    if (!_ready || action == CommsLabAction::Back)
    {
        return false;
    }

    return sendRequest(action, nowMs);
}

bool CommsLabService::sendRequest(
    CommsLabAction action,
    uint32_t nowMs
)
{
    JsonDocument doc;

    const uint32_t requestId =
        _nextRequestId++;

    doc["v"] = 1;
    doc["kind"] = "request";
    doc["id"] = requestId;
    doc["from"] = CommsLabConfig::NODE_ID;
    doc["to"] = CommsLabConfig::TARGET_ID;
    doc["impl"] = "physical";
    doc["sim"] = false;

    const char* operation = "node.status";

    switch (action)
    {
        case CommsLabAction::Status:
            operation = "node.status";
            break;

        case CommsLabAction::LedOff:
            operation = "lab.led.pattern";
            doc["pattern"] = "off";
            break;

        case CommsLabAction::LedSlow:
            operation = "lab.led.pattern";
            doc["pattern"] = "slow";
            break;

        case CommsLabAction::LedFast:
            operation = "lab.led.pattern";
            doc["pattern"] = "fast";
            break;

        case CommsLabAction::LedTriple:
            operation = "lab.led.pattern";
            doc["pattern"] = "triple";
            break;

        case CommsLabAction::Relay1:
        case CommsLabAction::Relay2:
        case CommsLabAction::Relay3:
        case CommsLabAction::Relay4:
        {
            operation = "lab.relay.pulse";

            const uint8_t channel =
                static_cast<uint8_t>(action) -
                static_cast<uint8_t>(CommsLabAction::Relay1) +
                1;

            doc["channel"] = channel;
            doc["pulse_ms"] =
                CommsLabConfig::RELAY_PULSE_MS;
            break;
        }

        case CommsLabAction::StubGardenDry:
            operation = "lab.stub.observation";
            doc["as"] = "garden-stub-01";
            doc["impl"] = "stub";
            doc["sim"] = true;
            doc["cap"] = "environment.observe";
            doc["rh"] = 37.0;
            doc["temp_c"] = 25.2;
            doc["light"] = 420;
            break;

        case CommsLabAction::StubGardenHumid:
            operation = "lab.stub.observation";
            doc["as"] = "garden-stub-01";
            doc["impl"] = "stub";
            doc["sim"] = true;
            doc["cap"] = "environment.observe";
            doc["rh"] = 64.0;
            doc["temp_c"] = 24.0;
            doc["light"] = 420;
            break;

        case CommsLabAction::StubGardenDark:
            operation = "lab.stub.observation";
            doc["as"] = "garden-stub-01";
            doc["impl"] = "stub";
            doc["sim"] = true;
            doc["cap"] = "environment.observe";
            doc["rh"] = 55.0;
            doc["temp_c"] = 22.0;
            doc["light"] = 5;
            break;

        case CommsLabAction::Back:
        case CommsLabAction::Count:
        default:
            return false;
    }

    doc["op"] = operation;

    char payload[241];
    const size_t length =
        serializeJson(
            doc,
            payload,
            sizeof(payload)
        );

    if (length == 0 || length >= sizeof(payload))
    {
        strlcpy(_lastState, "TX FAIL", sizeof(_lastState));
        strlcpy(_lastReason, "payload", sizeof(_lastReason));
        setChanged();
        return false;
    }

    const esp_err_t sendResult =
        esp_now_send(
            CommsLabConfig::BROADCAST_MAC,
            reinterpret_cast<const uint8_t*>(payload),
            length
        );

    _lastRequestId = requestId;
    _requestSentAtMs = nowMs;
    strlcpy(_lastOperation, operation, sizeof(_lastOperation));

    if (sendResult != ESP_OK)
    {
        _awaitingResponse = false;
        _lastResponseOk = false;
        strlcpy(_lastState, "TX FAIL", sizeof(_lastState));
        strlcpy(_lastReason, "esp_now_send", sizeof(_lastReason));
        setChanged();
        return false;
    }

    _awaitingResponse = true;
    _lastResponseOk = false;
    strlcpy(_lastState, "WAIT", sizeof(_lastState));
    strlcpy(_lastReason, "sent", sizeof(_lastReason));
    setChanged();

    Serial.print("COMMS_TX ");
    Serial.println(payload);

    return true;
}

void CommsLabService::processIncoming(
    uint32_t nowMs
)
{
    PendingRx incoming;

    if (!takePending(incoming))
    {
        return;
    }

    JsonDocument response;
    const DeserializationError error =
        deserializeJson(
            response,
            incoming.data,
            incoming.length
        );

    if (error)
    {
        strlcpy(_lastState, "RX INVALID", sizeof(_lastState));
        strlcpy(_lastReason, "json", sizeof(_lastReason));
        setChanged();
        return;
    }

    const char* kind = response["kind"] | "";
    const char* from = response["from"] | "";
    const char* to = response["to"] | "";

    if (strcmp(kind, "response") != 0)
    {
        return;
    }

    if (
        strcmp(from, CommsLabConfig::TARGET_ID) != 0 ||
        strcmp(to, CommsLabConfig::NODE_ID) != 0
    )
    {
        return;
    }

    formatMac(
        incoming.sourceMac,
        _peerMac,
        sizeof(_peerMac)
    );

    _peerSeen = true;
    _awaitingResponse = false;

    _lastRequestId =
        response["id"] | _lastRequestId;

    _lastResponseOk =
        response["ok"] | false;

    _remoteRelayClicksEnabled =
        response["relay_clicks"] | false;

    const char* state = response["state"] | "UNKNOWN";
    const char* reason = response["reason"] | "NONE";

    strlcpy(_lastState, state, sizeof(_lastState));
    strlcpy(_lastReason, reason, sizeof(_lastReason));

    setChanged();

    Serial.print("COMMS_RX ");
    Serial.write(incoming.data, incoming.length);
    Serial.println();

    (void)nowMs;
}

void CommsLabService::enqueueReceived(
    const uint8_t* sourceMac,
    const uint8_t* data,
    int length
)
{
    if (sourceMac == nullptr || data == nullptr || length <= 0)
    {
        return;
    }

    const size_t copyLength =
        static_cast<size_t>(length) > sizeof(_pendingRx.data) - 1
            ? sizeof(_pendingRx.data) - 1
            : static_cast<size_t>(length);

    portENTER_CRITICAL(&_rxMux);

    if (!_pendingRx.pending)
    {
        memcpy(_pendingRx.sourceMac, sourceMac, 6);
        memcpy(_pendingRx.data, data, copyLength);
        _pendingRx.data[copyLength] = 0;
        _pendingRx.length = copyLength;
        _pendingRx.pending = true;
    }

    portEXIT_CRITICAL(&_rxMux);
}

bool CommsLabService::takePending(
    PendingRx& output
)
{
    bool available = false;

    portENTER_CRITICAL(&_rxMux);

    if (_pendingRx.pending)
    {
        output = _pendingRx;
        _pendingRx.pending = false;
        available = true;
    }

    portEXIT_CRITICAL(&_rxMux);

    return available;
}

bool CommsLabService::consumeChanged()
{
    const bool changed = _changed;
    _changed = false;
    return changed;
}

bool CommsLabService::isReady() const
{
    return _ready;
}

bool CommsLabService::peerSeen() const
{
    return _peerSeen;
}

bool CommsLabService::awaitingResponse() const
{
    return _awaitingResponse;
}

bool CommsLabService::lastResponseOk() const
{
    return _lastResponseOk;
}

bool CommsLabService::remoteRelayClicksEnabled() const
{
    return _remoteRelayClicksEnabled;
}

uint8_t CommsLabService::channel() const
{
    uint8_t primary = 0;
    wifi_second_chan_t secondary =
        WIFI_SECOND_CHAN_NONE;

    if (
        esp_wifi_get_channel(
            &primary,
            &secondary
        ) != ESP_OK
    )
    {
        return 0;
    }

    return primary;
}

uint32_t CommsLabService::lastRequestId() const
{
    return _lastRequestId;
}

const char* CommsLabService::localMac() const
{
    return _localMac;
}

const char* CommsLabService::peerMac() const
{
    return _peerMac;
}

const char* CommsLabService::lastOperation() const
{
    return _lastOperation;
}

const char* CommsLabService::lastState() const
{
    return _lastState;
}

const char* CommsLabService::lastReason() const
{
    return _lastReason;
}

String CommsLabService::stateJson() const
{
    JsonDocument doc;
    doc["ready"] = _ready;
    doc["peer_seen"] = _peerSeen;
    doc["awaiting"] = _awaitingResponse;
    doc["request_id"] = _lastRequestId;
    doc["op"] = _lastOperation;
    doc["state"] = _lastState;
    doc["reason"] = _lastReason;
    doc["ok"] = _lastResponseOk;
    doc["remote_relay_clicks"] = _remoteRelayClicksEnabled;
    doc["implementation"] = "physical";
    doc["simulated"] = false;

    String output;
    serializeJson(doc, output);
    return output;
}

void CommsLabService::setChanged()
{
    _changed = true;
}

void CommsLabService::formatMac(
    const uint8_t* mac,
    char* output,
    size_t outputSize
)
{
    snprintf(
        output,
        outputSize,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void CommsLabService::onDataRecv(
    const esp_now_recv_info_t* info,
    const uint8_t* data,
    int length
)
{
    if (_instance == nullptr || info == nullptr)
    {
        return;
    }

    _instance->enqueueReceived(
        info->src_addr,
        data,
        length
    );
}
#else
void CommsLabService::onDataRecv(
    const uint8_t* mac,
    const uint8_t* data,
    int length
)
{
    if (_instance == nullptr)
    {
        return;
    }

    _instance->enqueueReceived(
        mac,
        data,
        length
    );
}
#endif
