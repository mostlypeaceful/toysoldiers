#ifndef __tFingerTrigger__
#define __tFingerTrigger__

#include "tFixedBitArray.hpp"
#include "Input/tTouch.hpp"

namespace Sig { namespace Input {
	class tMouse;

	/// \brief Represents a common finger event/condition of some sort (finger down, etc)
	class base_export tFingerTrigger
	{
	public:
		tFingerTrigger();
		/// \brief Returns if successfully parsed
		b32 fLoadFromString( const char* s );

		/// N.B.: value is input AND output
		/// the input of value should be the last frame's value for "toggle" to work correctly
		void fUpdate( b32& value, const tTouch& touch ) const;
		void fUpdate( b32& value, const tMouse& mouse ) const;

	private:
		enum tType { cTypeDown, cTypeHeld, cTypeUp, cTypeToggle, cTypeDragThreshold };

		tFixedBitArray<tTouch::cFingerCount,u32> mFingerFilter;
		tType mType;
	};
}}

#endif //ndef __tFingerTrigger__
