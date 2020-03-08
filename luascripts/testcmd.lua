function testcmdf(player,args)
	Game.Broadcast("lua script says hello", 16) 
	return 1
end

Game.BindCommand("testcmd", testcmdf)
