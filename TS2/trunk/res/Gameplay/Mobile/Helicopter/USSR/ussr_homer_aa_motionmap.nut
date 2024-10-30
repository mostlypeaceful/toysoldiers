sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Bosses/Red/mi12_homer_turret_top/mi12_homer_turret_top.anipk"

class ussr_homer_aa_motionmap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/mi12_homer_turret_top/mi12_homer_turret_top.anipk" )
	}

	function FireOneShot( params )
	{
		return 1.0
	}
	
	function GhostIdle( params )
	{
	}
	function Idle( params )
	{
	}
	function SpinUp( params )
	{
		return 0.1
	}
	function SpinDown( params )
	{
		return 0.1
	}
	function Reload( params )
	{
		return 5.0
	}

	function Recoil( params )
	{
		return 0.1
	}
	function FireLooping( params )
	{
	}
	function Repair( params )
	{
		return 5.0
	}
	function Upgrade( params )
	{
		return 5.0
	}

}

