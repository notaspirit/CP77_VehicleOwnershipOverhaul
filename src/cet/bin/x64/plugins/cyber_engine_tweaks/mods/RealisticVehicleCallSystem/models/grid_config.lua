---@class GridConfig
---@field public width number
---@field public length number
---@field public stepSize number
---@field public widthSteps number
---@field public lengthSteps number
---@field public new fun(): GridConfig
---@field public FromVehicleType fun(vehType: int): GridConfig | nil
---@field public Deserialize fun(self: GridConfig, t: table, encounteredNames: table<string, boolean>): ReturnValue
local GridConfig = {}

GridConfig.__index = GridConfig

function GridConfig.new()
    local self = setmetatable({}, GridConfig)
    return self
end

function GridConfig.FromVehicleType(vehType)
    local gc = GridConfig.new()

    if vehType == 0 then
        gc.width = 1
        gc.length = 3
    else if vehType == 1 or vehType == 2 then
        gc.width = 3
        gc.length = 6
    else
        return nil
    end
    end

    gc.stepSize = 0.5

    gc.widthSteps = gc.width / gc.stepSize
    gc.lengthSteps = gc.length / gc.stepSize
    return gc
end

return GridConfig