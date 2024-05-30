#ifndef __tLight__
#define __tLight__
#include "tMaterial.hpp"

namespace Sig {
	struct tSceneGraphDefaultLight;
}

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
			cLightTypeCount
		};

		// Interesting note on ColorType: these are call commented out for
		// getting carried over from editable properties in tEditableLightEntity ln 47.
		// It's unclear if these are even usable when specified through the editor.
		enum tColorType
		{
			cColorTypeFront,
			cColorTypeRim,
			cColorTypeBack,
			cColorTypeAmbient,
			cColorTypeCount
		};

	protected:
		tFixedArray< Math::tVec4f, cColorTypeCount >mColors;
		Math::tVec2f									mRadii;
		tEnum<tLightType,u32>							mLightType;

	public:
		tLight( );
		tLight( tNoOpTag );
		tLightType fLightType( ) const { return mLightType; }
		void fSetLightType( tLightType type ) { mLightType = type; }
		const Math::tVec4f* fColorsBegin( ) const { return mColors.fBegin( ); }
		Math::tVec4f&		fColor( tColorType ct ) { return mColors[ ct ]; }
		void				fSetColors( const Math::tVec3f& front, const Math::tVec3f& rim, const Math::tVec3f& back, const Math::tVec3f& ambient );
		const Math::tVec4f& fColor( tColorType ct ) const { sigassert( ct < cColorTypeCount ); return mColors[ ct ]; }
		Math::tVec3f		fColor3( tColorType ct ) const { return Math::tVec3f( mColors[ ct ].x, mColors[ ct ].y, mColors[ ct ].z ); }
		Math::tVec2f&		fRadii( ) { return mRadii; }
		const Math::tVec2f& fRadii( ) const { return mRadii; }

		f32 fDirectionalCoefficient( ) const { return mLightType == cLightTypeDirection ? 1.f : 0.f; }
		f32 fPositionalCoefficient( ) const { return mLightType == cLightTypePoint ? 1.f : 0.f; }

		void fSetTypeDirection( );
		void fSetTypePoint( const Math::tVec2f& radii );

		void fConvertFrom( tSceneGraphDefaultLight* const light );

	private:
		void fSetColorForScript( u32 type, const Math::tVec4f& val );
		Math::tVec4f fGetColorForScript( u32 type );
		Math::tVec2f fGetRadiiForScript( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
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
		Math::tVec4f					mLightAmbient;
		Math::tVec4f					mLightFront;
		Math::tVec4f					mLightSurround;
		Math::tVec4f					mLightBack;

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

