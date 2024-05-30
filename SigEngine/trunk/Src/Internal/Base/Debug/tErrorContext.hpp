
#ifndef __tErrorContext__
#define __tErrorContext__

#ifdef sig_assert
namespace Sig
{
	//Idea for tErrorContext taken from: http://www.altdevblogaday.com/2012/01/22/sensible-error-handling-part-1/
	class tErrorContext
	{
		//------------------------------------------------------------------------------
		// this data would just be placed in tErrorContext.cpp if Visual Studio could 
		//  handle debugging it in the watch window
		//------------------------------------------------------------------------------
	private:
		static const u32 cMaxThreads = 8;
		static const u32 cStackSize = 1024;
		struct tThreadContext
		{
			u32 mThreadId;
			char mStack[ cStackSize ];
		};
		static u32 gThreadCount;
		static tThreadContext gThreads[ cMaxThreads ]; //note: not using a tFixedGrowingArray so all of our code can use this class.

		static char* fGetCurrentThreadContextStack( );
		static u32 fPush( const char* data ); //returns how much was actually pushed so we know how much to pop
		static void fPop( u32 count );
	public:
		static void fPrint( );
		//------------------------------------------------------------------------------

	private: //data
		u32 mPushed;

	public: // User-Interface
		tErrorContext( );
		~tErrorContext( );

		tErrorContext& operator<<( const char* data );
		tErrorContext& operator<<( const void* ptr );
		tErrorContext& operator<<( s32 data );
		tErrorContext& operator<<( u32 data );
		tErrorContext& operator<<( f32 data );

		// Signal-Specific
		tErrorContext& operator<<( const class tStringPtr& str );
		tErrorContext& operator<<( const class tFilePathPtr& str );
	};
}//Sig
#define error_context( x ) ::Sig::tErrorContext __awesome_debug_error_context; __awesome_debug_error_context << x;
#else
#define error_context( x )
#endif//sig_assert

#endif//__tErrorContext__
