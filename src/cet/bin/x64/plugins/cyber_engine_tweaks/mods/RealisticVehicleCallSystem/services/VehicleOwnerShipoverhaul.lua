local utils = require("tools/utils")
local logger = require("libs/logger")
local json = require("libs/json")

local garage = require("models/garage")
local ReturnValue = require("models/return_value")

---@class VehicleOwnershipOverhaul
---@field public garages Garage[]
---@field public currentVehicleType number
---@field public currentVehicleID TweakDBID
---@field public currentVehicleSpawned boolean
---@field public currentDeliveredGarageName string
---@field public currentOccupyingVehicle vehicleCarBaseObject
---@field public new fun(): VehicleOwnershipOverhaul
---@field public LoadGarages fun(self: VehicleOwnershipOverhaul): void
---@field private PreSpawnPlayerVehicleNativeHook fun(self: VehicleOwnershipOverhaul, vehicleType: gamedataVehicleType, vehicleId: TweakDBID, spawnOnlyOnValidRoad: boolean): void
---@field private PostSpawnPlayerVehicleNativeHook fun(self: VehicleOwnershipOverhaul, vehicleType: gamedataVehicleType, vehicleId: TweakDBID, spawnOnlyOnValidRoad: boolean): void
---@field private PreFindSpawnLocationNativeHook fun(self: VehicleOwnershipOverhaul, playerPosition: Vector3, outPosition: Vector3, playerAndOutRotation: Quaternion): WorldTransform
---@field private PreSummonVehicleNativeHook fun(self: VehicleOwnershipOverhaul): void
---@field private IsGarageBought fun(garage: Garage): boolean
---@field private GarageOrSlotSupports fun(garage: Garage | GarageSlot, vehicleType: number, vehicleId: TweakDBID): boolean
---@field private CheckSlotOccupancy fun(slot: GarageSlot): entEntity | nil
---@field private DespawnOccupyingEntity fun(self: VehicleOwnershipOverhaul): void
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
        self:DespawnOccupyingEntity()

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

    self.currentOccupyingVehicle = nil
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
        local occupiedEnt = self.CheckSlotOccupancy(slot)
        if occupiedEnt == nil then
            randomSlot = slot
            break
        else
            slotEntityLookup[i] = occupiedEnt
        end
    end

    if randomSlot == nil then
        self.currentOccupyingVehicle = slotEntityLookup[1]
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

function VehicleOwnershipOverhaul.CheckSlotOccupancy(slot)
    local startV = utils.Vec3ToVec4(utils.AddVec3(slot.position, Vector3.new(0, 0, 0.2)))
    local endV = utils.Vec3ToVec4(utils.AddVec3(startV, Vector3.new(0, 0, 2)))
    local query = physicsQueryFilter.AddGroup("Vehicle")

    local hit, traceResult = GameInstance.GetSpatialQueriesSystem():SyncRaycastByQueryFilter(startV, endV, query, false, false);

    if not hit then
        return nil
    end

    return traceResult:GetHitEntity()
end

function VehicleOwnershipOverhaul:DespawnOccupyingEntity()
    local veh = self.currentOccupyingVehicle

    if veh == nil then
        return
    end

    if veh:IsPlayerVehicle() then
        ---@type vehicleGarageVehicleID
        local vehGarageId =  GarageVehicleID.Resolve(TDBID.ToStringDEBUG(veh:GetRecordID()))
        Game.GetVehicleSystem():DespawnPlayerVehicle(vehGarageId)
    else
        veh:Dispose()
    end
end

return VehicleOwnershipOverhaul