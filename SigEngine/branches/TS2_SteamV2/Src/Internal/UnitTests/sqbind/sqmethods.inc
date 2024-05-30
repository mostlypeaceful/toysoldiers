/////////////////////
// SqBind 0 arguments 
/////////////////////

template<class T >
SQInteger _sqbind_sqmethod_0(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)();	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	
	(self->*method)();		
	
	return 0;
}

template<class T >
SQInteger _sqbind_sqmethod_0c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)() const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	
	(self->*method)();		
	
	return 0;
}

template<class T, class R>
SQInteger _sqbind_sqmethod_0r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)();
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
		
	R r = (self->*method)();		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T, class R>
SQInteger _sqbind_sqmethod_0rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)() const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
		
	R r = (self->*method)();		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)() ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0<T>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)() const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0c<T>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)() ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0r<T,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)() const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0rc<T,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM>
SQInteger _sqbind_sqfunction_0(VM v) {

	typedef void (*F)();	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	
	(function)();		
	
	return 0;
}



template<class VM, class R>
SQInteger _sqbind_sqfunction_0r(VM v) {

	typedef R (*F)();
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
		
	R r = (function)();		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_0<VM>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_0r<VM,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 1 arguments 
/////////////////////

template<class T,class P1 >
SQInteger _sqbind_sqmethod_1(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1);	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;

	(self->*method)(p1.getter.get(v,2));		
	
	return 0;
}

template<class T,class P1 >
SQInteger _sqbind_sqmethod_1c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1) const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;

	(self->*method)(p1.getter.get(v,2));		
	
	return 0;
}

template<class T,class P1, class R>
SQInteger _sqbind_sqmethod_1r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1);
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	
	R r = (self->*method)(p1.getter.get(v,2));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class R>
SQInteger _sqbind_sqmethod_1rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1) const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	
	R r = (self->*method)(p1.getter.get(v,2));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1<T,P1>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1c<T,P1>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1r<T,P1,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1rc<T,P1,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1>
SQInteger _sqbind_sqfunction_1(VM v) {

	typedef void (*F)(P1);	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;

	(function)(p1.getter.get(v,2));		
	
	return 0;
}



template<class VM,class P1, class R>
SQInteger _sqbind_sqfunction_1r(VM v) {

	typedef R (*F)(P1);
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	
	R r = (function)(p1.getter.get(v,2));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_1<VM,P1>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_1r<VM,P1,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 2 arguments 
/////////////////////

template<class T,class P1, class P2 >
SQInteger _sqbind_sqmethod_2(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2);	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	
	return 0;
}

template<class T,class P1, class P2 >
SQInteger _sqbind_sqmethod_2c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2) const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	
	return 0;
}

template<class T,class P1, class P2, class R>
SQInteger _sqbind_sqmethod_2r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2);
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class R>
SQInteger _sqbind_sqmethod_2rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2) const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2<T,P1, P2>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2c<T,P1, P2>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2r<T,P1, P2,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2rc<T,P1, P2,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2>
SQInteger _sqbind_sqfunction_2(VM v) {

	typedef void (*F)(P1, P2);	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;

	(function)(p1.getter.get(v,2), p2.getter.get(v,3));		
	
	return 0;
}



template<class VM,class P1, class P2, class R>
SQInteger _sqbind_sqfunction_2r(VM v) {

	typedef R (*F)(P1, P2);
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_2<VM,P1, P2>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_2r<VM,P1, P2,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 3 arguments 
/////////////////////

template<class T,class P1, class P2, class P3 >
SQInteger _sqbind_sqmethod_3(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3);	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	
	return 0;
}

