sigimport "Gameplay/Mobile/Helicopter/USSR/ussr_helomi12_engine_momap.nut"
sigimport "Gameplay/Mobile/Helicopter/USSR/HeloMi12_engine.goaml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Mi12_Engine( )
}


class USSR_Helo_Mi12_Engine extends AnimatedBreakableLogic
{	
	ownerBoss = null
	
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
		ownerBoss = null
	}

	function DebugTypeName( )
		return "USSR_Helo_Mi12_Engine"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
		AnimatedBreakableLogic.AddHealthBar( )
		SetDamageTintColor( Math.Vec4.Construct( 3,0,0,0 ) )
	}	
	
	function SetBoss( boss ) ownerBoss = boss
	
	function SetMotionMap( ) 
	{
		if( OwnerEntity.GetName( ) == "engine_l" ) Animatable.MotionMap = USSRBossHomerEngineLMoMap( )
		else Animatable.MotionMap = USSRBossHomerEngineRMoMap( )
	}
	
	function SetMasterGoal( ) GoalDriven.MasterGoal = HomerEngineGoal( this, { } )
}
