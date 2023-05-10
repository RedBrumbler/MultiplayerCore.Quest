#include "Hooks/NetworkConfigHooks.hpp"
#include "logging.hpp"
#include "hooking.hpp"

#include "GlobalNamespace/NetworkConfigSO.hpp"
#include "GlobalNamespace/MainSystemInit.hpp"
#include "GlobalNamespace/ClientCertificateValidator.hpp"
#include "GlobalNamespace/UnifiedNetworkPlayerModel.hpp"
#include "GlobalNamespace/UnifiedNetworkPlayerModel_ActiveNetworkPlayerModelType.hpp"
#include "System/Security/Cryptography/X509Certificates/X509Certificate2.hpp"

#define LOG_VALUE(identifier, value) DEBUG("Overriding NetworkConfigSO::" identifier " to '{}'", cfg->value)

// File is equivalent to MultiplayerCore.Patchers.NetworkConfigPatcher from PC

namespace MultiplayerCore::Hooks {
    ServerConfig NetworkConfigHooks::officialServerConfig{};
    const ServerConfig* NetworkConfigHooks::currentServerConfig{};
    GlobalNamespace::NetworkConfigSO* NetworkConfigHooks::networkConfig = nullptr;
    UnorderedEventCallback<const ServerConfig*> NetworkConfigHooks::ServerChanged{};

    const ServerConfig* NetworkConfigHooks::GetCurrentServer() { return currentServerConfig; }
    const ServerConfig* NetworkConfigHooks::GetOfficialServer() { return &officialServerConfig; }

    bool NetworkConfigHooks::IsOverridingAPI() { return GetCurrentServer() && GetCurrentServer() != GetOfficialServer(); }

    void NetworkConfigHooks::UseServer(const ServerConfig* cfg) {
        if (!cfg) {
            UseOfficialServer();
            return;
        }

        currentServerConfig = cfg;

        if (networkConfig) {
            networkConfig->graphUrl = cfg->graphUrl;
            LOG_VALUE("graphUrl", graphUrl);
            networkConfig->multiplayerStatusUrl = cfg->masterServerStatusUrl;
            LOG_VALUE("multiplayerStatusUrl", masterServerStatusUrl);
            networkConfig->quickPlaySetupUrl = cfg->quickPlaySetupUrl;
            LOG_VALUE("quickPlaySetupUrl", quickPlaySetupUrl);

            // only 128 ids can exist, 0 & 127 are taken as server and broadcast
            // thus 128 - 2 = 126 absolute max player count
            networkConfig->maxPartySize = std::clamp(cfg->maxPartySize, 0, 126);
            LOG_VALUE("maxPartySize", maxPartySize);
            networkConfig->discoveryPort = cfg->discoveryPort;
            LOG_VALUE("discoveryPort", discoveryPort);
            networkConfig->partyPort = cfg->partyPort;
            LOG_VALUE("partyPort", partyPort);
            networkConfig->multiplayerPort = cfg->multiplayerPort;
            LOG_VALUE("multiplayerPort", multiplayerPort);
            networkConfig->forceGameLift = cfg->forceGameLift;
            LOG_VALUE("forceGameLift", forceGameLift);
        }

        ServerChanged.invoke(cfg);
    }

    void NetworkConfigHooks::UseOfficialServer() {
        UseServer(&officialServerConfig);
    }
}

using namespace MultiplayerCore::Hooks;

MAKE_AUTO_HOOK_MATCH(MainSystemInit_Init, &::GlobalNamespace::MainSystemInit::Init, void, GlobalNamespace::MainSystemInit* self) {
    MainSystemInit_Init(self);
    // construct original config from base game values
    auto& officialConfig = NetworkConfigHooks::officialServerConfig;
    officialConfig.graphUrl  = static_cast<std::string>(self->networkConfig->graphUrl);
    officialConfig.masterServerStatusUrl  = static_cast<std::string>(self->networkConfig->multiplayerStatusUrl);
    officialConfig.quickPlaySetupUrl = static_cast<std::string>(self->networkConfig->quickPlaySetupUrl);

    officialConfig.maxPartySize = self->networkConfig->maxPartySize;
    officialConfig.discoveryPort = self->networkConfig->discoveryPort;
    officialConfig.partyPort = self->networkConfig->partyPort;
    officialConfig.multiplayerPort = self->networkConfig->multiplayerPort;
    officialConfig.forceGameLift = self->networkConfig->forceGameLift;

    NetworkConfigHooks::networkConfig = self->networkConfig;

    NetworkConfigHooks::UseServer(NetworkConfigHooks::currentServerConfig);
}

MAKE_AUTO_HOOK_MATCH(UnifiedNetworkPlayerModel_SetActiveNetworkPlayerModelType, &GlobalNamespace::UnifiedNetworkPlayerModel::SetActiveNetworkPlayerModelType, void, GlobalNamespace::UnifiedNetworkPlayerModel* self, GlobalNamespace::UnifiedNetworkPlayerModel_ActiveNetworkPlayerModelType activeNetworkPlayerModelType) {
    auto currentConfig = NetworkConfigHooks::GetCurrentServer();
    if (NetworkConfigHooks::IsOverridingAPI()) {
        DEBUG("Disabling MasterServer, Setting to GameLift");
        UnifiedNetworkPlayerModel_SetActiveNetworkPlayerModelType(self, GlobalNamespace::UnifiedNetworkPlayerModel_ActiveNetworkPlayerModelType::GameLift);
        return;
    }

    DEBUG("Using Default");
    UnifiedNetworkPlayerModel_SetActiveNetworkPlayerModelType(self, activeNetworkPlayerModelType);
}

// possibly does not call orig
MAKE_AUTO_HOOK_ORIG_MATCH(ClientCertificateValidator_ValidateCertificateChainInternal, &GlobalNamespace::ClientCertificateValidator::ValidateCertificateChainInternal, void, GlobalNamespace::ClientCertificateValidator* self, GlobalNamespace::DnsEndPoint* endPoint, System::Security::Cryptography::X509Certificates::X509Certificate2* certificate, ::ArrayW<::ArrayW<uint8_t>> certificateChain) {
    if (NetworkConfigHooks::IsOverridingAPI()) {
        DEBUG("Ignoring certificate validation");
        return;
    }

    DEBUG("No EndPoint set, using default");
    ClientCertificateValidator_ValidateCertificateChainInternal(self, endPoint, certificate, certificateChain);
}
