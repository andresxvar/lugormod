-- this script shows how a new entity classname can be created using lua
-- newentsp is called when the entity spawn

function newentsp(ent)
		Game.Broadcast(os.date("(%H:%M:%S) ", osTime) .. "\x19^7: ^2spawned!?", 16)
end

GEntity.Register("lmd_newent", newentsp, 1)
