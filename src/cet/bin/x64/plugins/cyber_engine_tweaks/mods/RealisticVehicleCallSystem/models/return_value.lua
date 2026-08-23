local utils = require("tools/utils")



---@class ReturnValue
---@field public success boolean
---@field public warning_message string
---@field public error_message string
---@field public data any
---@field public AddErr fun(self: ReturnValue, msg: string): void
---@field public AddWarn fun(self: ReturnValue, msg: string): void
---@field public BuildLogMessage fun(self: ReturnValue): string
local ReturnValue = {}

ReturnValue.__index = ReturnValue

function ReturnValue.new()
    local self = setmetatable({}, ReturnValue)
    self.success = false
    self.warning_message = ""
    self.error_message = ""
    self.data = nil
    return self
end

function ReturnValue:AddErr(msg)
    self.error_message = self.error_message .. "\n" .. msg
end

function ReturnValue:AddWarn(msg)
    self.warning_message = self.warning_message .. "\n" .. msg
end

function ReturnValue:BuildLogMessage()
    local logMessage = ""
    if self.error_message ~= "" then
        logMessage = logMessage .. "[ERRORS] " .. self.error_message .. "\n"
    end
    if self.warning_message ~= "" then
        logMessage = logMessage .. "[WARNINGS] " .. self.warning_message .. "\n"
    end
    return logMessage
end

return ReturnValue
