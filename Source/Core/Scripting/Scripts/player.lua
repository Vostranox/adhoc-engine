require "AdHoc"

local this = GetThis()
AdHoc.Global.playerID = this
local rigidbody = GetComponent(this, "RigidBody")
local transform = GetComponent(this, "Transform")
local input = GetInput()
local entities = {}
local nextId = 0
local force = 0
local forward

function Start()
    rigidbody:SetAngularFactor(0,0,0)
    SetGravity(0,-9.7,0)
end

function Update()
    if input:GetKey(AdHoc.Key.d) == true then
        rigidbody:AddForce(1500, 0, 0)
        rigidbody:SetRotation(0, math.rad(90), 0)
    elseif input:GetKey(AdHoc.Key.a) == true then
        rigidbody:AddForce(-1500, 0, 0)
        rigidbody:SetRotation(0, math.rad(270), 0)
    elseif input:GetKey(AdHoc.Key.w) == true then
        rigidbody:AddForce(0, 0, 1500)
        rigidbody:SetRotation(0, math.rad(0), 0)
    elseif input:GetKey(AdHoc.Key.s) == true then
        rigidbody:AddForce(0, 0, -1500)
        rigidbody:SetRotation(0, math.rad(180), 0)
    end

    if input:GetKey(AdHoc.Key.r) == true then
        force = force +  10 * DeltaTime()
    end
    if input:GetKeyUp(AdHoc.Key.r) == true then
        entities[nextId] = CreateEntity()

        local transforms = GetComponent(entities[nextId], "Transform")
        transforms.scale.x = 0.5
        transforms.scale.y = 0.5
        transforms.scale.z = 0.5

        local meshes = GetComponent(entities[nextId], "Mesh")
        meshes:Load("sphere.obj")

        forward = transform:GetForward()

        AddComponent(entities[nextId], "RigidBody", "Sphere", "Dynamic");
        local tempRigidbody = GetComponent(entities[nextId], "RigidBody")
        tempRigidbody:SetTranslation(transform.translate.x + 2 * forward.x, transform.translate.y, transform.translate.z + 2 * forward.z)
        tempRigidbody:AddVelocity(force * forward.x, force * 2, force * forward.z)
        tempRigidbody.radius = 0.5
        tempRigidbody:UpdateGeometry()
        tempRigidbody:SetRestitution(0.2)
        
        nextId = nextId + 1

        force = 0
    end
end

function FixedUpdate()
    for i = 0, nextId - 1 do
        local r = GetComponent(entities[i], "RigidBody")
        r:AddForce(0, -20, 0)
    end
end