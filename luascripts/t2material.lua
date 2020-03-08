-- lua_material_pain: pain handler
function material_pain(material, attacker, damage)
	local attackWeapon = attacker:cliWeapon()
	if (attackWeapon == 1) 
	then
		Game.Broadcast("^5You mined ^6" .. math.random(5,10) .. " ^5hugs!", 0, attacker:Number())
		Game.PlayEffect( "effects/sparks/bluesparks", material:Position(), material:Position())
	end
end

-- lua_material_sp: spawning handler
function lua_material_sp(material)
	material:Model("models/map_objects/rift/crystal_floor")
	material:Health(9999)
	material:BindPain(material_pain)
end

-- entity registration
GEntity.Register("lua_material", lua_material_sp, 0)




