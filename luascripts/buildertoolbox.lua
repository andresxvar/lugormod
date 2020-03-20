
function ctrain_f(player,argc)
    if player:AdminRank() == 0 then
        return 0
    end

    local origin = player:Position()
    local radius = 300
    local n = 100

    if argc < 3 then
        player:PrintConsole("^7Syntax: /ctrain <name> <radius>\n")
    else
        local trainName = Game.Argument(1)
        local radius = tonumber(Game.Argument(2))

        for i=0,n do
            local theta = i*(2*math.pi)/n            
            local newx = origin[0] + radius*math.cos(theta)
            local newy = origin[1] + radius*math.sin(theta)
            local newz = origin[2]        
            local targetname = string.format("%s_pc_%d",trainName,i)
            local target
            if (i < n) then
                target = string.format("%s_pc_%d",trainName,i+1)            
            else
                target = string.format("%s_pc_%d",trainName,0)
            end

            GEntity.Place("classname,path_corner,group,".. trainName ..",targetname," .. targetname .. ",target," .. target .. ",origin,".. newx .." ".. newy .." ".. newz,1) 
        end
        GEntity.Place("classname,lmd_train,group,".. trainName ..",target,".. trainName .. "_pc_0,model,models/map_objects/factory/catw2_b",1) 
        player:PrintConsole("...Train created\n")
    end
    return 1;
end

Game.BindCommand("ctrain", ctrain_f)