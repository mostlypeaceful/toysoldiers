#include "ToolsPch.hpp"
#include "tHlslVariable.hpp"
#include "tHlslWriter.hpp"

namespace Sig { namespace HlslGen
{
	using namespace Gfx;
	namespace
	{
		struct tSortGlobals
		{
			inline b32 operator()( const tHlslVariableConstPtr& aVar, const tHlslVariableConstPtr& bVar ) const
			{
				const tHlslVariable::tSemantic& a = aVar->fSemantic( );
				const tHlslVariable::tSemantic& b = bVar->fSemantic( );
				if( a.mRegisterType == b.mRegisterType )
					return a.mRegisterIndex < b.mRegisterIndex;
				return a.mRegisterType < b.mRegisterType;
			}
		};

		static void fSafeVariableName( std::string& makeSafe )
		{
			for( u32 i = 0; i < makeSafe.length( ); ++i )
			{
				b32 invalid = false;
				if( i == 0 )	invalid = !isalpha( makeSafe[ i ] ) && makeSafe[ i ] != '_';
				else			invalid = !isalnum( makeSafe[ i ] ) && makeSafe[ i ] != '_';
				if( invalid )
					makeSafe[ i ] = '_';
			}
		}
	}


	void tShaderRequirements::fSortGlobals( )
	{
		std::sort( mGlobals.fBegin( ), mGlobals.fEnd( ), tSortGlobals( ) );
	}
	void tShaderRequirements::fComputeVertexFormat( tVertexFormat& vtxFormat, b32 instanced ) const
	{
		tGrowableArray<tVertexElement> vertexElems;
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			tVertexElement ve = mInputs[ i ]->fVertexElement( );
			if( ve.mSemantic != tVertexElement::cSemanticInvalid )
				vertexElems.fPushBack( ve );
		}

