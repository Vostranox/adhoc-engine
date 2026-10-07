require "AdHoc"

local count = 32
local spacing = 2
local height = 0.4

local colors = {
    { 1.0, 0.3, 0.2 },
    { 1.0, 0.8, 0.2 },
    { 0.3, 1.0, 0.3 },
    { 0.2, 0.7, 1.0 },
    { 0.7, 0.3, 1.0 },
}

function Start()
    for i = 0, count - 1 do
        for j = 0, count - 1 do
            local e = CreateEntity()
            RemoveComponent(e, "Mesh")
            RemoveComponent(e, "Material")
            AddComponent(e, "Light")

            local transform = GetComponent(e, "Transform")
            transform.translate.x = (i - (count - 1) / 2) * spacing
            transform.translate.y = height
            transform.translate.z = (j - (count - 1) / 2) * spacing

            local light = GetComponent(e, "Light")
            local color = colors[(i + 2 * j) % #colors + 1]
            light.color.x = color[1]
            light.color.y = color[2]
            light.color.z = color[3]
            light.intensity = 20
            light.range = 3
        end
    end
end
