 -- a table to organize data about each material
local materialData = 
{
	{
		name = "^4metal",
		model = "models/map_objects/rift/crystal_floor",
		effect = "effects/sparks/bluesparks",
	},

	{
		name = "^3crystal",
		model = "models/map_objects/rift/crystal_floor",
		effect = "effects/chunks/glassbreak",
	},
	
	{
		name = "^2organic",
		model = "models/map_objects/rift/crystal_floor",
		effect = "effects/env/acid_splash",
	},
	
	{
		name = "^1chemical",
		model = "models/map_objects/rift/crystal_floor",
		effect = "effects/scepter/impact",
	}
}

-- lua_material_pain: called when the material is hurt
function material_pain(material, attacker, damage)
	local attackWeapon = attacker:cliWeapon()
	if (attackWeapon == 1) 
	then
		local fxposition = material:Position();
		materialIndex = material:GenericValue();
		Game.Broadcast("^5You mined ^6" .. math.random(5,10) .. " " .. materialData[materialIndex].name .. "^5!", 0, attacker:Number())
		Game.PlayEffect( materialData[materialIndex].effect, fxposition)
	end
end

-- lua_material_sp: called when the material is spawned (eg. place lua_material ...)
function lua_material_sp(material)
	local materialIndex = GEntity.ReadSpawnVarInt("type") + 1;
	material:GenericValue(materialIndex);
	material:Model(materialData[materialIndex].model)
	material:Health(9999)
	material:BindPain(material_pain)
end

-- entity registration 
GEntity.Register("lua_material", lua_material_sp, 0)

