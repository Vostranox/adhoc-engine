require "AdHoc"

local this = GetThis()
local transform

function Start()
    transform = GetComponent(this, "Transform")
end

function Update()
    transform.rotation.y = transform.rotation.y + 2 * DeltaTime()

    local e = Raycast(transform.translate, transform:GetForward(), 50)
    if e ~= 0 then
        local m = GetComponent(e, "Material")
        m.albedo.x = math.random()
        m.albedo.y = math.random()
        m.albedo.z = math.random()
        m.metallicness = math.random()
    end
end