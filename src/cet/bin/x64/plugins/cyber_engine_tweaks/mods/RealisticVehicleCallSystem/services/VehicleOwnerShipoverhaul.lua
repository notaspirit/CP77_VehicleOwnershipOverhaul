local utils = require("tools/utils")
local logger = require("libs/logger")
local json = require("libs/json")

local garage = require("models/garage")
local gridConfig = require("models/grid_config")
local ReturnValue = require("models/return_value")

---@class VehicleOwnershipOverhaul
---@field public garages Garage[]
---@field public currentVehicleType number
---@field public currentVehicleID TweakDBID
---@field public currentVehicleSpawned boolean
---@field public currentDeliveredGarageName string
---@field public currentOccupyingVehicles table<vehicleCarBaseObject, true>
---@field public new fun(): VehicleOwnershipOverhaul
---@field public LoadGarages fun(self: VehicleOwnershipOverhaul): void
---@field private PreSpawnPlayerVehicleNativeHook fun(self: VehicleOwnershipOverhaul, vehicleType: gamedataVehicleType, vehicleId: TweakDBID, spawnOnlyOnValidRoad: boolean): void
---@field private PostSpawnPlayerVehicleNativeHook fun(self: VehicleOwnershipOverhaul, vehicleType: gamedataVehicleType, vehicleId: TweakDBID, spawnOnlyOnValidRoad: boolean): void
---@field private PreFindSpawnLocationNativeHook fun(self: VehicleOwnershipOverhaul, playerPosition: Vector3, outPosition: Vector3, playerAndOutRotation: Quaternion): WorldTransform
---@field private PreSummonVehicleNativeHook fun(self: VehicleOwnershipOverhaul): void
---@field private IsGarageBought fun(garage: Garage): boolean
---@field private GarageOrSlotSupports fun(garage: Garage | GarageSlot, vehicleType: number, vehicleId: TweakDBID): boolean
---@field private CheckSlotOccupancy fun(slot: GarageSlot, vehType: number): table<vehicleCarBaseObject, true> | nil
---@field private DespawnOccupyingVehicles fun(self: VehicleOwnershipOverhaul): void
local VehicleOwnershipOverhaul = {}

VehicleOwnershipOverhaul.__index = VehicleOwnershipOverhaul

function VehicleOwnershipOverhaul.new()
    local self = setmetatable({}, VehicleOwnershipOverhaul)

    Observe('RealisticVehicleCallSystemNative', 'hkPreSpawnPlayerVehicle', function(vehicleType, vehicleId, spawnOnlyOnValidRoad)
        self:PreSpawnPlayerVehicleNativeHook(vehicleType, vehicleId, spawnOnlyOnValidRoad)
    end)

    Observe('RealisticVehicleCallSystemNative', 'hkPostSpawnPlayerVehicle', function(vehicleType, vehicleId, spawnOnlyOnValidRoad)
        self:PostSpawnPlayerVehicleNativeHook(vehicleType, vehicleId, spawnOnlyOnValidRoad)
    end)

    Override('RealisticVehicleCallSystemNative', 'hkPreFindSpawnLocation', function(playerPosition, outPosition, playerAndOutRotation, originalMethod)
        return self:PreFindSpawnLocationNativeHook(playerPosition, outPosition, playerAndOutRotation)
    end)

    Observe('RealisticVehicleCallSystemNative', 'hkPreSummonVehicle', function()
        self:PreSummonVehicleNativeHook(playerPosition, outPosition, playerAndOutRotation)
    end)

    return self
end

function VehicleOwnershipOverhaul:LoadGarages()
    self.garages = {}

    local files = utils.ListFilesInDir("data/garages/", "json")
    local encounteredNames = {}

    for _, fileInfo in ipairs(files) do
        local file = io.open("data/garages/" .. fileInfo.name)
        if not file then
            print("failed to open file at " .. "data/garages/" .. fileInfo.name)
            goto continue
        end

        local jsonStr = file:read("*a")
        file:close()

        local jsonTable = json.parse(jsonStr)
        if not jsonTable then
            print("failed to deserialize json at " .. "data/garages/" .. fileInfo.name)
            goto continue
        end

        local g = garage.new()
        local rv = g:Deserialize(jsonTable, encounteredNames)
        if not rv.success then
            logger.error("Failed to deserialze garage at " .. fileInfo .. " with " .. rv:BuildLogMessage())
            goto continue
        end

        print("inserting " .. "data/garages/" .. fileInfo.name)

        table.insert(self.garages, g)

        ::continue::
    end
