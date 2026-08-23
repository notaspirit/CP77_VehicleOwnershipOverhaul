
---@class Utils
---@field public buildVehicleTypeDict fun(): table<integer, boolean>
---@field public isStringArray fun(t: any): boolean
---@field public isObjectArray fun(t: any): boolean
---@field public isVector3 fun(t: any): boolean
---@field public isQuaternion fun(t: any): boolean
---@field public isString fun(t: any): boolean
---@field public GetVehicleTypeEnumFromString fun(vehicleType: string): integer?
---@field public AddVec3 fun(a: Vector3 | Vector4, b: Vector3 | Vector4): Vector3
---@field public DivideVec3 fun(a: Vector3, scalar: number): Vector3
---@field public Vec3ToVec4 fun(a: Vector3): Vector4
---@field public BuildDefaultSimpleMessage fun(): SimpleScreenMessage
---@field public Distance fun(a: Vector3 | Vector4, b: Vector3 | Vector4): number
---@field public ListFilesInDir fun(directory: string, ext: string): table<string>
---@field public ShuffleArray fun(t: table): void
---@field public DeepCopy fun(t: table): table
local Utils = {}

function Utils.buildVehicleTypeDict()
    local vehicleTypes = {}
    vehicleTypes[tonumber(EnumInt(gamedataVehicleType.Bike))] = false
    vehicleTypes[tonumber(EnumInt(gamedataVehicleType.Car))] = false
    vehicleTypes[tonumber(EnumInt(gamedataVehicleType.Panzer))] = false
    vehicleTypes[tonumber(EnumInt(gamedataVehicleType.Count))] = false
    vehicleTypes[tonumber(EnumInt(gamedataVehicleType.Invalid))] = false
    return vehicleTypes
end

function Utils.isStringArray(t)
    if t == nil then
        return false
    end

    if type(t) ~= "table" then
        return false
    end

    for k, v in pairs(t) do
        if type(v) ~= "string" then
            return false
        end
    end

    return true
end

function Utils.isObjectArray(t)
    if t == nil then
        return false
    end

    if type(t) ~= "table" then
        return false
    end

    for k, v in pairs(t) do
        if type(v) ~= "table" then
            return false
        end
    end

    return true
end

function Utils.isVector3(t)
    if t == nil then
        return false
    end

    if type(t) ~= "table" then
        return false
    end

    if type(t.X) ~= "number" or type(t.Y) ~= "number" or type(t.Z) ~= "number" then
        return false
    end

    return true
end

function Utils.isQuaternion(t)
    if t == nil then
        return false
    end

    if type(t) ~= "table" then
        return false
    end

    if type(t.I) ~= "number" or type(t.J) ~= "number" or type(t.K) ~= "number" or type(t.R) ~= "number" then
        return false
    end

    return true
end

function Utils.isString(t)
    return t ~= nil and type(t) == "string"
end

local gamedataVehicleType_lookup = {}
gamedataVehicleType_lookup["Bike"] = tonumber(EnumInt(gamedataVehicleType.Bike))
gamedataVehicleType_lookup["Car"] = tonumber(EnumInt(gamedataVehicleType.Car))
gamedataVehicleType_lookup["Panzer"] = tonumber(EnumInt(gamedataVehicleType.Panzer))
gamedataVehicleType_lookup["Count"] = tonumber(EnumInt(gamedataVehicleType.Count))
gamedataVehicleType_lookup["Invalid"] = tonumber(EnumInt(gamedataVehicleType.Invalid))

function Utils.GetVehicleTypeEnumFromString(vehicleType)
    return gamedataVehicleType_lookup[vehicleType]
end

function Utils.AddVec3(a, b)
    return Vector3.new(a.x + b.x, a.y + b.y, a.z + b.z)
end

function Utils.DivideVec3(a, scalar)
    return Vector3.new(a.x / scalar, a.y / scalar, a.z / scalar)
end

function Utils.Vec3ToVec4(a)
    return Vector4.new(a.x, a.y, a.z, 1)
end

function Utils.MergeTables(t1, t2)
    local merged = {}
    for k, v in pairs(t1) do
        merged[k] = v
    end
    for k, v in pairs(t2) do
        merged[k] = v
    end
    return merged
end

function Utils.BuildDefaultSimpleMessage()
    local msg = SimpleScreenMessage.new()
    msg.isShown = true
    msg.duration = 5.0
    msg.isInstant = true
    msg.type = SimpleMessageType.Neutral
    return msg
end

function Utils.Distance(a, b)
    local dx = a.x - b.x
    local dy = a.y - b.y
    local dz = a.z - b.z

    return math.sqrt(dx * dx + dy * dy + dz * dz)
end

function Utils.ListFilesInDir(directory, ext)
    local files = {}
    local success, dir_files = pcall(function()
        return dir(directory)
    end)

    if not success then
        print("failed to get dir")
        return files
    end

    for _, file in pairs(dir_files) do
        if file.name:lower():match("%." .. ext .. "$") then
            table.insert(files, file)
        end
    end
    
    return files
end

function Utils.ShuffleArray(t)
    for i = #t, 2, -1 do
        local j = math.random(1, i)
        t[i], t[j] = t[j], t[i]
    end
end

function Utils.DeepCopy(t)
    local o = {}
    for k, v in pairs(t) do
        if type(v) == "table" then
            o[k] = Utils.DeepCopy(v)
        else
            o[k] = v
        end
    end
    return o
end

return Utils