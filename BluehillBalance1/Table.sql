ALTER TABLE BuildFeatures ADD COLUMN Food INT DEFAULT 0;

ALTER TABLE Policies ADD COLUMN AllFeatureFood INT DEFAULT 0;

ALTER TABLE Technologies ADD COLUMN FeatureFoodModifier INT DEFAULT 0;

ALTER TABLE HandicapInfos ADD COLUMN FreeXPPercent INT DEFAULT 0;