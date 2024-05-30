#ifndef __tAttachmentEntity__
#define __tAttachmentEntity__
#include "tEntityDef.hpp"
#include "tStateableEntity.hpp"

namespace Sig
{
	class base_export tAttachmentEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tAttachmentEntityDef, 0x4EB17202 );
	public:
		u16 mStateMask;
		u16 pad0;

	public:
		tAttachmentEntityDef( );
		tAttachmentEntityDef( tNoOpTag );
		~tAttachmentEntityDef( );
		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};

	class base_export tAttachmentEntity : public tStateableEntity
	{
		debug_watch( tAttachmentEntity );
		define_dynamic_cast( tAttachmentEntity, tStateableEntity );
	private:
		const tAttachmentEntityDef* mEntityDef;
	public:
		tAttachmentEntity( const tAttachmentEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );
		virtual b32 fIsHelper( ) const { return true; }
		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		virtual void fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tAttachmentEntity );

}

#endif//__tAttachmentEntity__
