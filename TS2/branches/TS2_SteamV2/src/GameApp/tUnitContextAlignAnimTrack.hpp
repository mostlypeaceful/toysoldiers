#ifndef __tUnitContextAlignAnimTrack__
#define __tUnitContextAlignAnimTrack__
#include "tUnitPathAnimTrack.hpp"

namespace Sig
{
	struct tUnitContextAlignDesc
	{
		f32			mBlendIn;
		f32			mBlendOut;
		f32			mTimeScale;
		f32			mBlendScale;
		tUnitPath*	mUnitPath;
		f32			mRotateSpeed;

		tUnitContextAlignDesc( )
			: mBlendIn(0.2f)
			, mBlendOut(0.2f)
			, mTimeScale(1.0f)
			, mBlendScale(1.0f)
			, mUnitPath( NULL )
			, mRotateSpeed(1.0f)
		{
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class base_export tUnitContextAlignAnim : public tUnitPathAnimTrack
	{
		define_dynamic_cast( tUnitContextAlignAnim, tUnitPathAnimTrack );

	public:
		tUnitContextAlignAnim( const tUnitContextAlignDesc& desc );

		virtual const Math::tVec3f fGetTarget( );
	};
}

#endif//__tUnitContextAlignAnimTrack__

