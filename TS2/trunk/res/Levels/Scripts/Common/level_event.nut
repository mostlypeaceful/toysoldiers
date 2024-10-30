
class ScriptedLevelEvent
{
	conditions = null
	callBack = null
	
	constructor( cbFunc )
	{
		conditions = { }
		callBack = cbFunc
	}
	
	function AddCondition( stringId, startValue )
	{
		conditions[ stringId ] <- startValue
	}
	
	function UpdateCondition( stringId )
	{
		if( conditions.rawin( stringId ) )
		{
			conditions[ stringId ] -= 1
			
			if( AllConditionsMet( ) )
				Fire( )
		}
		else
		{
			print( "UpdateCondition failed. Can't find condition: " + stringId )
		}
	}
	
	function AllConditionsMet( )
	{
		foreach( i in conditions )
		{
			if( i > 0 )
				return false
		}
		
		return true
	}
	
	function Fire( )
	{
		if( callBack )
			callBack( )
	}
}
