require "AdHoc"

local this = GetThis()
local input = GetInput()

local transform
local forward

local force = 0

local entities = {}
local nextId = 0

local mouseDown = 0

function Start()
    transform = GetComponent(this, "Transform")
end

function Update()
    if input:GetKey(AdHoc.Key.a) then
        transform.translate.x = transform.translate.x - 5 * DeltaTime()
    elseif input:GetKey(AdHoc.Key.d) then
        transform.translate.x = transform.translate.x + 5 * DeltaTime()
    end

    if input:GetKey(AdHoc.Key.q) then
        transform.rotation.y = transform.rotation.y - 2 * DeltaTime()
    elseif input:GetKey(AdHoc.Key.e) then
        transform.rotation.y = transform.rotation.y + 2 * DeltaTime()
    elseif input:GetKey(AdHoc.Key.w) then
        transform.rotation.x = transform.rotation.x + 2 * DeltaTime()
    elseif input:GetKey(AdHoc.Key.s) then
        transform.rotation.x = transform.rotation.x - 2 * DeltaTime()
    end
    
    if input:GetKey(AdHoc.Key.space) then
        transform.rotation.x = 0
        transform.rotation.y = 0
    end

    forward = transform:GetForward()
    
    if input:GetKey(AdHoc.Key.r) == true then
        force = force +  80 * DeltaTime()
        if force > 80 then
            force = 80
        end
    end
    
    if input:GetKeyUp(AdHoc.Key.r) == true then
		if force > 20 then
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
			tempRigidbody:AddVelocity(force * forward.x, forward.y * force, force * forward.z)
			tempRigidbody.radius = 0.5
			tempRigidbody:UpdateGeometry()
			tempRigidbody:SetRestitution(0.2)
        
			nextId = nextId + 1
		end

        force = 0
        transform.translate.y = -4.5
        transform.translate.z = 0
    end
    
    transform.translate.x = transform.translate.x + (math.random(math.floor(-force), math.floor(force)) / 2000)
    transform.translate.y = transform.translate.y + (math.random(math.floor(-force), math.floor(force)) / 2000)
    transform.translate.z = transform.translate.z + (math.random(math.floor(-force), math.floor(force)) / 2000)

    -- if input:GetLeftMouseButtonDown() then
    --     mouseDown = 1
    -- end
    -- if input:GetLeftMouseButtonUp() then
    --     mouseDown = 0
    -- end

    -- if mouseDown == 1 then
    --     if math.deg(transform.rotation.y) < 80 then
    --         transform.rotation.y = transform.rotation.y + 0.01
    --     elseif math.deg(transform.rotation.y) > -80 then
    --         transform.rotation.y = transform.rotation.y - 0.01
    --     end
    -- end
end
