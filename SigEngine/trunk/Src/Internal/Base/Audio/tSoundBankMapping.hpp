#ifndef __tSoundBankMapping__
#define __tSoundBankMapping__

namespace Sig
{
	class tApplication;
	class tResourceLoadList2;
}

namespace Sig { namespace Audio { namespace SoundBankMapping
{
	void fSetup( tApplication& app, tResourceLoadList2* preLoad );
	void fReset( );
	tFilePathPtr fMap( const tStringPtr& id );
}}}

#endif//__tSoundBankMapping__
