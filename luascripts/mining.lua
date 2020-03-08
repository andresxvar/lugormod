function material_pain(material, attacker, damage)
	local attackWeapon = attacker:cliWeapon()
	if (attackWeapon == 1) 
	then
	        Game.Broadcast( "ouch!",  0,  attacker:Number())
	end
end

function material_f(player, argsc)
	local origin = player:cliAimOrigin()
	local entid = player:Number()

	material = GEntity.Place("classname,misc_model_breakable,origin,".. origin[0] .." ".. origin[1] .." ".. origin[2] ..",targetname,tech_".. entid .."_lua_pb,spawnflags,97,health,99999,target,tech_".. entid .."_lua_pb_on,wait,5,model,items/psgun.glm,modelscale,2")	

	material:BindPain(material_pain)

	return 1
end

Game.BindCommand("spawnmaterial", material_f)
