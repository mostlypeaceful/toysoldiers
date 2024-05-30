#ifndef __tHlslVariable__
#define __tHlslVariable__
#include "tHlslOutput.hpp"

namespace Sig { namespace HlslGen
{
	class tHlslWriter;

	class tHlslVariable;
	typedef tRefCounterPtr<tHlslVariable> tHlslVariablePtr;
	typedef tRefCounterPtr<const tHlslVariable> tHlslVariableConstPtr;

	struct tools_export tShaderRequirements
	{
		u32 mNumLights; // for pixel shaders only
		u32 mNextConstantRegister;
		u32 mNextSamplerRegister;

		tGrowableArray<tHlslVariableConstPtr> mGlobals;
		tGrowableArray<tHlslVariableConstPtr> mInputs;
		tGrowableArray<tHlslVariableConstPtr> mCommon;
		tGrowableArray<tHlslVariableConstPtr> mOutputs;

		tShaderRequirements( ) : mNumLights( 0 ), mNextConstantRegister( 0 ), mNextSamplerRegister( 0 ) { }
		void fSortGlobals( );
		void fComputeVertexFormat( Gfx::tVertexFormat& vtxFormat ) const;
		void fComputeMaterialGlue( tShaderOutputBase& shaderOutput ) const;
	};

	class tools_export tHlslVariable : public tRefCounter
	{
	public:
		enum tScope
		{
			cTemp,
			cInput,
			cOutput,
			cInputOutput,
			cGlobal,
			cMember,
		};
		enum tType
		{
			cFloat,
			cHalf,
			cInt,
			cSampler2D,
			cSampler3D,
			cSamplerCube,
			cStruct,
		};
		struct tSemantic
		{
			std::string mRegisterType;
			s32			mRegisterIndex;
			std::string mSemanticText;

			tSemantic( ) : mRegisterIndex( -1 ) { }
			tSemantic( const char* semanticText )
				: mRegisterIndex( -1 ), mSemanticText( semanticText ) { }
			explicit tSemantic( const std::string& semanticText )
				: mRegisterIndex( -1 ), mSemanticText( semanticText ) { }
			tSemantic( const std::string& registerType, u32 regIndex )
				: mRegisterType( registerType ), mRegisterIndex( regIndex ) { }
			std::string fDeclaration( ) const;
		};
	private:
		std::string mName;
		tSemantic	mSemantic;
		tScope		mScope;
		tType		mType;
		u32			mDimensionX; // e.g., for a float4 this would be 4
		u32			mDimensionY; // e.g., for a float3x4 this would be 3
		u32			mArrayCount;

		Gfx::tVertexElement			mVtxElem;
		Gfx::tShadeMaterialGluePtr	mMtlGlue;

	public:
		static tHlslVariable* fMakeVectorFloat( const std::string& name, tScope scope = cTemp, u32 dimensionX = 4, const tSemantic& semantic = tSemantic( ), u32 arrayCount = 0 );
		static tHlslVariable* fMakeVectorInt( const std::string& name, tScope scope = cTemp, u32 dimensionX = 4, const tSemantic& semantic = tSemantic( ), u32 arrayCount = 0 );
		static tHlslVariable* fMakeMatrixFloat( const std::string& name, tScope scope = cTemp, u32 dimensionX = 4, u32 dimensionY = 3, const tSemantic& semantic = tSemantic( ), u32 arrayCount = 0 );
		static tHlslVariable* fMakeSampler2D( const std::string& name, u32 registerIndex, u32 arrayCount = 0 );
		static tHlslVariable* fMakeSampler3D( const std::string& name, u32 registerIndex, u32 arrayCount = 0 );
		static tHlslVariable* fMakeSamplerCube( const std::string& name, u32 registerIndex, u32 arrayCount = 0 );
		static tHlslVariable* fMakeGlobalVector( const std::string& name, u32 registerIndex, u32 dimensionX = 4, u32 arrayCount = 0 );
		static tHlslVariable* fMakeTempVector( const std::string& name, u32 tempIndex, u32 dimensionX = 4, u32 arrayCount = 0 );

