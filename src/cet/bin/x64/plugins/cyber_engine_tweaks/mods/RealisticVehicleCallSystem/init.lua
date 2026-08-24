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

    ImGui.SameLine()

    if (ImGui.Button("Copy Position As Slot")) then
        local serializedSlot = {
            VehicleTypes = {
                "Car",
                "Bike"
            },
            WhiteListedVehicles = {},
            BlackListedVehicles = {
                "Vehicle.cs_savable_mahir_mt28_coach_dab",
                "Vehicle.oranje3_table_top_oranje",
                "Vehicle.oranje3_table_top_oranje_container",
                "Vehicle.oranje3_table_top_oranje_container_frame",
                "Vehicle.oranje3_table_top_oranje_hauler",
                "Vehicle.oranje3_table_top_oranje_hauler_empty",
                "Vehicle.oranje3_table_top_oranje_mixer",
                "Vehicle.oranje3_table_top_oranje_tanker"
            },
            Position = {
                X = 402.0,
                Y = -915.0,
                Z = 24.5
            },
            Rotation = {
                I = 0.0,
                J = 0.0,
                K = 0.6085609,
                R = -0.7935073
            }
        }

        local ppos = GetPlayer():GetWorldPosition()
        local prot = GetPlayer():GetWorldOrientation()

        serializedSlot.Position.X = ppos.x
        serializedSlot.Position.Y = ppos.y
        serializedSlot.Position.Z = ppos.z

        serializedSlot.Rotation.I = prot.i
        serializedSlot.Rotation.J = prot.j
        serializedSlot.Rotation.K = prot.k
        serializedSlot.Rotation.R = prot.r

        ImGui.SetClipboardText(require("libs/json").stringify(serializedSlot, true))
    end

    ImGui.End()
end)