#ifndef __tToolAnimatedLightData__
#define __tToolAnimatedLightData__
#include "FX/tAnimatedLightEntity.hpp"


namespace Sig
{
	class tools_export tToolAnimatedLightData : public tRefCounter
	{
	public:

		tToolAnimatedLightData( tToolAnimatedLightData* rhs );
		tToolAnimatedLightData( );
		~tToolAnimatedLightData( );

		void fUpdate( );

		FX::tBinaryAnimatedLightData fCreateBinaryData( ) const;

	public:
		static const tStringPtr mGraphNames[ FX::cLightGraphCount ];
		static const tStringPtr mFlagNames[ FX::cLightFlagCount ];
		static const Math::tVec3f mColors[ FX::cLightGraphCount ];

		u32 mFlags;
		tGrowableArray< FX::tGraphPtr > mGraphs;


		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{
			s( "Graphs", mGraphs );
			s( "Flags", mFlags);
		}
	};

	typedef tRefCounterPtr< tToolAnimatedLightData > tToolAnimatedLightDataPtr;
}

#endif // __tToolAnimatedLightData__