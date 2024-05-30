#ifndef __tLight__
#define __tLight__
#include "tMaterial.hpp"

namespace Sig { namespace Gfx
{

	class base_export tLight
	{
		declare_reflector( );
	public:
		enum tLightType
		{
			cLightTypeDirection,
			cLightTypePoint,
			cLightTypeSpot,
			cLightTypeCount
		};

		enum tColorType
		{
			cColorTypeFront,
			cColorTypeRim,
			cColorTypeBack,
			cColorTypeAmbient,
			cColorTypeCount
		};

	protected:
		tFixedArray< Math::tNoOpVec4f, cColorTypeCount >mColors;
		Math::tVec2f									mRadii;
		Math::tVec2f									mAngles;
		tEnum<tLightType,u32>							mLightType;

	public:
		tLight( );
		tLight( tNoOpTag );
		tLightType fLightType( ) const { return mLightType; }
		void fSetLightType( tLightType type ) { mLightType = type; }
		const Math::tVec4f* fColorsBegin( ) const { return mColors.fBegin( ); }
		Math::tVec4f&		fColor( tColorType ct ) { return mColors[ ct ]; }
		const Math::tVec4f& fColor( tColorType ct ) const { return mColors[ ct ]; }
		Math::tVec3f		fColor3( tColorType ct ) { return Math::tVec3f( mColors[ ct ].x, mColors[ ct ].y, mColors[ ct ].z ); }
		Math::tVec2f&		fRadii( ) { return mRadii; }
		const Math::tVec2f& fRadii( ) const { return mRadii; }
		const Math::tVec2f& fAngles( ) const { return mAngles; }

		f32 fDirectionalCoefficient( ) const { return mLightType == cLightTypeDirection ? 1.f : 0.f; }
		f32 fPositionalCoefficient( ) const { return mLightType == cLightTypePoint ? 1.f : 0.f; }

		void fSetTypeDirection( );
		void fSetTypePoint( const Math::tVec2f& radii );
		void fSetTypeSpot( const Math::tVec2f& radii, const Math::tVec2f& angles );
	};

	class base_export tRimLightShaderConstants
	{
	public:
		Math::tVec4f					mLightDir;
		Math::tVec4f					mLightColor;

		tRimLightShaderConstants( ) { fZeroOut( this ); }
	};

	struct base_export tLightShaderConstants
	{
		Math::tVec4f					mLightDir;
		Math::tVec4f					mLightPos;
		Math::tVec4f					mLightAttenuation;
		Math::tVec4f					mLightAngles;
		Math::tVec4f					mLightAmbient;
		Math::tVec4f					mLightColor0;
		Math::tVec4f					mLightColor1;
		Math::tVec4f					mLightColor2;

		tLightShaderConstants( ) { fZeroOut( this ); }
	};

	class base_export tLightShaderConstantsArray
	{
	public:
		static const u32 cShaderSlotsPerLight = ( sizeof( tLightShaderConstants ) / ( 4 * sizeof( float ) ) );
	public:
		tFixedArray<tLightShaderConstants, tMaterial::cMaxLights>	mLightArray;
		u32															mLightCount;
	public:
		tLightShaderConstantsArray( ) : mLightCount( 0 ) { }
		inline tLightShaderConstants&		operator[]( u32 i )			{ return mLightArray[ i ]; }
		inline const tLightShaderConstants& operator[]( u32 i ) const	{ return mLightArray[ i ]; }
	};

}}

#endif//__tLight__

