#ifndef __tReferenceFrameEntity__
#define __tReferenceFrameEntity__

namespace Sig
{
	class base_export tReferenceFrameEntity : public tEntity
	{
		define_dynamic_cast( tReferenceFrameEntity, tEntity );
	private:
		const tEntityDefProperties* mEntityDef;
	public:
		explicit tReferenceFrameEntity( const tEntityDefProperties* entityDef );
		tReferenceFrameEntity( const tEntityDefProperties* entityDef, const Math::tMat3f& objectToWorld );
		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );

		static tEntity* fSkipOverRefFrameEnts( tEntity* start )
		{
			while( start && start->fDynamicCast<tReferenceFrameEntity>( ) )
			{
				start = start->fChild( 0 ).fGetRawPtr( );
			}

			return start;
		}
	};
}

#endif//__tReferenceFrameEntity__
