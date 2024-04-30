#include "UI/CustomEditableBeatmapSelectionView.hpp"
#include "Beatmaps/Packets/MpBeatmapPacket.hpp"
#include "Beatmaps/Providers/MpBeatmapLevelProvider.hpp"
#include "GlobalNamespace/LevelBar.hpp"
#include "GlobalNamespace/EnvironmentName.hpp"
#include "GlobalNamespace/BeatmapBasicData.hpp"
#include "UI/MpRequirementsUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "Utilities.hpp"
#include "logging.hpp"

DEFINE_TYPE(MultiplayerCore::UI, CustomEditableBeatmapSelectionView);

namespace MultiplayerCore::UI {

    void CustomEditableBeatmapSelectionView::Inject(Objects::MpPlayersDataModel* mpPlayersDataModel, Beatmaps::Providers::MpBeatmapLevelProvider* mpBeatmapLevelProvider) {
        _mpPlayersDataModel = mpPlayersDataModel;
        _mpBeatmapLevelProvider = mpBeatmapLevelProvider;
    }

    void CustomEditableBeatmapSelectionView::SetBeatmap(ByRef<GlobalNamespace::BeatmapKey> beatmapKey) {
        static auto method = il2cpp_utils::il2cpp_type_check::MetadataGetter<&GlobalNamespace::EditableBeatmapSelectionView::SetBeatmap>::methodInfo();
        static auto base = [](GlobalNamespace::EditableBeatmapSelectionView* self, ByRef<GlobalNamespace::BeatmapKey> beatmapKey){ return il2cpp_utils::RunMethodRethrow<void, false>(self, method, beatmapKey); };
        if (!beatmapKey->IsValid()) {
            DEBUG("beatmapkey was invalid");
            return base(this, beatmapKey);
        }

        auto levelHash = HashFromLevelID(beatmapKey->levelId);
        if (levelHash.empty()) { // level wasn't custom
            DEBUG("Level {} wasn't custom", beatmapKey->levelId);
            return base(this, beatmapKey);
        }

        auto packet = _mpPlayersDataModel->FindLevelPacket(levelHash);
        if (!packet) { // level was probably selected by the local player from their installed maps, since no other player had it selected
            DEBUG("Could not find packet for levelhash {}", levelHash);
            return base(this, beatmapKey);
        }

        // we have a packet to select from, update values correctly without calling base, and setup the levelbar with our own level
        _clearButton->gameObject->SetActive(showClearButton);
        _noLevelText->enabled = false;
        _levelBar->hide = false;

        // create a beatmap level for display
        auto level = _mpBeatmapLevelProvider->GetBeatmapFromPacket(packet);
        auto dict = System::Collections::Generic::Dictionary_2<System::ValueTuple_2<UnityW<GlobalNamespace::BeatmapCharacteristicSO>, GlobalNamespace::BeatmapDifficulty>, GlobalNamespace::BeatmapBasicData*>::New_ctor();
        dict->Add(
            System::ValueTuple_2<UnityW<GlobalNamespace::BeatmapCharacteristicSO>, GlobalNamespace::BeatmapDifficulty>(
                beatmapKey->beatmapCharacteristic,
                beatmapKey->difficulty
            ),
            GlobalNamespace::BeatmapBasicData::New_ctor(
                0, 0, GlobalNamespace::EnvironmentName::getStaticF_Empty(),
                nullptr, 0, 0, 0,
                level->allMappers, level->allLighters
            )
        );

        _levelBar->Setup(level, beatmapKey->beatmapCharacteristic, beatmapKey->difficulty);
    }
}