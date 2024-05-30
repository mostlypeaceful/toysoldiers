#ifndef __tFingerDrag__
#define __tFingerDrag__

#include "tFixedBitArray.hpp"
#include "Input/tTouch.hpp"

namespace Sig { namespace Input {
	class tMouse;

	/// \brief Represents a common finger drag value
	class base_export tFingerDrag
	{
	public:
		tFingerDrag();
		// \brief Returns if successfully parsed
		b32 fLoadFromString( const char* s );
		Math::tVec2f fEvaluate( const tTouch& touch ) const;
		Math::tVec2f fEvaluate( const tMouse& mouse ) const;
		b32 fIsAbsolute() const { return mType == cTypeAbsolute; }
	private:
		enum tType { cTypeAbsolute, cTypeRelative };

		tFixedBitArray<tTouch::cFingerCount,u32> mFingerFilter;
		tType mType;
		b8 mFlipX, mFlipY, mPad0, mPad1;
	};
}}

#endif //ndef __tFingerDrag__
