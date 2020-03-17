-- seller speaks when used
function seller_use(seller,player,activator)
    local ply = Player.FromEntity(player)
    ply:PrintCenter("hello")
end
function callseller_f(player,argc)
    local seller = player:NPCSpawn("jawa","zykmodSeller")
    seller:MakeUsable();
    seller:BindUse(seller_use)
	return 1
end
Game.BindCommand("callseller", callseller_f)

-- seller chats
function sellerchat_f(player,argc)
    local target = player:AimAnyTarget(128)
    if target ~= nil and target:Targetname() == "zykmodSeller" then
        msg = Game.ConcatArgs(1)
        if msg:find("weap") then
            player:PrintCenter("weapons menu")
        elseif msg:find("ammo") then
            player:PrintCenter("ammo menu")
        elseif msg:find("item") then
            player:PrintCenter("items menu")
        else
            return 0
        end
        player:PrintChat(string.format("^7[%s]: ^6%s", player:Name(),msg))
        return 1
    end
    return 0
end
Game.BindCommand("say", sellerchat_f)