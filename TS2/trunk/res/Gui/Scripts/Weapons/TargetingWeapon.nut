// Base weapon UI for units that have target-locking ability

// Requires
sigimport "gui/scripts/weapons/weaponbase.nut"
sigimport "gui/scripts/weapons/targetlock.nut" 

sigvars Acquire Targets
@[AcqTargetSize] { "Size", (1280,720), [ 0:2000 ] }

sigvars Target Lock
@[TargetLockSize] { "Size", (180,180), [ 0:2000 ] }

class TargetingWeaponUI extends WeaponBase
{
	// Display
	targets = null
	locks = null
	
	// Data
	visRect = null

	constructor( owner_, reticle_ = null )
	{
		::WeaponBase.constructor( owner_, reticle_ )
		targets = { }
		locks = { }
		visRect = ::Math.Rect.Construct( ::Math.Vec2.Construct( 0, 0 ), @[AcqTargetSize] )
	}
	
	function AddTarget( uID, pos )
	{
		if( uID in targets && targets[ uID ] )
		{
			//::print( vpIndex.tostring( ) + ": Tried to add " + uID.tostring( ) + " but it already existed" )
			RemoveTarget( uID )
		}
		
		//::print( vpIndex.tostring( ) + ": Added " + uID.tostring( ) )
		targets[ uID ] <- AddIcon( ::TargetIcon( ), pos )
	}
	
	function RemoveTarget( uID )
	{
		if( uID in targets )
		{
			//::print( vpIndex.tostring( ) + ": Removed " + uID.tostring( ) )
			targets[ uID ].DeleteSelf( )
			RemoveChild( targets[ uID ] )
			delete targets[ uID ]
		}
	}
	
	function SetTargetPosition( uID, pos )
	{
		if( uID in targets )
			targets[ uID ].SetPosition( pos )
	}
	
	function AddLock( uID, pos )
	{
		if( uID in locks && locks[ uID ] )
			RemoveLock( uID )
			
		locks[ uID ] <- AddIcon( ::LockIcon( ), pos )
	}
	
	function AddIcon( icon, pos )
	{
		icon.SetPosition( pos )
		icon.SetScissorRect( visRect )
		AddChild( icon )
		
		return icon
	}
	
	function RemoveLock( uID )
	{
		if( uID in locks )
		{
			RemoveChild( locks[ uID ] )
			delete locks[ uID ]
		}
	}
	
	function SetLockPosition( uID, pos )
	{
		if( uID in locks )
			locks[ uID ].SetPosition( pos )
	}
	
	function ClearTargets( )
	{
		foreach( key, val in targets) 
			RemoveChild( val )
		targets.clear( )
		foreach( key, val in locks) 
			RemoveChild( val )
		locks.clear( )
	}
	
	function UserControl( userControlled, player )
	{
		if( !userControlled )
			ClearTargets( )
		
		if( userControlled && player )
			visRect = player.User.ComputeViewportRect( )
		
		::WeaponBase.UserControl( userControlled, player )
	}
	
	function TargetingBoxSize( )
	{
		return visRect.WidthHeight
	}
	
	function TargetLockSize( )
	{
		return @[TargetLockSize]
	}
}