template<class T,class P1, class P2, class P3 >
SQInteger _sqbind_sqmethod_3c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3) const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class R>
SQInteger _sqbind_sqmethod_3r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3);
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class R>
SQInteger _sqbind_sqmethod_3rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3) const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3<T,P1, P2, P3>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3c<T,P1, P2, P3>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3r<T,P1, P2, P3,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3rc<T,P1, P2, P3,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3>
SQInteger _sqbind_sqfunction_3(VM v) {

	typedef void (*F)(P1, P2, P3);	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;

	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class R>
SQInteger _sqbind_sqfunction_3r(VM v) {

	typedef R (*F)(P1, P2, P3);
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_3<VM,P1, P2, P3>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_3r<VM,P1, P2, P3,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 4 arguments 
/////////////////////

template<class T,class P1, class P2, class P3, class P4 >
SQInteger _sqbind_sqmethod_4(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3, P4);	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4 >
SQInteger _sqbind_sqmethod_4c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3, P4) const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class R>
SQInteger _sqbind_sqmethod_4r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3, P4);
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class R>
SQInteger _sqbind_sqmethod_4rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3, P4) const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4<T,P1, P2, P3, P4>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4c<T,P1, P2, P3, P4>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4r<T,P1, P2, P3, P4,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4rc<T,P1, P2, P3, P4,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3, class P4>
SQInteger _sqbind_sqfunction_4(VM v) {

	typedef void (*F)(P1, P2, P3, P4);	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;

	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class P4, class R>
SQInteger _sqbind_sqfunction_4r(VM v) {

	typedef R (*F)(P1, P2, P3, P4);
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3, class P4>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_4<VM,P1, P2, P3, P4>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_4r<VM,P1, P2, P3, P4,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 5 arguments 
/////////////////////

template<class T,class P1, class P2, class P3, class P4, class P5 >
SQInteger _sqbind_sqmethod_5(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3, P4, P5);	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5 >
SQInteger _sqbind_sqmethod_5c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3, P4, P5) const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class R>
SQInteger _sqbind_sqmethod_5r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3, P4, P5);
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class R>
SQInteger _sqbind_sqmethod_5rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3, P4, P5) const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5<T,P1, P2, P3, P4, P5>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5c<T,P1, P2, P3, P4, P5>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5r<T,P1, P2, P3, P4, P5,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5rc<T,P1, P2, P3, P4, P5,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3, class P4, class P5>
SQInteger _sqbind_sqfunction_5(VM v) {

	typedef void (*F)(P1, P2, P3, P4, P5);	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;

	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class P4, class P5, class R>
SQInteger _sqbind_sqfunction_5r(VM v) {

	typedef R (*F)(P1, P2, P3, P4, P5);
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3, class P4, class P5>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4, P5), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_5<VM,P1, P2, P3, P4, P5>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4, class P5,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4, P5), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_5r<VM,P1, P2, P3, P4, P5,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 6 arguments 
/////////////////////

template<class T,class P1, class P2, class P3, class P4, class P5, class P6 >
SQInteger _sqbind_sqmethod_6(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3, P4, P5, P6);	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6 >
SQInteger _sqbind_sqmethod_6c(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );
	typedef void (T::*M)(P1, P2, P3, P4, P5, P6) const;	
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6, class R>
SQInteger _sqbind_sqmethod_6r(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3, P4, P5, P6);
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6, class R>
SQInteger _sqbind_sqmethod_6rc(HSQUIRRELVM v) {

	const T* self = &SqBind<T>::get(v, 1 );		
	typedef R (T::*M)(P1, P2, P3, P4, P5, P6) const;
	M*methodptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&methodptr,NULL);
	M method = *methodptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5, P6) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6<T,P1, P2, P3, P4, P5, P6>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5, P6) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6c<T,P1, P2, P3, P4, P5, P6>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5, P6) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6r<T,P1, P2, P3, P4, P5, P6,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5, P6) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6rc<T,P1, P2, P3, P4, P5, P6,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3, class P4, class P5, class P6>
SQInteger _sqbind_sqfunction_6(VM v) {

	typedef void (*F)(P1, P2, P3, P4, P5, P6);	
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;

	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class P4, class P5, class P6, class R>
SQInteger _sqbind_sqfunction_6r(VM v) {

	typedef R (*F)(P1, P2, P3, P4, P5, P6);
	F*functionptr;	
	sq_getuserdata(v,-1,(SQUserPointer*)&functionptr,NULL);
	F function = *functionptr;
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3, class P4, class P5, class P6>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4, P5, P6), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_6<VM,P1, P2, P3, P4, P5, P6>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4, class P5, class P6,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4, P5, P6), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_6r<VM,P1, P2, P3, P4, P5, P6,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