end


function VehicleOwnershipOverhaul:PreSpawnPlayerVehicleNativeHook(vehicleType, vehicleId, spawnOnlyOnValidRoad)
    self.currentVehicleSpawned = false
    self.currentVehicleType = tonumber(EnumInt(vehicleType))
    self.currentVehicleID = vehicleId
end

function VehicleOwnershipOverhaul:PostSpawnPlayerVehicleNativeHook(vehicleType, vehicleId, spawnOnlyOnValidRoad)
    local tdbid = TDBID.ToStringDEBUG(self.currentVehicleID)
    local name = Game.GetLocalizedTextByKey(TDB.GetLocKey(tdbid .. ".displayName"))
    
    if (self.currentVehicleSpawned) then
        -- works even though it is technically after the native method has completed
        self:DespawnOccupyingVehicles()

        Game.AddToInventory("Items.money", -1000)
        local als = Game.GetActivityLogSystem()
        als:AddLog("Vehicle Delivery")

        local msg = utils.BuildDefaultSimpleMessage()
        msg.message = "Delivered " .. name .. " to " .. self.currentDeliveredGarageName
        GameInstance.GetBlackboardSystem():Get(GetAllBlackboardDefs().UI_Notifications):SetVariant(GetAllBlackboardDefs().UI_Notifications.WarningMessage, ToVariant(msg), true)
    else
        local msg = utils.BuildDefaultSimpleMessage()
        msg.message = name .. " is nearby"
        GameInstance.GetBlackboardSystem():Get(GetAllBlackboardDefs().UI_Notifications):SetVariant(GetAllBlackboardDefs().UI_Notifications.WarningMessage, ToVariant(msg), true)
    end

    self.currentOccupyingVehicles = nil
    self.currentVehicleSpawned = nil
    self.currentVehicleType = nil
    self.currentVehicleId = nil
end

function VehicleOwnershipOverhaul:PreFindSpawnLocationNativeHook(playerPosition, outPosition, playerAndOutRotation)
    local playerPosition = GetPlayer():GetWorldPosition()
    
    local wt = WorldTransform.new()

    if self.garages == nil then
        print("garages is nil")
        return wt
    end

    if #self.garages == 0 then
        print("garages contains no entries")
        return wt
    end

    local closestGarage = nil
    local closestDistance = math.huge

    for _, garage in ipairs(self.garages) do
        print("checking garage " .. garage.name)
        if not self.IsGarageBought(garage) then
            print("garage is not bought")
            goto continue
        end

        if not self.GarageOrSlotSupports(garage, self.currentVehicleType, self.currentVehicleID) then
            print("garage doesn't support type or vehicle")
            goto continue
        end

        local dist = utils.Distance(playerPosition, garage.position)
        print("distance is " .. dist)
        if dist < closestDistance then
            print(garage.name .. "is now the closest garage")
            closestGarage = garage
            closestDistance = dist
        end

        ::continue::
    end

    if closestGarage == nil then
        return wt
    end

    ---@type table<GarageSlot>
    local matchingSlots = {}

    for _, slot in ipairs(closestGarage.slots) do
        if self.GarageOrSlotSupports(slot, self.currentVehicleType, self.currentVehicleID) then
            table.insert(matchingSlots, slot)
        end
    end

    if #matchingSlots == 0 then
        return wt
    end

    utils.ShuffleArray(matchingSlots)

    ---@type GarageSlot
    local randomSlot = nil

    ---@type table<entEntity>
    local slotEntityLookup = {}

    for i, slot in ipairs(matchingSlots) do
        local occupiedEnt = self.CheckSlotOccupancy(slot, self.currentVehicleType)
        if occupiedEnt == nil then
            randomSlot = slot
            break
        else
            slotEntityLookup[i] = occupiedEnt
        end
    end

    if randomSlot == nil then
        self.currentOccupyingVehicles = slotEntityLookup[1]
        randomSlot = matchingSlots[1]
    end
    
    wt:SetPosition(utils.Vec3ToVec4(randomSlot.position))
    wt:SetOrientation(randomSlot.rotation)

    self.currentDeliveredGarageName = closestGarage.name

    print("set out position and rotation")

    return wt
