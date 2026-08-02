#include <RealisticVehicleCallSystem.h>
#include "RED4ext/Scripting/Utils.hpp"
#include "RED4ext/Scripting/Natives/Generated/physics/GeometryCache.hpp"

#include "RED4ext/Scripting/Natives/Generated/world/StreamingWorld.hpp"

void RealisticVehicleCallSystem::RealisticVehicleCallSystem::OnGeoCachePostLoad(
    RED4ext::IScriptable *aContext,
    RED4ext::CStackFrame *aFrame,
    RED4ext::CString *aOut,
    int64_t a4)
{
    RED4ext::Handle<RED4ext::physicsGeometryCache> geoCache;
    RED4ext::GetParameter(aFrame, &geoCache);
    aFrame->code++;

    for (auto& sector: geoCache->sectorEntries)
    {
        sector.sectorBounds.Min.W = 0;
        sector.sectorBounds.Min.X = -16384;
        sector.sectorBounds.Min.Y = -16384;
        sector.sectorBounds.Min.Z = -16384;

        sector.sectorBounds.Max.W = 0;
        sector.sectorBounds.Max.X = 16384;
        sector.sectorBounds.Max.Y = 16384;
        sector.sectorBounds.Max.Z = 16384;
    }
}
