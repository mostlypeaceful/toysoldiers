#include "MaxPlugin.hpp"
#include "StringUtil.hpp"
#include <sstream>

namespace Sig { namespace MaxPlugin
{
	static b32 gQuietMode = false;

	b32	fGetQuietModeEnabled( )
	{
		return gQuietMode;
	}

	void fSetQuietModeEnabled( b32 enabled )
	{
		gQuietMode = enabled;
	}

	f32 fToSeconds( TimeValue t )
	{
		const f32 ticksPerFrame = ( f32 )GetTicksPerFrame( );
		const f32 framesPerSecond = ( f32 )GetFrameRate( );
		const f32 numTicks = ( f32 )t;
		return ( numTicks / ( ticksPerFrame * framesPerSecond ) );
	}

	TimeValue fToTimeValue( f32 seconds )
	{
		return ( TimeValue )( seconds * f32( GetTicksPerFrame( ) * GetFrameRate( ) ) + 0.49999f );
	}

	u32	fComputeNumINodes( INode* root )
	{
		u32 i = 1;
		for( u32 ichild = 0; ichild < root->NumberOfChildren( ); ++ichild )
			i += fComputeNumINodes( root->GetChildNode( ichild ) );
		return i;
	}

	INode* fFindINode( Interface* maxIface, INode* root, const char* nodeName )
	{
		if( !_stricmp( root->GetName( ), nodeName ) )
			return root;

		for( u32 i = 0; i < root->NumberOfChildren( ); ++i )
		{
			INode* o = fFindINode( maxIface, root->GetChildNode( i ), nodeName );
			if( o )
				return o;
		}

		return 0;
	}

	Object* fEvaluateINode( Interface* maxIface, INode* node, TimeValue t )
	{
		if( t == (TimeValue)-1 )
			t = maxIface->GetAnimRange( ).Start( );

		ObjectState os = node->EvalWorldState( t );

		return os.obj;
	}

	TriObject* fGetTriobjFromObject( Interface* maxIface, Object* object, TimeValue t )
	{
		if( t == (TimeValue)-1 )
			t = maxIface->GetAnimRange( ).Start( );

		if( !object->CanConvertToType( triObjectClassID ) )
			return 0;

		return ( TriObject* )object->ConvertToType( t, triObjectClassID );
	}

	void fMakeINodeNameSafeForXml( std::string& out, const char* nodeName )
	{
		out = nodeName;

		strutl::fReplaceAllOf( out, " ", "_" );
	}

	void fMakePointerSafeForXml( std::string& out, void* address )
	{
		std::stringstream ss;
		ss.setf( std::ios::hex, std::ios::basefield );
		ss.setf( std::ios::showbase );
		ss << "_" << ( size_t )address;
		out = ss.str( );
	}

}}

