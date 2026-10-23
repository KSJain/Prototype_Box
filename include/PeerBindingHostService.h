#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <InputEvent.h>
#include <KSJ_SSD1306Display.h>

class PeerBindingHostService
{
public:
    explicit PeerBindingHostService(
        KSJ::SSD1306Display& display
    );

    bool begin();

    // Serial remains only as a lab fallback.
    void updateSerial();

    // Called continuously from loop().
    void update(
        uint32_t nowMs
    );

    // Invoked from Settings > Pair Peer.
    void startPairing(
        uint32_t nowMs
    );

    // While the pairing overlay owns the OLED, encoder events come here.
    void handleInput(
        KSJ::InputEvent event,
        uint32_t nowMs
    );

    bool ownsDisplay() const;

    // Lets main restore the Settings screen after overlay completion.
    bool consumeDisplayReleased();

    bool hasBinding() const;
    const String& hostId() const;
    const String& peerId() const;
    const String& peerFingerprint() const;
    const String& bindingId() const;
    const String& capabilities() const;

    bool consumeBindingChanged();
    bool lastBindingChangeRemoved() const;
    const String& changedPeerId() const;
    const String& changedPeerFingerprint() const;
    const String& changedBindingId() const;
    const String& changedCapabilities() const;

    void unbindLocal();

private:
    enum class HMIState : uint8_t
    {
        Idle = 0,
        Scanning,
        Confirm,
        Working,
        Success,
        Failure
    };

    KSJ::SSD1306Display& _display;
    Preferences _prefs;

    String _hostId;
    String _hostFingerprint;

    String _peerId;
    String _peerFingerprint;
    String _bindingId;
    String _capabilities;

    bool _hasBinding = false;

    String _serialBuffer;

    HMIState _hmiState = HMIState::Idle;
    String _candidateCode;
    String _candidateAddress;

    uint32_t _screenSinceMs = 0;
    bool _displayReleased = false;

    bool _bindingChanged = false;
    bool _lastBindingChangeRemoved = false;
    String _changedPeerId;
    String _changedPeerFingerprint;
    String _changedBindingId;
    String _changedCapabilities;

    void ensureHostIdentity();
    void loadBinding();
    void saveBinding();
    void clearBinding();

    void handleCommand(
        const String& command
    );

    bool findArmedPeer();

    void pairWithCandidate();

    void pairWithCode(
        const String& code,
        const String& address = ""
    );

    void printBinding() const;
    void printHelp() const;

    void releaseDisplay();

    void renderScanning();
    void renderConfirm();
    void renderSuccess();

    void renderFailure(
        const char* line2
    );

    void renderLines(
        const char* title,
        const char* line2,
        const char* line3 = nullptr,
        const char* line4 = nullptr
    );

    String createThreeWordId() const;
    String createFingerprint() const;
};
