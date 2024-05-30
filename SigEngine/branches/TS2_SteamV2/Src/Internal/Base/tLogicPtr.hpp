#ifndef __tLogicPtr__
#define __tLogicPtr__

namespace Sig
{
	///
	/// \brief Acts as the glue between script and C++ logic, bound to tEntity.
	class tLogicPtr : public tScriptOrCodeObjectPtr<tLogic>
	{
	public:
		tLogicPtr( ) { }
		explicit tLogicPtr( const Sqrat::Object& o ) 
			: tScriptOrCodeObjectPtr<tLogic>( o ) { }
		explicit tLogicPtr( tLogic* o ) 
			: tScriptOrCodeObjectPtr<tLogic>( o ) { }
		const char* fDebugTypeName( ) const;
		void fOnSpawn( );
		void fOnDelete( );
		void fComputeDebugText( std::string& textOut ) const;
	};
}

#endif//__tLogicPtr__
