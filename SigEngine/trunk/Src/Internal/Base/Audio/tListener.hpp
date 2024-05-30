#ifndef __tListener__
#define __tListener__

namespace Sig { 

	class tAudioLogic;
	
namespace Audio
{

	class base_export tListener : public tRefCounter
	{
	public:
		tListener( const char *debugName = NULL );
		~tListener( );

		void fSetTransform( const Math::tMat3f& xform, u32 index );

		const Math::tMat3f& fLastXForm( ) const { return mLastXForm; }

		void fPushVolume( tAudioLogic& volume );
		void fPopVolume( tAudioLogic& volume );
		void fClearVolumeStack( );

	private:
		Math::tMat3f mLastXForm;

		tGrowableArray<tEntityPtr> mVolumeStack;
	};

	typedef tRefCounterPtr<tListener> tListenerPtr;

}}


#endif//__tListener__

