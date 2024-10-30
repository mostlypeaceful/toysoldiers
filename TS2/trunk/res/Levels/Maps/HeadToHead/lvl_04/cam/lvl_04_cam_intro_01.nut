sigvars Camera Path Parameters
@[TOTAL_TIME] { "Total Time", 5.0, [ 0.0:999999999.0 ], "TODO COMMENT" }
@[EASE_IN] { "Ease In", 0.5, [ 0.0:1.0 ], "TODO COMMENT" }
@[EASE_OUT] { "Ease Out", 0.5, [ 0.0:1.0 ], "TODO COMMENT" }
@[GAME_BLEND_IN] { "Game Blen In", 0.5, [ 0.0:999999999.0 ], "TODO COMMENT" }

sigexport function EntityOnCreate( entity )
{
	entity.Logic = FollowPathCameraPointLogic( )
	local params = entity.Logic.Params
	
	params.TotalTime = @[TOTAL_TIME]
	params.EaseIn = @[EASE_IN]
	params.EaseOut = @[EASE_OUT]
	params.GameBlendIn = @[GAME_BLEND_IN]
	params.LookAtTarget = "nis_intro_focal_target_01"
	
	GameApp.CurrentLevel.RegisterCameraPathStart( entity )
}
