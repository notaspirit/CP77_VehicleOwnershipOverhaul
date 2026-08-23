local utils = require("tools/utils")

local returnValue = require("models/return_value")
local garageSlot = require("models/garage_slot")

---@class Garage
---@field public name string
---@field public questFact string
---@field public position Vector3
---@field public vehicleTypes table<integer, boolean>
---@field public whiteListedVehicles table<TweakDBID, true>
---@field public blackListedVehicles table<TweakDBID, true>
---@field public slots GarageSlot[]
---@field public new fun(): Garage
---@field public Deserialize fun(self: Garage, t: table, encounteredNames: table<string, boolean>): ReturnValue
local Garage = {}

Garage.__index = Garage

function Garage.new()
    local self = setmetatable({}, Garage)
    return self
end

function Garage:Deserialize(t, encounteredNames)
    local result = returnValue.new()

    if not utils.isString(t.Name) then
        result.success = false
        result:AddErr("Name must be a string")
        return result
    end

    self.name = t.Name

    while encounteredNames[self.name] do
        local name, number = self.name:match("^(.-)(%d+)$")
        if name and number then
            self.name = name .. (tonumber(number) + 1)
        else
            self.name = self.name .. "_1"
        end
    end

    encounteredNames[self.name] = true

    if not utils.isString(t.QuestFact) then
        result.success = false
        result:AddErr("QuestFact must be a string")
        return result
    end

    self.questFact = t.QuestFact

    if not utils.isObjectArray(t.Slots) then
        result.success = false
        result:AddErr("Slots must be an object array")
        return result
    end

    self.slots = {}

    local totalSlotPosition = Vector3.new(0, 0, 0)
    self.whiteListedVehicles = {}
    self.blackListedVehicles = {}
    self.vehicleTypes = utils.buildVehicleTypeDict()

    for i, slotTable in ipairs(t.Slots) do
        local slot = garageSlot.new()
        local slotResult = slot:Deserialize(slotTable)
        if not slotResult.success then
            result:AddWarn("Failed to deserialize slot at index " .. i .. ": \n" .. slotResult:BuildLogMessage())
        else
           table.insert(self.slots, slot)
           totalSlotPosition = utils.AddVec3(totalSlotPosition, slot.position)
           self.whiteListedVehicles = utils.MergeTables(self.whiteListedVehicles, slot.whiteListedVehicles)
           self.blackListedVehicles = utils.MergeTables(self.blackListedVehicles, slot.blackListedVehicles)
           for vehType, supports in pairs(slot.vehicleTypes) do
                if supports then
                    self.vehicleTypes[vehType] = supports
                end
           end
        end
    end

    if (self.slots == nil or #self.slots == 0) then
        result.success = false
        result:AddErr("Garage must have at least one valid slot")
        return result
    end

    self.position = utils.DivideVec3(totalSlotPosition, #self.slots)

    result.success = true
    return result
end

return Garage