		if( instanced )
		{
			//make sure our vertex format isn't taken up by something fancy
			// that would prevent instanced rendering
			for( u32 i = 0; i < mInputs.fCount( ); ++i )
			{
				const tVertexElement& ve = mInputs[ i ]->fVertexElement( );
				sigassert( ve.mStreamIndex != tVertexFormat::cStreamIndex_Instanced );
				if( ve.mStreamIndex == tVertexFormat::cStreamIndex_Instanced )
					return; //we need this stream for instance data so we can't possibly do this shader
				if( ve.mSemantic == tVertexElement::cSemanticPosition )
				{
					sigassert( ve.mSemanticIndex == 0 );
					if( ve.mSemanticIndex == 0 )
						return; //we use position1, 2 and 3 for pulling our per-instance matrix
				}
			}

			//add matrix (3 float4s) to stream 2
			vertexElems.fPushBack( tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_4, 1, tVertexFormat::cStreamIndex_Instanced ) );
			vertexElems.fPushBack( tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_4, 2, tVertexFormat::cStreamIndex_Instanced ) );
			vertexElems.fPushBack( tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_4, 3, tVertexFormat::cStreamIndex_Instanced ) );
		}

		vtxFormat.fReset( vertexElems.fBegin( ), vertexElems.fCount( ) );
	}
	void tShaderRequirements::fComputeMaterialGlue( tShaderOutputBase& shaderOutput ) const
	{
		for( u32 i = 0; i < mGlobals.fCount( ); ++i )
		{
			Gfx::tShadeMaterialGluePtr glue = mGlobals[ i ]->fMaterialGlue( );
			if( !glue )
				continue;
			shaderOutput.fAddMaterialGlue( glue );
		}
	}

	tHlslVariable* tHlslVariable::fMakeVectorFloat( const std::string& name, tScope scope, u32 dimensionX, const tSemantic& semantic, u32 arrayCount )
	{
		return new tHlslVariable( name, scope, dimensionX, 1, semantic, cFloat, arrayCount );
	}

	tHlslVariable* tHlslVariable::fMakeVectorInt( const std::string& name, tScope scope, u32 dimensionX, const tSemantic& semantic, u32 arrayCount )
	{
		return new tHlslVariable( name, scope, dimensionX, 1, semantic, cInt, arrayCount );
	}

	tHlslVariable* tHlslVariable::fMakeMatrixFloat( const std::string& name, tScope scope, u32 dimensionX, u32 dimensionY, const tSemantic& semantic, u32 arrayCount )
	{
		return new tHlslVariable( name, scope, dimensionX, dimensionY, semantic, cFloat, arrayCount );
	}

	tHlslVariable* tHlslVariable::fMakeSampler2D( const std::string& name, u32 registerIndex, u32 arrayCount )
	{
		std::stringstream realNameSs;
		realNameSs << name << "_" << registerIndex;
		std::string realName = realNameSs.str( );
		fSafeVariableName( realName );
		return new tHlslVariable( realName, cGlobal, 0, 0, tSemantic( "s", registerIndex ), cSampler2D, arrayCount );
	}
	tHlslVariable* tHlslVariable::fMakeSampler3D( const std::string& name, u32 registerIndex, u32 arrayCount )
	{
		std::stringstream realNameSs;
		realNameSs << name << "_" << registerIndex;
		std::string realName = realNameSs.str( );
		fSafeVariableName( realName );
		return new tHlslVariable( realName, cGlobal, 0, 0, tSemantic( "s", registerIndex ), cSampler3D, arrayCount );
	}
	tHlslVariable* tHlslVariable::fMakeSamplerCube( const std::string& name, u32 registerIndex, u32 arrayCount )
	{
		std::stringstream realNameSs;
		realNameSs << name << "_" << registerIndex;
		std::string realName = realNameSs.str( );
		fSafeVariableName( realName );
		return new tHlslVariable( realName, cGlobal, 0, 0, tSemantic( "s", registerIndex ), cSamplerCube, arrayCount );
	}

	tHlslVariable* tHlslVariable::fMakeGlobalVector( const std::string& name, u32 registerIndex, u32 dimensionX, u32 arrayCount )
	{
		std::stringstream realNameSs;
		realNameSs << name << "_" << registerIndex;
		std::string realName = realNameSs.str( );
		fSafeVariableName( realName );
		return new tHlslVariable( realName, cGlobal, dimensionX, 1, tSemantic( "c", registerIndex ), cFloat, arrayCount );
	}

	tHlslVariable* tHlslVariable::fMakeTempVector( const std::string& name, u32 tempIndex, u32 dimensionX, u32 arrayCount )
	{
		std::stringstream realNameSs;
		realNameSs << name << "_" << tempIndex;
		std::string realName = realNameSs.str( );
		fSafeVariableName( realName );
		return new tHlslVariable( realName, cTemp, dimensionX, 1, tSemantic( ), cFloat, arrayCount );
	}

	std::string tHlslVariable::tSemantic::fDeclaration( ) const
	{
		std::stringstream ss;
		if( mSemanticText.length( ) > 0 )
			ss << " : " << mSemanticText;
		if( mRegisterType.length( ) > 0 )
		{
			sigassert( mRegisterIndex >= 0 );
			ss << " : register( " << mRegisterType << mRegisterIndex << " )";
		}
		return ss.str( );
	}

	tHlslVariable::tHlslVariable( 
		const std::string& name, 
		tScope scope, 
		u32 dimensionX, 
		u32 dimensionY, 
		const tSemantic& semantic, 
		tType type, 
		u32 arrayCount )
		: mName( name )
		, mSemantic( semantic )
		, mScope( scope )
		, mType( type )
		, mDimensionX( dimensionX )
		, mDimensionY( dimensionY )
		, mArrayCount( arrayCount )
	{
		sigassert( !fIsNumeric( ) || ( mDimensionX >= 1 && mDimensionY >= 1 ) );
	}

	std::string tHlslVariable::fTypeString( ) const
	{
		std::stringstream ss;
		switch( mType )
		{
		case cFloat: ss << "float"; break;
		case cHalf: ss << "half"; break;
		case cInt: ss << "int"; break;
		case cSampler2D: ss << "sampler2D"; break;
		case cSampler3D: ss << "sampler3D"; break;
		case cSamplerCube: ss << "samplerCUBE"; break;
		default: sigassert( !"unrecognized type" ); break;
		}

		if( !fIsSampler( ) )
		{
			if( mDimensionY > 1 )
				ss << mDimensionY << "x";
			if( mDimensionY > 1 || mDimensionX > 1 )
				ss << mDimensionX;
		}

		return ss.str( );
	}

	std::string tHlslVariable::fCastValueToType( f32 scalarValue ) const
	{
		sigassert( fIsNumeric( ) );

		std::stringstream ss;

		if( mDimensionX > 1 )
			ss << fTypeString( ) << "( ";

		for( u32 i = 0; i < mDimensionX; ++i )
		{
			ss << scalarValue;
			if( i < mDimensionX - 1 )
				ss << ", ";
		}

		if( mDimensionX > 1 )
			ss << " )";

		return ss.str( );
	}

	std::string tHlslVariable::fDeclaration( const std::string& extraText ) const
	{
		std::stringstream ss;

		switch( mScope )
		{
		case cInput: ss << "in "; break;
		case cOutput: ss << "out "; break;
		case cInputOutput: ss << "inout "; break;
		}

		ss << fTypeString( ) << " " << mName;

		if( mArrayCount > 0 )
			ss << "[ " << mArrayCount << " ]";

		const std::string semanticText = mSemantic.fDeclaration( );
		if( semanticText.length( ) > 0 )
			ss << semanticText;

		ss << extraText;

		return ss.str( );
	}

	std::string tHlslVariable::fSwizzle( const std::string& swizzleText, const std::string& extraText ) const
	{
		std::stringstream ss;

		ss << mName << "." << swizzleText << extraText;

		return ss.str( );
	}

	std::string tHlslVariable::fSwizzle( const tHlslVariable& matchThisVar, const std::string& extraText ) const
	{
		sigassert( mDimensionY == 1 && matchThisVar.mDimensionY == 1 );
		sigassert( mDimensionX >= 1 && mDimensionX <= 4 );
		sigassert( matchThisVar.mDimensionX >= 1 && matchThisVar.mDimensionX <= 4 );
		sigassert( fIsNumeric( ) && matchThisVar.fIsNumeric( ) );

		std::string o;

		switch( matchThisVar.mDimensionX )
		{
		case 1:
			switch( mDimensionX )
			{
			case 1: o = fName( ); break;
			case 2: o = fSwizzle( "x" ); break;
			case 3: o = fSwizzle( "x" ); break;
			case 4: o = fSwizzle( "x" ); break;
			default: sigassert( !"invalid case" ); break;
			}
			break;
		case 2:
			switch( mDimensionX )
			{
			case 1: o = matchThisVar.fTypeString( ) + "( " + fName( ) + ", " + fName( ) + " )"; break;
			case 2: o = fName( ); break;
			case 3: o = fSwizzle( "xy" ); break;
			case 4: o = fSwizzle( "xy" ); break;
			default: sigassert( !"invalid case" ); break;
			}
			break;
		case 3:
			switch( mDimensionX )
			{
			case 1: o = matchThisVar.fTypeString( ) + "( " + fName( ) + ", " + fName( ) + ", " + fName( ) + " )"; break;
			case 2: o = matchThisVar.fTypeString( ) + "( " + fSwizzle( "xy" ) + ", 0 )"; break;
			case 3: o = fName( ); break;
			case 4: o = fSwizzle( "xyz" ); break;
			default: sigassert( !"invalid case" ); break;
			}
			break;
		case 4:
			switch( mDimensionX )
			{
			case 1: o = matchThisVar.fTypeString( ) + "( " + fName( ) + ", " + fName( ) + ", " + fName( ) + ", " + fName( ) + " )"; break;
			case 2: o = matchThisVar.fTypeString( ) + "( " + fSwizzle( "xy" ) + ", 0, 1 )"; break;
			case 3: o = matchThisVar.fTypeString( ) + "( " + fSwizzle( "xyz" ) + ", 1 )"; break;
			case 4: o = fName( ); break;
			default: sigassert( !"invalid case" ); break;
			}
			break;
		default:
			sigassert( !"invalid case" );
			break;
		}

		return o;
	}

	std::string tHlslVariable::fSwizzle( u32 matchThisDimension, const std::string& extraText ) const
	{
		tHlslVariable matchThisVar( "swizzler", mScope, matchThisDimension, mDimensionY, mSemantic, mType, mArrayCount );
		return fSwizzle( matchThisVar, extraText );
	}

	std::string tHlslVariable::fIndexArray( const std::string& indexText, const std::string& extraText ) const
	{
		sigassert( mArrayCount > 0 );

		std::stringstream ss;

		ss << mName << "[ " << indexText << " ]" << extraText;

		return ss.str( );
	}

	std::string tHlslVariable::fIndexArrayAndSwizzle( const std::string& indexText, const std::string& swizzleText, const std::string& extraText ) const
	{
		sigassert( mArrayCount > 0 );

		std::stringstream ss;

		ss << mName << "[ " << indexText << " ]." << swizzleText << extraText;

		return ss.str( );
	}


	tHlslStructVariable::tHlslStructVariable(
		const std::string& name,
		tScope scope,
		const std::string& typeName,
		const tSemantic& semantic,
		const tDynamicArray<tHlslVariableConstPtr>& members,
		u32 arrayCount )
		: tHlslVariable( name, scope, 0, 0, semantic, cStruct, arrayCount )
		, mTypeName( typeName )
		, mMembers( members )
	{
	}
	u32 tHlslStructVariable::fRegisterCount( ) const
	{
		u32 o = 0;
		for( u32 i = 0; i < mMembers.fCount( ); ++i )
			o += mMembers[ i ]->fRegisterCount( );
		return fMax( fArrayCount( ), 1u ) * o;
	}
	void tHlslStructVariable::fDeclareType( tHlslWriter& writer ) const
	{
		// see if type has already been declared
		if( writer.fDeclareType( mTypeName ) )
			return;

		for( u32 i = 0; i < mMembers.fCount( ); ++i )
			mMembers[ i ]->fDeclareType( writer );

		writer.fBeginLine( true ) << "// declare struct" << std::endl;
		writer.fBeginLine( ) << "struct " << mTypeName << std::endl;
		writer.fBeginLine( ) << "{" << std::endl;
		writer.fPushTab( );
		for( u32 i = 0; i < mMembers.fCount( ); ++i )
			writer.fBeginLine( ) << mMembers[ i ]->fDeclaration( ) << ";" << std::endl;
		writer.fPopTab( );
		writer.fBeginLine( ) << "};" << std::endl;
	}
	std::string	tHlslStructVariable::fTypeString( ) const
	{
		return mTypeName;
	}

	tHlslVariableConstPtr tVariableRegistry::fFindMostAppropriateVariable( u32 ithVariable, tHlslPlatformId pid )
	{
		if( ithVariable >= mVariables.fCount( ) )
			return tHlslVariableConstPtr( );
		if( mVariables[ ithVariable ][ pid ] )
			return tHlslVariableConstPtr( mVariables[ ithVariable ][ pid ].fGetRawPtr( ) );
		if( mVariables[ ithVariable ][ cPidDefault ] )
			return tHlslVariableConstPtr( mVariables[ ithVariable ][ cPidDefault ].fGetRawPtr( ) );
		return tHlslVariableConstPtr( );
	}

}}

