require "AdHoc"

local this = GetThis()
local entities = {}
local rigidbodies = {}
local nextId = 0

function FixedUpdate()
        entities[nextId] = CreateEntity()
        AddComponent(entities[nextId], "RigidBody", "Box", "Dynamic");
        rigidbodies[nextId] = GetComponent(entities[nextId], "RigidBody")
        rigidbodies[nextId]:SetRestitution(0.1)
        nextId = nextId + 1
end
