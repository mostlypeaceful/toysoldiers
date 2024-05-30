#ifndef __tAsyncFileReaderQueue__
#define __tAsyncFileReaderQueue__
#include "tAsyncFileReader.hpp"
#include "Threads/tLocklessQueue.hpp"
#include "Threads/tWorkerThread.hpp"

namespace Sig
{

	class tAsyncFileReader;

	///
	/// \brief This class is used internally by the tAsyncFileReader class. Developers
	/// should not interact with this class directly (i.e., you should never
	/// find yourself calling fEnqueueForOpen or fEnqueueForRead).  To that end,
	/// everything in this class is private.
	class base_export tAsyncFileReaderQueue : public Threads::tWorkerThread
	{
		friend class tAsyncFileReader;
		declare_singleton_define_own_ctor_dtor( tAsyncFileReaderQueue );

	private:

		Threads::tLocklessQueue<tAsyncFileReader*>	mOpenQueue;
		Threads::tLocklessQueue<tAsyncFileReader*>	mReadQueue;

	private:

		tAsyncFileReaderQueue( );
		void fEnqueueForOpen( tAsyncFileReader* fileReader );
		void fEnqueueForRead( tAsyncFileReader* fileReader );
		virtual void fOnThreadTick( );
	};
}

#endif//__tAsyncFileReaderQueue__
