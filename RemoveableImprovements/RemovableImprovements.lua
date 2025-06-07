local noImprovementID = GameInfoTypes["IMPROVEMENT_NO_IMPROVEMENT"]
local noImprovementWaterID = GameInfoTypes["IMPROVEMENT_NO_IMPROVEMENT_WATER"]

function OnBuildFinished(playerID, x, y, improvementID)
	if (improvementID == noImprovementID) or (improvementID == noImprovementWaterID) then
		Map.GetPlot(x, y):SetImprovementType(-1);
	end
end

GameEvents.BuildFinished.Add(OnBuildFinished)