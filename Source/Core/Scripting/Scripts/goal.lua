require "AdHoc"

local this = GetThis()
local transform
local rigidbody

local moving = false

function Start()
    transform = GetComponent(this, "Transform")
    rigidbody = GetComponent(this, "RigidBody")
    rigidbody:SetHasGravity(false)
    rigidbody:SetLinearFactor(true, false, true)
    rigidbody:SetAngularFactor(true, true, true)
end

function Update()
    if moving == true then
		transform = GetComponent(this, "Transform")
		rigidbody = GetComponent(this, "RigidBody")
        local y = transform.translate.y
        local force = 0;

        if y > 10 then
            force = -10000;
        elseif y < 2 then
            force = 10000;
        end

         rigidbody:AddForce(0, force, 0)
    end
end

function OnCollisionEnter(rhs)
    local m = GetComponent(this, "Material")
    m.albedo.x = math.random()
    m.albedo.y = math.random()
    m.albedo.z = math.random()
    
    moving = true
end
