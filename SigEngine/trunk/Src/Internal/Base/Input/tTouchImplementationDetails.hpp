#ifndef __tTouchImplementationDetails__
#define __tTouchImplementationDetails__

namespace Sig { namespace Input { namespace ImplDetails {
	namespace {
		const char* const cValidAfterFlag = "\t\r\n 0123456789";
		const f32 cDragStartThreshold = 5.0f;
	}

	b32 fCheckFlag( const char*& s, const char* flagName, b32& flag );

}}} // Sig::Input::ImplDetails

#endif //ndef __tTouchImplementationDetails__
