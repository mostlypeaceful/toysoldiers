#ifndef __tDelegate__
#error This file is intended to be included multiple times, but only from within tDelegate.hpp
#endif//__tDelegate__


#if argument_count > 0
#	define argument_separator ,
#else
#	define argument_separator
#endif

#define delegate_class_name join_macros( tDelegate, argument_count )

///
/// \brief The ultimate purpose of this class template is to provide extremely efficient
/// C++ delegates, supporting both member function calls and static function calls within
/// the same delegate type and function call idiom. I.e., as long as the signatures match,
/// you can pass either a static function or a member function to the same delegate type,
/// and invoke the given function or member appropriately later (as a delegate).
///
/// An example usage of a delegate:
/// // imagine you had an object like this:
/// struct tSomeObject
/// {
/// 	void foo( int a, int b ) { log_line( 0, "tSomeObject::foo !!! " << a << " " << b ); }
/// };
/// tSomeObject so;
/// // here's the delegate, returning void and taking two integers:
/// typedef tDelegate2<void, int, int> tMyDelegate;
/// tMyDelegate d = tMyDelegate::fFromMethod<tSomeObject, &tSomeObject::foo>( &so );
/// // invoke tSomeObject's method foo, passing it 0 and 1; this delegate can be passed around,
/// // stored in lists, etc., deferring and data-izing the call-ability of all functions sharing foo's signature.
/// d( 0, 1 );
///
/// \note Template in both the C++ sense, as well as in the pre-processor macro sense;
/// i.e., this class is just a template for a templatized class; only after certain pre-processor
/// macros are defined does it in fact create a real templatized class which can then be used
/// in actual C++ code. See tDelegate.hpp for the instantations of the various templatized classes
/// (basically, one for each number of parameters, 0-N).
///
/// Note that this is basically a (legal) rip-off of "The Impossibly Fast C++ Delegates" By Sergey Ryazanov,
/// as posted on www.codeproject.com, http://www.codeproject.com/KB/cpp/ImpossiblyFastCppDelegate.aspx.
/// His implementation is in turn inspired by Don Clugston, who began the search for a fast C++ delegate:
/// http://www.codeproject.com/KB/cpp/FastDelegate.aspx.
template< class tReturnType argument_separator argument_list_template >
class delegate_class_name
{
protected:

	typedef tReturnType ( *tStub )( void* object argument_separator argument_list_declared );

	void* mObject;
	tStub mStub;

public:
	// tMethodStub + fInferMethodStubWrapper:
	//	Allows for implicit conversion / discarding (via specialization) of the return type (from "TakenR" to "FinalR")
	//	Allows for implicit conversion of the instance pointer (from "ArgumentObjectT" to "MemberOfT")
	//	Could be extended to allow for implicit conversion of method arguments as well.

	template< class FinalR, class TakenR, class ArgumentObjectT, class MemberOfT >
	struct tMethodStub
	{
		template< TakenR (MemberOfT::*method)( argument_list_declared ) >
		static inline FinalR fMethodStub( void* object argument_separator argument_list_declared )
		{
			ArgumentObjectT* p = static_cast<ArgumentObjectT*>( object );
			return ( p->*method )( argument_list_passed );
		}
	};
	
	template< class TakenR, class ArgumentObjectT, class MemberOfT >
	struct tMethodStub< void, TakenR, ArgumentObjectT, MemberOfT >
	{
		template< TakenR (MemberOfT::*method)( argument_list_declared ) >
		static inline void fMethodStub( void* object argument_separator argument_list_declared )
		{
			ArgumentObjectT* p = static_cast<ArgumentObjectT*>( object );
			( p->*method )( argument_list_passed );
		}
	};

