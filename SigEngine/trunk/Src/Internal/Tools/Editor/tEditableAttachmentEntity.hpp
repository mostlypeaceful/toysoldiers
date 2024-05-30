#ifndef __tEditableAttachmentEntity__
#define __tEditableAttachmentEntity__
#include "tEditableObject.hpp"

namespace Sig { namespace Sigml { class tAttachmentObject; }}

namespace Sig
{
	class tools_export tEditableAttachmentEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableAttachmentEntity, tEditableObject );
	private:
		Gfx::tRenderableEntityPtr mShellSphere;
		Gfx::tRenderState mShellRenderState;
	public:
		tEditableAttachmentEntity( tEditableObjectContainer& container );
		tEditableAttachmentEntity( tEditableObjectContainer& container, const Sigml::tAttachmentObject& ao );
		~tEditableAttachmentEntity( );
		virtual std::string fGetToolTip( ) const;
	private:
		void fCommonCtor( );
		void fAddEditableProperties( );
	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
	};

}

#endif//__tEditableAttachmentEntity__
