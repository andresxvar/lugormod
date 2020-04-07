-- rancor speaks when used
function rancor_use(seller,player,activator)
    local ply = Player.FromEntity(player)
    ply:PrintChat("^1Rancor^7: ^6rawr!!")
end

function makerancor_f(player,argc)
    local rancor = player:NPCSpawn("rancor","tffarancor")
    rancor:RGBA(tonumber(Game.Argument(1)),tonumber(Game.Argument(2)),tonumber(Game.Argument(3)),tonumber(Game.Argument(4)))
    rancor:MakeUsable();
    rancor:BindUse(rancor_use)
	return 1
end

function entrgb_f(player,argc)
    local tent = GEntity.FromNumber(tonumber(Game.Argument(1)))
    tent:RGBA(tonumber(Game.Argument(2)),tonumber(Game.Argument(3)),tonumber(Game.Argument(4)),tonumber(Game.Argument(5)))
	return 1
end

Game.BindCommand("makerancor", makerancor_f)
Game.BindCommand("entrgb", entrgb_f)


function lua_boltmodel_use(self,player,activator)
    local pl = Player.FromEntity(activator)
    local model = self:Model()
    pl:BoltModel(model,0.5)
end
function lua_boltmodel_sp(self)
    self:BindUse(lua_boltmodel_use)
end
GEntity.Register("lua_boltmodel", lua_boltmodel_sp, 0)


GEntity.Register("lua_rancorSpawner", lua_rancorSpawner_sp, 0)

function lua_rancorSpawner_think(self)
    local rancor = self:NPCSpawn("rancor","tffarancor")
    rancor:RGBA(255,0,0,0)
    rancor:Team(1)
end

function lua_rancorSpawner_sp(self)
    local rgb = GEntity.ReadSpawnVarInt("rgb")

    self:BindThink(lua_rancorSpawner_think)
end





