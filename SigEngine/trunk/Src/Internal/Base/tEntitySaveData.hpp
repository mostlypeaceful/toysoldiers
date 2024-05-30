#ifndef __tEntitySaveData__
#define __tEntitySaveData__

namespace Sig
{
	class tEntity;

	///
	/// \class tEntitySaveData
	/// \brief 
	struct base_export tEntitySaveData : public Rtti::tSerializableBaseClass,  public tRefCounter
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tEntitySaveData, 0x237B67A );
	public:
		tEntitySaveData( )
			: mDeleted( false )
		{
		}
		virtual void fSpawnSavedEntity( ) { }
		virtual void fRestoreSavedEntity( tEntity* entity ) const { }
		virtual void fSaveLoadDerived( tGameArchive& archive ) { }
		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mDeleted );
			if( archive.fFailed() ) return;
			fSaveLoadDerived( archive );
		}

		b8 mDeleted;
	};
}

#endif //ndef __tEntitySaveData__