end

function VehicleOwnershipOverhaul:PreSummonVehicleNativeHook()
    self.currentVehicleSpawned = true
end

function VehicleOwnershipOverhaul.IsGarageBought(garage)
    if (#garage.questFact == 0) then
        return true
    end

    return Game.GetQuestsSystem():GetFactStr(garage.questFact) == 1
end

function VehicleOwnershipOverhaul.GarageOrSlotSupports(garageOrSlot, vehicleType, vehicleId)
    if not garageOrSlot.vehicleTypes or not garageOrSlot.vehicleTypes[vehicleType] then
        return false
    end

    local whiteListedVehicles = garageOrSlot.whiteListedVehicles or {}
    if next(whiteListedVehicles) ~= nil
        and not whiteListedVehicles[vehicleId] then
        return false
    end

    local blackListedVehicles = garageOrSlot.blackListedVehicles or {}
    if blackListedVehicles[vehicleId] then
        return false
    end

    return true
end

function VehicleOwnershipOverhaul.CheckSlotOccupancy(slot, vehType)
    local query = physicsQueryFilter.AddGroup("Vehicle")
    local entities = {}

    local startZ = slot.position.z + 0.2
    local endZ = startZ + 2

    local gc = gridConfig.FromVehicleType(vehType)
    print(vehType)
    if gc == nil then
        gc = gridConfig.FromVehicleType(1)
        ---@cast gc -nil
    end

    local right = slot.rotation:GetRight()
    local forward = slot.rotation:GetForward()

    local gridOriginLow = utils.AddVec3(slot.position,
        utils.AddVec3(
            utils.MultiplyVec3(right, -(gc.width / 2)),
            utils.MultiplyVec3(forward, -(gc.length / 2))
        ))
    gridOriginLow.z = startZ

    for i = 0, gc.widthSteps do
        for j = 0, gc.lengthSteps do
            local rayStart = utils.AddVec3(gridOriginLow,
                utils.AddVec3(
                    utils.MultiplyVec3(right, i * gc.stepSize),
                    utils.MultiplyVec3(forward, j * gc.stepSize)
                ))
            local rayEnd = Vector3.new(rayStart.x, rayStart.y, endZ)
            
            local hit, traceResult = GameInstance.GetSpatialQueriesSystem():SyncRaycastByQueryFilter(utils.Vec3ToVec4(rayStart), utils.Vec3ToVec4(rayEnd), query, false, false);

            if hit then
                entities[traceResult:GetHitEntity()] = true
            end
        end
    end

    local centerLow = utils.Vec3ToVec4(slot.position)
    centerLow.z = startZ

    local centerHigh = Vector4.new(centerLow)
    centerHigh.z = endZ

    local hit, traceResult = GameInstance.GetSpatialQueriesSystem():SyncRaycastByQueryFilter(centerLow, centerHigh, query, false, false);

    if hit then
        entities[traceResult:GetHitEntity()] = true
    end

    if next(entities) == nil then
        return nil
    end
    return entities
end

function VehicleOwnershipOverhaul:DespawnOccupyingVehicles()
    for veh, _ in pairs(self.currentOccupyingVehicles or {}) do
        if veh:IsPlayerVehicle() then
            ---@type vehicleGarageVehicleID
            local vehGarageId =  GarageVehicleID.Resolve(TDBID.ToStringDEBUG(veh:GetRecordID()))
            Game.GetVehicleSystem():DespawnPlayerVehicle(vehGarageId)
        else
            veh:Dispose()
        end
    end
end

return VehicleOwnershipOverhaul