	template< typename TakenR, typename ArgumentObjectT, typename MemberOfT >
	static inline tMethodStub< tReturnType, TakenR, ArgumentObjectT, MemberOfT > fInferMethodStubWrapper
		( ArgumentObjectT*		/* unused, except for type inference */
		, TakenR (MemberOfT::*	/* unused, except for type inference */)( argument_list_declared ) )
	{
		return tMethodStub< tReturnType, TakenR, ArgumentObjectT, MemberOfT >( );
	}

public:
	// tFunctionStub + fInferFunctionStubWrapper.  Allows for implicit conversion / discarding of the return type.
	//	Allows for implicit conversion / discarding (via specialization) of the return type (from "TakenR" to "FinalR")
	//	Could be extended to allow for implicit conversion of function arguments as well.

	template< class FinalR, class TakenR >
	struct tFunctionStub
	{
		template< TakenR (*function)( argument_list_declared ) >
		static inline FinalR fFunctionStub( void* /* unused */ argument_separator argument_list_declared )
		{
			return ( *function )( argument_list_passed );
		}
	};

	template< class TakenR >
	struct tFunctionStub< void, TakenR >
	{
		template< TakenR (*function)( argument_list_declared ) >
		static inline void fFunctionStub( void* /* unused */ argument_separator argument_list_declared )
		{
			( *function )( argument_list_passed );
		}
	};

	template< typename TakenR >
	static inline tFunctionStub< tReturnType, TakenR > fInferFunctionStubWrapper
		( TakenR (*	/* unused, except for type inference */)( argument_list_declared ) )
	{
		return tFunctionStub< tReturnType, TakenR, ArgumentObjectT, MemberOfT >( );
	}

public:
	inline delegate_class_name( )
		: mObject( 0 )
		, mStub( 0 )
	{ }

	///
	/// \brief Create a delegate from an object and a method on that object.
	/// This method modifies the current object.
	template< class T, tReturnType (T::*method)( argument_list_declared ) >
	void fFromMethod( T* object )
	{
		mObject = object;
		mStub = &fInferMethodStubWrapper( object, method ).fMethodStub< method >;
	}

	///
	/// \brief Create a delegate from an object and a method on that object.
	/// Global function, creates a new delegate.
	template< class T, tReturnType (T::*method)( argument_list_declared ) >
	static delegate_class_name fCreateFromMethod( T* object )
	{
		delegate_class_name d;
		d.fFromMethod< T, method >( object );
		return d;
	}

	///
	/// \brief Create a delegate from a global function.
	/// This method modifies the current object.
	template< tReturnType (*function)( argument_list_declared ) >
	void fFromFunction( )
	{
		mStub = &fInferFunctionStubWrapper( function ).fFunctionStub< function >;
	}

	///
	/// \brief Create a delegate from a global function.
	/// Global function, creates a new delegate.
	template< tReturnType (*function)( argument_list_declared ) >
	static delegate_class_name fCreateFromFunction( )
	{
		delegate_class_name d;
		d.fFromFunction< function >( );
		return d;
	}

	static delegate_class_name fCreateFromRawObjectAndStub( void* object, tStub stub )
	{
		delegate_class_name d;
		d.mObject = object;
		d.mStub = stub;
		return d;
	}

	///
	/// \brief Invoke the delegate with the appropriate arguments.
	inline tReturnType operator( )( argument_list_declared ) const
	{
		return ( *mStub )( mObject argument_separator argument_list_passed );
	}

	///
	/// \brief Find out if the delegate is null
	inline b32 fNull( ) const
	{
		return mStub == 0;
	}

	///
	/// \brief Make the delegate null again.
	inline void fSetToNull( )
	{
		mObject = 0;
		mStub = 0;
	}

	///
	/// \brief Comparison for delegates
	inline b32 operator==( const delegate_class_name& o ) const
	{
		return mObject == o.mObject && mStub == o.mStub;
	}
};



///
/// \brief Specialization acting purely as a wrapper to allow nicer syntax.  I.e., we can
/// write tDelegate< tReturnType ( tParam1, tParam2 ) > instead of  tDelegate2< tReturnType, tParam1, tParam2 >.
template< class tReturnType argument_separator argument_list_template >
class tDelegate< tReturnType ( argument_list_template_inst ) >
  : public delegate_class_name< tReturnType argument_separator argument_list_template_inst >
{
public:

	// Make using the base type a bit easier via typedef.
	typedef delegate_class_name< tReturnType argument_separator argument_list_template_inst > tBase;

	// allow conversion from base type
	inline tDelegate( ) { }
	inline tDelegate( const tBase& x ) : tBase( x ) { }
	inline void operator=( const tBase& x ){ *static_cast<tBase*>( this ) = x; }
};

