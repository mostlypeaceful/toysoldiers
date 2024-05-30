#ifndef __tResourceLoadList2__
#define __tResourceLoadList2__

namespace Sig
{
	///
	/// \brief The simpler cousin of tResourceLoadList that's easier to understand and use, and intended to eventually replace it.
	///		No callbacks, no virtuals, no inheriting classes.  Final destination.
	///		Fewer possible states of the member lists.
	class tResourceLoadList2 : public tRefCounter
	{
		tGrowableArray< tResourcePtr > mResources;		///< The resources of the list.  These are either loading, loaded, or failed to load.

	public:

		void fAdd( const tResourceId& rid );			///< Add and start loading (if not already) "rid" to this list.

		b32 fDone( ) const;								///< Returns true if every added resource has completed loading successfully (or if this list is empty)
		f32 fPercentComplete( ) const;					///< Returns in the [0..1] range depending on how many individual resources of the list have completely loaded.  Returns 1 if empty.
		void fCount( u32& total, u32& complete ) const;	///< Gets the total elements in mResources, and how many of them have completely loaded.
	};
}

#endif//__tResourceLoadList2__
