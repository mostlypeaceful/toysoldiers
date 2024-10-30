// Script for controlling how pro-tips are shown

// Change this when you add more pro-tips
const PROTIP_COUNT = 57

function GetProTipLocString( tipIndex )
{
	return ::GameApp.LocString( "ProTip" + tipIndex.tostring( ) ).Clone( )
}

function GetProTipLocStringWithLabel( tipIndex )
{
	return ::GameApp.LocString( "ProTip_Label" ) % " " % GetProTipLocString( tipIndex )
}

function GetRandomProTipLocString( )
{
	return GetProTipLocString( ::SubjectiveRand.Int( 0, PROTIP_COUNT - 1 ).tostring( ) )
}

function GetRandomProTipLocStringWithLabel( )
{
	return GetProTipLocStringWithLabel( ::SubjectiveRand.Int( 0, PROTIP_COUNT - 1 ).tostring( ) )
}