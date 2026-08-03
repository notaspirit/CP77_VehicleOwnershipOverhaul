local isOverlayVisible = false

registerForEvent('onInit', function()
    ObserveAfter('RealisticVehicleCallSystemNative', 'ShowSimpleScreenMessage', function(message)
        GameInstance.GetBlackboardSystem():Get(GetAllBlackboardDefs().UI_Notifications):SetVariant(GetAllBlackboardDefs().UI_Notifications.WarningMessage, ToVariant(message), true)
    end)
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

    if (ImGui.Button("Set spawn position")) then
        local pos = GetPlayer():GetWorldPosition()
        local rot = GetPlayer():GetWorldOrientation()

        RealisticVehicleCallSystemNative.SetSpawnPoint(pos, rot)
    end

    ImGui.End()
end)