	public:
		tHlslVariable( 
			const std::string& name, 
			tScope scope, 
			u32 dimensionX, 
			u32 dimensionY = 3, 
			const tSemantic& semantic = tSemantic( ), 
			tType type = cFloat, 
			u32 arrayCount = 0 );
		virtual ~tHlslVariable( ) { }

		const std::string&	fName( ) const { return mName; }
		const tSemantic&	fSemantic( ) const { return mSemantic; }
		tScope				fScope( ) const { return mScope; }
		tType				fType( ) const { return mType; }
		u32					fDimensionX( ) const { return mDimensionX; }
		u32					fDimensionY( ) const { return mDimensionY; }
		u32					fArrayCount( ) const { return mArrayCount; }
		b32					fIsNumeric( ) const { return mType == cFloat || mType == cHalf || mType == cInt; }
		b32					fIsSampler( ) const { return mType == cSampler2D || mType == cSamplerCube; }
		const Gfx::tVertexElement&			fVertexElement( ) const { return mVtxElem; }
		void								fSetVertexElement( const Gfx::tVertexElement& vtxElem ) { mVtxElem = vtxElem; }
		const Gfx::tShadeMaterialGluePtr&	fMaterialGlue( ) const { return mMtlGlue; }
		void								fSetMaterialGlue( const Gfx::tShadeMaterialGluePtr& mtlGlue ) { mMtlGlue = mtlGlue; }
		void								fSetMaterialGlue( Gfx::tShadeMaterialGlue* mtlGlue ) { mMtlGlue.fReset( mtlGlue ); }

		virtual u32			fMemberCount( ) const { return 0; }
		virtual tHlslVariableConstPtr fMember( u32 ithMember ) const { return tHlslVariableConstPtr( ); }
		virtual u32			fRegisterCount( ) const { return fMax( mArrayCount, 1u ) * mDimensionY; }
		virtual void		fDeclareType( tHlslWriter& writer ) const { }
		virtual std::string	fTypeString( ) const;
		std::string			fCastValueToType( f32 scalarValue ) const;
		std::string			fDeclaration( const std::string& extraText = "" ) const;
		std::string			fSwizzle( const std::string& swizzleText, const std::string& extraText = "" ) const;
		std::string			fSwizzle( const tHlslVariable& matchThisVar, const std::string& extraText = "" ) const;
		std::string			fSwizzle( u32 matchThisDimension, const std::string& extraText = "" ) const;
		std::string			fIndexArray( const std::string& indexText, const std::string& extraText = "" ) const;
		std::string			fIndexArrayAndSwizzle( const std::string& indexText, const std::string& swizzleText, const std::string& extraText = "" ) const;
	};

	class tools_export tHlslStructVariable : public tHlslVariable
	{
	private:
		std::string mTypeName;
		tDynamicArray<tHlslVariableConstPtr> mMembers;
	public:
		tHlslStructVariable(
			const std::string& name,
			tScope scope,
			const std::string& typeName,
			const tSemantic& semantic,
			const tDynamicArray<tHlslVariableConstPtr>& members,
			u32 arrayCount = 0 );
		virtual u32			fMemberCount( ) const { return mMembers.fCount( ); }
		virtual tHlslVariableConstPtr fMember( u32 ithMember ) const { return mMembers[ ithMember ]; }
		virtual u32			fRegisterCount( ) const;
		virtual void		fDeclareType( tHlslWriter& writer ) const;
		virtual std::string	fTypeString( ) const;
	};


	class tools_export tVariableRegistry
	{
	public:
		typedef tFixedArray<tHlslVariablePtr, cPidCount> tPlatformVariableArray;
	protected:
		tGrowableArray<tPlatformVariableArray> mVariables;
		tHlslVariableConstPtr fFindMostAppropriateVariable( u32 ithVariable, tHlslPlatformId pid );
	};

}}

#endif//__tHlslVariable__
