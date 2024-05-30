#ifndef __tSoundBank__
#define __tSoundBank__

namespace Sig { namespace Audio
{
	// tSoundBank
	//   If user wishes to unload the bank then just fRelease() it. Once 
	//    all references are released it will automatically unload.
	class tSoundBank : public tUncopyable, public tThreadSafeRefCounter
	{
	public:
		virtual ~tSoundBank( ) { }
		virtual const tFilePathPtr& fPath( ) const = 0;
	};
	typedef tRefCounterPtr< tSoundBank > tSoundBankPtr;


	// fLoadSoundBankById
	//  Id looks up in the SoundBankMapping table to generate the path
	tSoundBankPtr fLoadSoundBankById( const tStringPtr& id );
	tSoundBankPtr fLoadSoundBankAsyncById( const tStringPtr& id );

	// fLoadSoundBankByPath
	//  Only use this if you know the explicit path to the sound bank to load.
	tSoundBankPtr fLoadSoundBankByPath( const tFilePathPtr& id );
	tSoundBankPtr fLoadSoundBankAsyncByPath( const tFilePathPtr& id );


	// fUpdateBanks
	//  Tick this every frame to update loads/unloads of sound banks
	void fUpdateBanks( );

	// fAnyLoading
	//	Returns true if any banks are currently loading
	b32 fAnyBanksLoading( );
}}

#endif//__tSoundBank__
