#include "UI/MpLoadingIndicator.hpp"
#include "logging.hpp"
#include "assets.hpp"

#include "bsml/shared/BSML.hpp"

#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/IReadOnlyDictionary_2.hpp"
#include "GlobalNamespace/PreviewDifficultyBeatmap.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/ILobbyPlayerData.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"

DEFINE_TYPE(MultiplayerCore::UI, MpLoadingIndicator);

template<typename T, typename U>
int IReadOnlyDictionary_2_Count(System::Collections::Generic::IReadOnlyDictionary_2<T, U>* dict) {
    auto coll = dict->i_KeyValuePair_2_TKey_TValue();
    return coll->get_Count();
}

namespace MultiplayerCore::UI {
    void MpLoadingIndicator::ctor(
        GlobalNamespace::IMultiplayerSessionManager* sessionManager,
        GlobalNamespace::ILobbyGameStateController* gameStateController,
        GlobalNamespace::ILobbyPlayersDataModel* playersDataModel,
        GlobalNamespace::NetworkPlayerEntitlementChecker* entitlementChecker,
        Objects::MpLevelLoader* levelLoader,
        GlobalNamespace::CenterStageScreenController* screenController
    ) {
        _sessionManager = sessionManager;
        _gameStateController = gameStateController;
        _playersDataModel = playersDataModel;
        _entitlementChecker = il2cpp_utils::try_cast<Objects::MpEntitlementChecker>(entitlementChecker).value_or(nullptr);
        _levelLoader = levelLoader;
        _screenController = screenController;
    }

    void MpLoadingIndicator::Dispose() {
        _levelLoader->progressUpdated -= {&MpLoadingIndicator::Report, this};
    }

    void MpLoadingIndicator::Initialize() {
        BSML::parse_and_construct(IncludedAssets::LoadingIndicator_bsml, _screenController->get_transform(), this);
        auto existingLoadingControl = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::LoadingControl*>().First()->get_gameObject();
        auto go = UnityEngine::Object::Instantiate(existingLoadingControl, vert->get_transform());
        _loadingControl = go->GetComponent<GlobalNamespace::LoadingControl*>();
        _loadingControl->Hide();

        _levelLoader->progressUpdated += {&MpLoadingIndicator::Report, this};
    }

    void MpLoadingIndicator::Tick() {
        if (_isDownloading) {
            return;
        } else if (
            _screenController->get_countdownShown() &&
            _sessionManager->get_syncTime() >= _gameStateController->get_startTime() &&
            _gameStateController->get_levelStartInitiated() &&
            _levelLoader->gameplaySetupData != nullptr
        ) {
            int okCount = OkPlayerCountNoRequest();
            int totalCount = IReadOnlyDictionary_2_Count(_playersDataModel->i_ILobbyPlayerData());
            _loadingControl->ShowLoading(fmt::format("{} of {} players ready...", okCount, totalCount));
        } else {
            _loadingControl->Hide();
        }
    }

    int MpLoadingIndicator::OkPlayerCountNoRequest() {
        // TODO: check if this entire thing is fine with all these interface castings
        auto dict = _playersDataModel->i_ILobbyPlayerData();
        auto coll = dict->i_KeyValuePair_2_TKey_TValue();

        int count = coll->get_Count();
        auto enumerator_1 = coll->i_IEnumerable_1_T()->GetEnumerator();
        auto enumerator = enumerator_1->i_IEnumerator();

        auto levelId = _levelLoader->gameplaySetupData->get_beatmapLevel()->get_beatmapLevel()->get_levelID();

        int okCount = 0;
        while (enumerator->MoveNext()) {
            auto cur = enumerator_1->get_Current();
            auto tocheck = cur.key;
            if (_entitlementChecker->GetUserEntitlementStatusWithoutRequest(tocheck, levelId) == GlobalNamespace::EntitlementsStatus::Ok) okCount++;
        }
        enumerator_1->i_IDisposable()->Dispose();
        return okCount;
    }

    void MpLoadingIndicator::Report(double value) {
        _isDownloading = value < 1.0;
        _loadingControl->ShowDownloadingProgress(fmt::format("Downloading ({:.2f}%)...", (float)value), value);
    }
}