#ifndef __tEditableObject__
#define __tEditableObject__
#include "Gfx/tRenderableEntity.hpp"
#include "tEditableProperty.hpp"
#include "Sigml.hpp"
#include "tEditorSelectionList.hpp"

namespace Sig
{
	class tEditableObjectContainer;

	///
	/// \brief Base class for "objects" that live in the editor world.
	/// This class is the base type that should be used on entities.
	/// Provides lots of useful editing functionality for moving, selecting,
	/// picking, deleting ... etc ... related to the world.
	class tools_export tEditableObject : public tEntity
	{
		friend class tEditableObjectContainer;
		define_dynamic_cast( tEditableObject, tEntity );
	public:

		class tDummyObjectEntity : public Gfx::tRenderableEntity
		{
			define_dynamic_cast( tDummyObjectEntity, Gfx::tRenderableEntity );
		private:
			b32 mUseSphereCollision;
		public:
			tDummyObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox, b32 useSphereCollision = false );
			virtual void fApplyCreationFlags( const tEntityCreationFlags& creationFlags );
			virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
			virtual b32	fIntersects( const Math::tFrustumf& v ) const;
		};

		enum tState
		{
			cStateShown,
			cStateHidden,
			cStateFrozen,
			cStateCount
		};

	protected:

		// dynamic state relating to document/editor management, not serialized
		tEditableObjectContainer& mContainer;
		b32					mInContainer;
		b32					mSelected;

		// serialized values
		u32					mGuid;
		tState				mState; // i.e., hidden, frozen, visible, etc
		std::string			mLayer;

		// editable properties (these go in the property dialog, and are serialized)
		mutable tEditablePropertyTable mEditableProperties;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnPropertyChanged;

		// helper graphics objects
		Gfx::tRenderableEntityPtr		mSelectionBox;
		Gfx::tRenderableEntityPtr		mDummyBox;
		Gfx::tRenderableEntityPtr		mDummySphere;

		// for being able to translate gizmo snap
		Math::tMat3f	mTransition;

	public:

		tEditableObject( tEditableObjectContainer& container );
		virtual ~tEditableObject( );

		b32								fInContainer( ) const { return mInContainer; }
		tEditableObjectContainer&		fGetContainer( ) { return mContainer; }
		const tEditableObjectContainer& fGetContainer( ) const { return mContainer; }

		tEntityPtr						fToSmartPtr( ) { return tEntityPtr( this ); }

		virtual void 					fAddToWorld( );
		virtual void 					fRemoveFromWorld( );

		virtual void					fTickUpdate( ) { }

		void 							fEnableSelectionBox( );
		void 							fDisableSelectionBox( );

		u32								fGuid( ) const { return mGuid; }
		void							fSetGuid( u32 newGuid ) { mGuid = newGuid; }
		tState							fState( ) const { return mState; }
		void							fSetState( tState newState );
		const std::string&				fLayer( ) const { return mLayer; }
		void							fSetLayer( const std::string& layer ) { mLayer = layer; }
		b32								fIsShown( ) const { return mState == cStateShown; }
		void							fHide( b32 hide = true );
		b32								fIsHidden( ) const { return mState == cStateHidden; }
		void							fFreeze( b32 freeze = true );
		b32								fIsFrozen( ) const { return mState == cStateFrozen; }

		void							fSetSelected( b32 sel );
		b32								fGetSelected( ) const { return mSelected; }

		std::string						fGetName( ) const;
		std::string						fGetScriptName( ) const;
		virtual std::string				fGetToolTip( ) const;

		Math::tMat3f&					fTransition( ) { return mTransition; }

		b32								fIsSelectionBoxEntity( const tEntity& entity ) const { return &entity == mSelectionBox.fGetRawPtr( ); }

		virtual tEditablePropertyTable&			fGetEditableProperties( ) { return mEditableProperties; }
		virtual const tEditablePropertyTable&	fGetEditableProperties( ) const { return mEditableProperties; }

		Math::tAabbf					fObjectSpaceBox( ) const;
		Math::tAabbf					fWorldSpaceBox( ) const;
		Math::tObbf						fWorldSpaceObb( ) const;

		void							fSnapToGround( const tDynamicArray< tEntity* >& toIgnore );

		virtual b32 					fSupportsTranslation( )	{ return true; }
		virtual b32 					fSupportsRotation( )	{ return true; }
		virtual b32 					fSupportsScale( )		{ return true; }
		virtual b32 					fUniformScaleOnly( )	{ return false; }

		void							fComputeDisplayStats( Gfx::tDisplayStats& displayStatsOut ) const;

		virtual void					fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable ) { }
		virtual void					fAfterAllObjectsCloned( const tEditorSelectionList& siblingObjects ) { }

		virtual void					fPreStateChange( ) { }
		virtual void					fPostStateChange( ) { }

		///
		/// \brief Base implementation of fClone serializes the object, then creates a new instance;
		/// if this functionality is not suitable, you can still override it, though its preferable
		/// to re-use the serialization mechanism
		virtual tEntityPtr				fClone( );

		///
		/// \brief Convert the editable object to a serializable sigml object.
		virtual Sigml::tObjectPtr		fSerialize( b32 clone ) const = 0;

		Math::tVec4f					fCurrentStateTint( ) const;
		virtual void					fUpdateStateTint( );

	protected:

		void fComputeDisplayStats( const tEntity& entity, Gfx::tDisplayStats& displayStatsOut ) const;

		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );

		void fSerializeBaseObject( Sigml::tObject* baseObject, b32 clone ) const;
		void fDeserializeBaseObject( const Sigml::tObject* baseObject );

		virtual void fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max );

		virtual void fAfterAllObjectsDeserialized( ) { }
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
		void fUpdateShadowCasting( );
	};

}


#endif//__tEditableObject__
