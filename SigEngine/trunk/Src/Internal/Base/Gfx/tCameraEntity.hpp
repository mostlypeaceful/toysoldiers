#ifndef __tCameraEntity__
#define __tCameraEntity__
#include "tEntityDef.hpp"

namespace Sig
{
	class tLensProperties
	{
		declare_reflector( );
	public:

		// FOV is stored in the horizontal and in radians.
		f32 mFOV;

		tLensProperties( ) 
			: mFOV( Math::cPiOver2 )
		{ }

		tLensProperties( tNoOpTag )
		{ }
	
		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "fov", mFOV );
		}
	};

	class base_export tCameraEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tCameraEntityDef, 0x49EC69DE );
	public:
		tLensProperties mLensProperties;

	public:
		tCameraEntityDef( );
		tCameraEntityDef( tNoOpTag );
		~tCameraEntityDef( );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};

	class base_export tCameraEntity : public tEntity
	{
		define_dynamic_cast( tCameraEntity, tEntity );
	private:
		const tCameraEntityDef* mEntityDef;
	public:
		explicit tCameraEntity( const Math::tMat3f& objectToWorld, const tCameraEntityDef& def  );
		virtual void fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton );

		tLensProperties mLensProperties;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tCameraEntity );

}

#endif//__tCameraEntity__
