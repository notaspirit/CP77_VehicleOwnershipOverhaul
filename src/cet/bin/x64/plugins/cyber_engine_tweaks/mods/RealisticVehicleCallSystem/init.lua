local isOverlayVisible = false

---@type VehicleOwnershipOverhaul
local voo = {}

registerForEvent('onInit', function()
    voo = require("services/VehicleOwnershipOverhaul").new()
    voo:LoadGarages()
end)

registerForEvent('onOverlayOpen', function()
    isOverlayVisible = true
end)

registerForEvent('onOverlayClose', function()
    isOverlayVisible = false
end)

registerForEvent('onDraw', function()
    if not isOverlayVisible then return end

    ImGui.Begin("Realistic Vehicle Call System")

    if (ImGui.Button("Load Garages")) then
        voo:LoadGarages()
    end

    ImGui.End()
end)