#ifndef __Debug_CrashDump__
#define __Debug_CrashDump__

namespace Sig { namespace Debug
{
	void fSetDumpPath( tFilePathPtr path );
	void fAddFileToDumps( tFilePathPtr path );

#if defined( platform_metro )
	/// \brief Creates a dump file and forces the program to crash
	LONG base_export fDoSehCrash( PEXCEPTION_POINTERS pointers );

	/// \brief Wheither __try / __except / fDoSehCrash wrappers should be used.  Typically corresponds to a debugger being attached.
	b32 base_export fDisableDoSehCrash();

	/// \brief Ensure any SEH exceptions create a dump file and crash the program.
	template < typename F >
	void fDumpCrashSEH( const F& f )
	{
		if( fDisableDoSehCrash() ) { f(); }
		else __try { f(); }
		__except( fDoSehCrash(GetExceptionInformation()) ) {}
	}

	/// \brief FSTC = Force SEH To Crash (probably should've named these better but they're implementation detail structs anyways)
	template < typename F > struct tWrapFSTC;

	template <> struct tWrapFSTC< void(*)() >
	{
		typedef void(*F)();
		F mCB;
		tWrapFSTC( const F& cb ): mCB(cb) {}
		void operator()() const
		{
			if( fDisableDoSehCrash() ) { mCB(); }
			else __try { return mCB(); }
			__except( fDoSehCrash( GetExceptionInformation() ) ) {}
		}
	};

	template < typename A1 > struct tWrapFSTC< void(*)(A1) >
	{
		typedef void(*F)(A1);
		F mCB;
		tWrapFSTC( const F& cb ): mCB(cb) {}
		void operator()( const A1& a1 ) const
		{
			if( fDisableDoSehCrash() ) { mCB(a1); }
			else __try { return mCB(a1) }
			__except( fDoSehCrash( GetExceptionInformation() ) ) {}
		}
	};

	template < typename A1, typename A2 > struct tWrapFSTC< void(*)(A1,A2) >
	{
		typedef void(*F)(A1,A2);
		F mCB;
		tWrapFSTC( const F& cb ): mCB(cb) {}
		void operator()( const A1& a1, const A2& a2 ) const
		{
			if( fDisableDoSehCrash() ) { mCB(a1,a2); }
			else __try { return mCB(a1,a2); }
			__except( fDoSehCrash( GetExceptionInformation() ) ) {}
		}
	};

	template < typename T > struct tWrapFSTC< void(T::*)() >
	{
		typedef void(T::*F)();
		tRefCounterPtr<T> mThat;
		F mCB;
		tWrapFSTC( T* that, const F& cb ): mThat(that), mCB(cb) {}
		void operator()() const
		{
			if( fDisableDoSehCrash() ) { mThat.fGetRawPtr()->*mCB(); }
			else __try { return (mThat.fGetRawPtr()->*mCB)(); }
			__except( fDoSehCrash( GetExceptionInformation() ) ) {}
		}
	};

	template < typename T, typename A1 > struct tWrapFSTC< void(T::*)(A1) >
	{
		typedef void(T::*F)(A1);
		tRefCounterPtr<T> mThat;
		F mCB;
		tWrapFSTC( T* that, const F& cb ): mThat(that), mCB(cb) {}
		void operator()( const A1& a1 ) const
		{
			if( fDisableDoSehCrash() ) { mThat.fGetRawPtr()->*mCB(a1); }
			else __try { return (mThat.fGetRawPtr()->*mCB)(a1) }
			__except( fDoSehCrash( GetExceptionInformation() ) ) {}
		}
	};

	template < typename T, typename A1, typename A2 > struct tWrapFSTC< void(T::*)(A1,A2) >
	{
		typedef void(T::*F)(A1,A2);
		tRefCounterPtr<T> mThat;
		F mCB;
		tWrapFSTC( T* that, const F& cb ): mThat(that), mCB(cb) {}
		void operator()( const A1& a1, const A2& a2 ) const
		{
			if( fDisableDoSehCrash() ) { (mThat.fGetRawPtr()->*mCB)(a1,a2); }
			else __try { return (mThat.fGetRawPtr()->*mCB)(a1,a2); }
			__except( fDoSehCrash( GetExceptionInformation() ) ) {}
		}
	};



	template < typename F >
	struct tInferWrapFSTC
	{
		tInferWrapFSTC( F f ): mF(f) {}

		F mF;
		template < typename WinrtDelegateType >
		operator WinrtDelegateType^() const
		{
			return ref new WinrtDelegateType( tWrapFSTC<F>(mF) );
		}
	};

	template < typename T, typename F >
	struct tInferWrapMemberFSTC
	{
		tInferWrapMemberFSTC( T* that, F f ): mThat(that), mF(f) {}

		T* mThat;
		F mF;

		template < typename WinrtDelegateType >
		operator WinrtDelegateType^() const
		{
			return ref new WinrtDelegateType( tWrapFSTC<F>(mThat,mF) );
		}
	};



	template < typename F >
	tInferWrapFSTC<F> fInferCallbackTypeAndDumpCrashSEH( const F& f )
	{
		return tInferWrapFSTC<F>(f);
	}

	template < typename T, typename F >
	tInferWrapMemberFSTC<T,F> fInferCallbackTypeAndDumpCrashSEH( T* that, const F& f )
	{
		return tInferWrapMemberFSTC<T,F>(that,f);
	}

#else
	template < typename F > void fDumpCrashSEH( const F& f ) { f(); }

#endif

}}

#endif //ndef __Debug_CrashDump__
