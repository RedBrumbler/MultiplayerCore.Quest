#include "ScorePoseSyncState/MpScoreSyncStatePacket.hpp"
#include "GlobalNamespace/VarIntExtensions.hpp"

DEFINE_TYPE(MultiplayerCore::ScoreSyncState, MpScoreSyncStatePacket);

namespace MultiplayerCore::ScoreSyncState {
    void MpScoreSyncStatePacket::ctor() {
        INVOKE_BASE_CTOR(classof(MultiplayerCore::Networking::Abstractions::MpPacket*));
    }

    void MpNodePoseSyncStatePacket::Serialize(LiteNetLib::Utils::NetDataWriter* writer) {
        writer->PutVarLong(deltaUpdateFrequencyMs);
        writer->PutVarLong(fullStateUpdateFrequencyMs);
    }

    void MpNodePoseSyncStatePacket::Deserialize(LiteNetLib::Utils::NetDataReader* reader) {
        deltaUpdateFrequencyMs = reader->GetVarLong();
        fullStateUpdateFrequencyMs = reader->GetVarLong();
    }
}
