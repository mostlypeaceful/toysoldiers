#ifndef __tSolidColorGrid__
#define __tSolidColorGrid__
#include "tSolidColorLines.hpp"
#include "Gfx/tDevice.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief Lightweight wrapper around tSolidColorLines, can generate
	/// a nice 2D grid of lines with major/minor demarcations.
	class base_export tSolidColorGrid : public tSolidColorLines, public tDeviceResource
	{
		u32 mGridLinesPerHalfAxis;
		u32 mMinorIncrement;
		u32 mMajorIncrement;
		Math::tVec3f mLastZ;
		Math::tVec3f mLastX;
	public:
		tSolidColorGrid( );
		virtual void fOnDeviceLost( tDevice* device ) { }
		virtual void fOnDeviceReset( tDevice* device );
		void fResetDeviceObjects( 
			const tDevicePtr& device,
			const tMaterialPtr& material, 
			const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const tIndexBufferVRamAllocatorPtr& indexAllocator );
		void fGenerate( u32 gridLinesPerHalfAxis, u32 inc = 1, u32 major = 10, Math::tVec3f xAxis = Math::tVec3f( 1.f, 0.f, 0.f ), Math::tVec3f zAxis = Math::tVec3f( 0.f, 0.f, 1.f ) );

		inline u32 fGridLinesPerHalfAxis( ) const { return mGridLinesPerHalfAxis; }
		inline u32 fMinorIncrement( ) const { return mMinorIncrement; }
		inline u32 fMajorIncrement( ) const { return mMajorIncrement; }

		inline void fGetBounds( Math::tVec3f& min, Math::tVec3f& max )
		{
			max.y = min.y = 0.f;
			max.x = max.z = ( f32 )mGridLinesPerHalfAxis * mMinorIncrement;			
			min.x = min.z = -max.x;
		}
	};

	typedef tRefCounterPtr<tSolidColorGrid> tSolidColorGridPtr;
}}

#endif//__tSolidColorGrid__
