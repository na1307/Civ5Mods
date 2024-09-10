-- Major civs
for playerID = 0, GameDefines.MAX_MAJOR_CIVS-1, 1 do
 local pPlayer = Players[playerID];
 if pPlayer:IsAlive() then
  pPlayer:SetAlwaysSeeBarbCampsCount(1);
 end
end 

-- Minor civs
for playerID = GameDefines.MAX_MAJOR_CIVS, GameDefines.MAX_CIV_PLAYERS-1, 1 do
 local pPlayer = Players[playerID];
 if pPlayer:IsAlive() then
  pPlayer:SetAlwaysSeeBarbCampsCount(1);
 end
end