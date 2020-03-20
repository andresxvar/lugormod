-- adjust player's healthbar every few seconds
function lua_survivalmode_think(self)
    for plid = 0,9 do
        local ply = GEntity.FromNumber(plid)
        local plycl = Player.FromNumber(plid)
        plycl:MaxHealth(self:Health())
        ply:ScaleNetHealth()
    end
    self:NextThink(Game.Time() + 3000)
end

-- lua_survivalmode_setup: extra entitties
function lua_survivalmode_setup(self)
    GEntity.Place("classname,lmd_event,deathtarget,sDie,")
    GEntity.Place("classname,target_relay,targetname,sDie,target,sDropCr,target2,sSpec")
    GEntity.Place("classname,lmd_drop,spawnflags,1,targetname,sDropCr")
    GEntity.Place("classname,lua_spectimer,targetname,sSpec")    
    self:BindThink(lua_survivalmode_think)
    self:NextThink(Game.Time() + 3000)    
end

-- lua_survivalmode_sp: handle ent spawn
function lua_survivalmode_use(self,player,activator)
    local maxhp = self:Health()
    local ply = Player.FromEntity(activator)    
    ply:MaxHealth(maxhp)
    ply:Health(maxhp)
end

function lua_survivalmode_sp(self)
    local maxhp = self:Health()
    local ply = Player.FromNumber(0)
    for plid = 0,9 do
        local ply = Player.FromNumber(plid)
        ply:MaxHealth(maxhp)
        ply:Health(maxhp)
    end
    self:BindThink(lua_survivalmode_setup)
    self:NextThink(Game.Time() + 500)
    self:BindUse(lua_survivalmode_use)
end

function lua_spectimer_use(self,player,activator)
    local ply = Player.FromEntity(activator)
    ply:SiegeSpecTimer(10)
    ply:Team("s",10)
end 
function lua_spectimer_sp(self)
    self:BindUse(lua_spectimer_use)
end

GEntity.Register("lua_survivalmode", lua_survivalmode_sp, 0)
GEntity.Register("lua_spectimer",lua_spectimer_sp,0)