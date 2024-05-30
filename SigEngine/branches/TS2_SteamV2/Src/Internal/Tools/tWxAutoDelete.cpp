#include "ToolsPch.hpp"
#include "tWxAutoDelete.hpp"

namespace Sig
{
	class tWxAutoDeleteContainer : public wxClientData
	{
		tGrowableArray<tWxAutoDelete*> mDeleteThese;
	public:
		virtual ~tWxAutoDeleteContainer( );
		void fAdd( tWxAutoDelete* add );
		void fRemove( tWxAutoDelete* remove );
	};

	tWxAutoDeleteContainer::~tWxAutoDeleteContainer( )
	{
		for( u32 i = 0; i < mDeleteThese.fCount( ); ++i )
			delete mDeleteThese[i];
	}
	void tWxAutoDeleteContainer::fAdd( tWxAutoDelete* add )
	{
		mDeleteThese.fFindOrAdd( add );
	}
	void tWxAutoDeleteContainer::fRemove( tWxAutoDelete* remove )
	{
		mDeleteThese.fFindAndErase( remove );
	}

	tWxAutoDelete::tWxAutoDelete( wxWindow* parent )
		: mParent( parent )
	{
		// we add a client object array so that we can be auto-deleted by the parent
		wxClientData* clientData = parent->GetClientObject( );
		if( !clientData )
		{
			// this is the first of our custom controls allocated
			// and assigned to this parent; create the container and assign it
			clientData = new tWxAutoDeleteContainer();
			parent->SetClientObject( clientData );
		}

		// add ourself to the list of auto-cleaned-up pointers
		sigassert( clientData );
		static_cast<tWxAutoDeleteContainer*>( clientData )->fAdd( this );
	}

	tWxAutoDelete::~tWxAutoDelete( )
	{
		wxClientData* clientData = mParent ? mParent->GetClientObject( ) : 0;
		if( clientData )
		{
			// remove ourself from the list of auto-cleaned-up pointers
			static_cast<tWxAutoDeleteContainer*>( clientData )->fRemove( this );
		}
	}

}

