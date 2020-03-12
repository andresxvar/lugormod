-- a table of different materials
local materialData = 
{
	{
		name = "^3crystal",
		model = "models/map_objects/rift/crystal_floor",
		effect = "effects/chunks/glassbreak",
	},

	{
		name = "^4iron",
		model = "models/map_objects/rift/crystal_floor",
		effect = "effects/sparks/bluesparks",
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

-- lua_material_pain: handle mining
function material_pain(material, attacker, damage)
	local ply = Player.FromEntity(attacker)

	if (ply:Weapon() == 1) then
		local fxposition = material:Position()
		fxposition[2] = fxposition[2]+32
		materialIndex = material:GenericValues()
		materialCount = math.random(100,200)
		plyNum = attacker:Number()
		Game.Broadcast("^5You mined ^6" .. materialCount  .. " " .. materialData[materialIndex].name .. "^5!", 0, plyNum)
		materialInventory[plyNum][materialIndex] = materialInventory[plyNum][materialIndex] + materialCount
		Game.PlayEffect( materialData[materialIndex].effect, fxposition)
	end
end

-- lua_material_sp: handle spawning entity
function lua_material_sp(material)
	local materialIndex = GEntity.ReadSpawnVarInt("type") + 1
	material:GenericValues(materialIndex)
	material:Model(materialData[materialIndex].model)
	material:Health(9999)
	material:BindPain(material_pain)
end

-- register the entity to lugormod
GEntity.Register("lua_material", lua_material_sp, 0)

-- initialize the inventory for all players
materialInventory = {}
for i=0,31 do
	materialInventory[i] = {0,0,0,0}
end 

function buildable_fire_sp(player)
	local loc = player:AimOrigin(64)
	local fire = GEntity.Place("classname,fx_runner,spawnflags,4,fxfile,env/fire,delay,100,dmg,150,gravity,1,origin,".. loc[0] .." ".. loc[1] .." ".. loc[2])
end

-- bomb explodes when used
function bomb_use(material,player,activator)
	local ply = Player.FromEntity(player)
	if(ply:Hack(material,4000)) then
		material:Blowup(5000)
	end
end
-- spawn a hackable bomb
function buildable_bomb_sp(player)		
	local loc = player:AimOrigin(64)
	local bomb = GEntity.Place("classname,misc_model_breakable,gravity,1,origin,".. loc[0] .." ".. loc[1] .." ".. loc[2] ..",spawnflags,129,model,models/map_objects/imperial/crate_banded")
	bomb:MakeHackable()
	bomb:BindUse(bomb_use)
end

-- table of buildables
local buildableData = 
{
	{
		name = "small fire",
		materialcost = {10,0,10,10},
		spawn = buildable_fire_sp,
	},
	{
		name = "bomb",
		materialcost = {10,20,0,20},
		spawn = buildable_bomb_sp,
	}
}
-- handle the "build" command
function build_f(player,argc)
	local pid = player:Number()
	local pInv = materialInventory[pid]
	if argc == 1 then
		-- print buildable table
		player:PrintConsole("^5Buildable       ^4Crystal      ^2Iron         ^6Organic      ^1Chemical\n")
		for k, v in pairs (buildableData) do
			player:PrintConsole(string.format("^3%-15s ^7%-12d %-12d %-12d %d\n",
			v.name,v.materialcost[1],v.materialcost[2],v.materialcost[3],v.materialcost[4]))
		end
		player:PrintConsole(string.format("^5Inventory       ^3%-12d %-12d %-12d %d\n",
			pInv[1],pInv[2],pInv[3],pInv[4]))
	else
		-- do some building
		query = Game.ConcatArgs(1)
		for k, v in pairs (buildableData) do
			if v.name == query then
				if pInv[1]>=v.materialcost[1] and pInv[2]>=v.materialcost[2] 
					and pInv[3]>=v.materialcost[3] and pInv[4]>=v.materialcost[4] then
				player:PrintConsole("... making a " .. query .."\n")
				v.spawn(player);				
				materialInventory[pid][1] = materialInventory[pid][1] - v.materialcost[1];
				materialInventory[pid][2] = materialInventory[pid][2] - v.materialcost[2];
				materialInventory[pid][3] = materialInventory[pid][3] - v.materialcost[3];
				materialInventory[pid][4] = materialInventory[pid][4] - v.materialcost[4];
				end
			end			
		end		
	end
	return 1
end


function buildall_f(player,argc)
	local pid = player:Number()
	materialInventory[pid][1] = 999;
	materialInventory[pid][2] = 999;
	materialInventory[pid][3] = 999;
	materialInventory[pid][4] = 999;
	return 1
end


Game.BindCommand("build", build_f)
Game.BindCommand("buildall", buildall_f)