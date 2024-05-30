#ifndef __tAnimatedLightEntity__
#define __tAnimatedLightEntity__
#include "tEntityDef.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig { namespace FX
{
	enum tLightGraphs
	{
		cLightColorGraph,
		cIntensityGraph,
		cInnerRadiusGraph,
		cOuterRadiusGraph,
		cTranslationGraph,
		cLightGraphCount
	};
	enum tLightFlags
	{
		cSomeFlag = ( 1 << 0 ),
		cLightFlagCount,
	};

	class base_export tBinaryAnimatedLightData
	{
		declare_reflector( );
	public:

		tBinaryAnimatedLightData( );
		tBinaryAnimatedLightData( tNoOpTag );
		~tBinaryAnimatedLightData( );

		template< class GraphType >
		typename GraphType::tType fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const GraphType* >( mGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ); 
		}

		template<>
		typename f32 fSampleGraph<tBinaryF32Graph>( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const tBinaryF32Graph* >( mGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ).x; 
		}

		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mGraphs;
		u32														mFlags; // This is unused for now but I'm leaving it for the future.
	};

	///
	/// \class tAnimatedLightDef
	/// \brief The def for the associated entity.
	class base_export tAnimatedLightDef : public Gfx::tLightEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tAnimatedLightDef, 0xBB7ACD8E );

	public:
		tBinaryAnimatedLightData mBinaryData;

	public:
		tAnimatedLightDef( );
		tAnimatedLightDef( tNoOpTag );
		~tAnimatedLightDef( );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};


	///
	/// \class tAnimatedLightEntity
	/// \brief A simple entity that provides animation logic and editing for tLightEntity.
	class base_export tAnimatedLightEntity : public Gfx::tLightEntity 
	{
		define_dynamic_cast( tAnimatedLightEntity, tLightEntity );
	private:

		tBinaryAnimatedLightData mData;

		f32 mTime;

		tMersenneGenerator mRandomGenerator; 

		Math::tVec3f mCurrentColor;
		f32 mCurrentIntensity;
		f32 mCurrentInnerRadius;
		f32 mCurrentOuterRadius;

	public:
		tAnimatedLightEntity( );
		tAnimatedLightEntity( const Gfx::tLight& lightData );
		tAnimatedLightEntity( const tAnimatedLightDef* def );
		virtual ~tAnimatedLightEntity( );

	public:
		const tMersenneGenerator* fRandomNumberGenerator( ) const { return &mRandomGenerator; }

		u32 fFlags( ) const { return mData.mFlags; }

		//void fAddFlag( u32 flag ) { mData->fAddFlag( flag ); }
		//void fRemoveFlag( u32 flag ) { mData->fRemoveFlag( flag ); }
		//b32	fHasFlag( u32 flag ) const { return mData->fHasFlag( flag ); }

		tBinaryAnimatedLightData fGetData( ) { return mData; }
		void fSetData( tBinaryAnimatedLightData& data ) { mData = data; }
		void fUpdateGraphValues( const f32 delta );

		Math::tVec3f fCurrentColor( ) const { return mCurrentColor; }
		f32 fCurrentIntensity( ) const { return mCurrentIntensity; }
		f32 fCurrentInnerRadius( ) const { return mCurrentInnerRadius; }
		f32 fCurrentOuterRadius( ) const { return mCurrentOuterRadius; }

	private:
		void fCommonCtor( );
	};

	typedef tRefCounterPtr< tAnimatedLightEntity > tAnimatedLightEntityPtr;
}}

#endif // __tAnimatedLightEntity__