///
/// \brief Event class manages a list of observers who desire notification
/// of an occurrence of the event. The observers (delegates) will be invoked
/// each time the fFire method of the event is called.
template< class tReturnType argument_separator argument_list_template >
class tEvent< tReturnType ( argument_list_template_inst ) >
{
public:

	// Make using the delegate type a bit easier via typedef.
	typedef delegate_class_name< tReturnType argument_separator argument_list_template_inst > tDelegateType;

	///
	/// \brief Observer class, along with the tEvent class, provides an implementation of 
	/// the observer pattern. In short, events contain a list of observers; when an event
	/// fires, all observers are notified. This provides a generic and very re-usable mechanism
	/// for event notification, allowing arbitrary types to wrap/contain observers/events.
	class tObserver : public tDelegateType
	{
		friend class tEvent;

		typedef tGrowableArray< tEvent* >	tEventList;
		tEventList mOwnerEvents;

		void fEraseEventFromEventList( tEvent* event )
		{
			mOwnerEvents.fFindAndEraseOrdered( event );
		}

	public:

		inline tObserver( ) { }

		///
		/// \brief Construct the observer with a delegate object.
		inline tObserver( const tDelegateType& delegate ) : tDelegateType( delegate ) { }

		///
		/// \brief Destructor, automatically deregisters itself with the owning event (if any).
		inline ~tObserver( ) { fRemoveFromAllEvents( ); }

		///
		/// \brief Query for how many events this delegate is currently inserted into.
		inline u32 fOwnerEventCount( ) const { return mOwnerEvents.fCount( ); }

		///
		/// \brief Remove the observer from all owner events.
		void fRemoveFromAllEvents( )
		{
			while( mOwnerEvents.fCount( ) > 0 )
				mOwnerEvents.fFront( )->fRemoveObserver( this );
		}
	};

	typedef tGrowableArray< tObserver* >	tObserverList;

private:

	const b32		mClearObserversAfterFire;
	tObserverList	mObserverList;

	void fClearObservers( )
	{
		for( u32 i = 0; i < mObserverList.fCount( ); ++i )
			mObserverList[i]->fEraseEventFromEventList( this );
		mObserverList.fSetCount( 0 );
	}

public:

	///
	/// \brief Construct the event, setting its behavior after an event fires;
	/// if you want it to auto-clear its observer list after each firing, pass true;
	/// otherwise, if you want the observer list to persist after each firing (as in
	/// a recurrent event), pass false.
	inline tEvent( const b32 clearObserversAfterFire )
		: mClearObserversAfterFire( clearObserversAfterFire )
	{
	}

	///
	/// \brief Destructor, clears observer list and automatically deregisters
	/// itself with each observer.
	~tEvent( )
	{
		fClearObservers( );
	}

	///
	/// \brief Add an observer to the event; the observer will be notified
	/// whenever the event fires.
	void fAddObserver( tObserver* observer )
	{
		// add observer
		if( !mObserverList.fFind( observer ) )
		{
			observer->mOwnerEvents.fPushBack( this );
			mObserverList.fPushBack( observer );
		}
	}

	///
	/// \brief Remove an observer from the event; the observer will no longer
	/// be notified when the event fires.
	void fRemoveObserver( tObserver* observer )
	{
		if( mObserverList.fFindAndEraseOrdered( observer ) )
			observer->fEraseEventFromEventList( this );
	}

	///
	/// \brief Fire the event, notifying any observers.
	void fFire( argument_list_declared )
	{
		tObserverList tempObservers = mObserverList;
		for( u32 i = 0; i < tempObservers.fCount( ); ++i )
			(*tempObservers[i])( argument_list_passed );
		if( mClearObserversAfterFire )
			fClearObservers( );
	}

};


#undef delegate_class_name
#undef argument_separator

