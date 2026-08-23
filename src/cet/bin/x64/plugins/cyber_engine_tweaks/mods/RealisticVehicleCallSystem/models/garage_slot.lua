local utils = require("tools/utils")
local logger = require("libs/logger")

local ReturnValue = require("models/return_value")


---@class GarageSlot
---@field public vehicleTypes table<integer, boolean>
---@field public whiteListedVehicles table<TweakDBID, true>
---@field public blackListedVehicles table<TweakDBID, true>
---@field public position Vector3
---@field public rotation Quaternion
---@field public new fun(): GarageSlot
---@field public Deserialize fun(self: GarageSlot, t: table): ReturnValue
local GarageSlot = {}

GarageSlot.__index = GarageSlot

function GarageSlot.new()
    local self = setmetatable({}, GarageSlot)
    return self
end


function GarageSlot:Deserialize(t)
    local result = ReturnValue.new()

    self.vehicleTypes = utils.buildVehicleTypeDict()
    self.whiteListedVehicles = {}
    self.blackListedVehicles = {}
    
    if not utils.isStringArray(t.VehicleTypes) then
        result.success = false
        result:AddErr("VehicleTypes must be an array of strings")
        return result
    end

    for _, vehicleType in ipairs(t.VehicleTypes) do
        local vehicleTypeEnum = utils.GetVehicleTypeEnumFromString(vehicleType)
        if vehicleTypeEnum == nil then
            result:AddWarn("Unknown vehicle type: " .. vehicleType)
        else
            self.vehicleTypes[vehicleTypeEnum] = true
        end
    end

    if not utils.isStringArray(t.WhiteListedVehicles) then
        result.success = false
        result:AddErr("WhiteListedVehicles must be an array of strings")
        return result
    end

    for _, vehicle in ipairs(t.WhiteListedVehicles) do
        self.whiteListedVehicles[TweakDBID.new(vehicle)] = true
    end

    if not utils.isStringArray(t.BlackListedVehicles) then
        result.success = false
        result:AddErr("BlackListedVehicles must be an array of strings")
        return result
    end

    for _, vehicle in ipairs(t.BlackListedVehicles) do
        self.blackListedVehicles[TweakDBID.new(vehicle)] = true
    end

    if not utils.isVector3(t.Position) then
        result.success = false
        result:AddErr("Position must be a Vector3 with X, Y, Z fields")
        return result
    end

    self.position = Vector3.new(t.Position.X, t.Position.Y, t.Position.Z)

    print("slot pos is " .. require("libs/json").stringify(t.Position))

    if not utils.isQuaternion(t.Rotation) then
        result.success = false
        result:AddErr("Rotation must be a Quaternion with I, J, K, R fields")
        return result
    end

    self.rotation = Quaternion.new(t.Rotation.I, t.Rotation.J, t.Rotation.K, t.Rotation.R)

    result.success = true
    return result
end

return GarageSlot
