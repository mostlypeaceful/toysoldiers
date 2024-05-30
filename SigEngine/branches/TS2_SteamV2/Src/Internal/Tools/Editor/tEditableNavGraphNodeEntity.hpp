//------------------------------------------------------------------------------
// \file tEditableNavGraphNodeEntity.hpp - 06 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableNavGraphNodeEntity__
#define __tEditableNavGraphNodeEntity__
#include "tEditableObject.hpp"
#include "Gfx\tSolidColorQuads.hpp"

namespace Sig
{
	namespace Sigml
	{
		class tNavGraphNodeObject;
		class tNavGraphRootObject;
	}


	class tEditableNavGraphRootEntity;
	class tEditableNavGraphNodeEntity;
	define_smart_ptr( tools_export, tRefCounterPtr, tEditableNavGraphNodeEntity );

	///
	/// \class tEditableNavGraphNodeEntity
	/// \brief 
	class tools_export tEditableNavGraphNodeEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableNavGraphNodeEntity, tEditableObject );

		Gfx::tRenderState mConnectorOverride;

		Gfx::tRenderableEntityPtr mShellBox;
		Gfx::tRenderState mShellRenderState;
		tGrowableArray<Gfx::tRenderableEntityPtr> mConnectionQuads;
		tGrowableArray<Gfx::tSolidColorQuadsPtr> mGeometry;

		tGrowableArray<tEditableNavGraphNodeEntityPtr> mBackConnections;
		tGrowableArray<tEditableNavGraphNodeEntityPtr> mConnections;

		tDynamicArray<u32> mSavedConnectionGuids;

	public:
		tEditableNavGraphNodeEntity( tEditableObjectContainer& container );
		tEditableNavGraphNodeEntity( tEditableObjectContainer& container, const Sigml::tNavGraphNodeObject* ao );
		virtual ~tEditableNavGraphNodeEntity( );

		void fRefresh( );

		virtual void fConnect( tEditableNavGraphNodeEntity* to, b32 removeConnection = false );
		virtual void fDisconnect( b32 outwardOnly = false );

		const tGrowableArray<tEditableNavGraphNodeEntityPtr>& fBackConnections( ) const { return mBackConnections; }
		const tGrowableArray<tEditableNavGraphNodeEntityPtr>& fConnections( ) const { return mConnections; }

		virtual std::string fGetToolTip( ) const;

		void fAcquireEntireGraph( tGrowableArray<tEditableNavGraphNodeEntity*>& nodes );

		tEditableNavGraphRootEntity* fRoot( );

		void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );

		void fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable );

	protected:
		void fCommonCtor( );
		void fAfterAllObjectsDeserialized( );
		void fRecordConnections( Sigml::tNavGraphNodeObject* ao ) const;
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		void fClearConnections( );

		///
		/// \brief Returns 4 flat verts of the box in world-space.
		void fGetFlat( tGrowableArray<Math::tVec3f>& outVerts );

		static void fSplitCorners( 
			const tGrowableArray<Math::tVec3f>& verts,
			const Math::tMat3f& basis,
			const Math::tVec3f& org, 
			Math::tVec3f& lVert, 
			Math::tVec3f& rVert,
			tGrowableArray<Math::tVec3f>* allLeftVerts = NULL,
			tGrowableArray<Math::tVec3f>* allRightVerts = NULL );

		void fAddToWorld( );
		void fRemoveFromWorld( );
		virtual void fOnMoved( b32 recomputeParentRelative );
	};

	define_smart_ptr( tools_export, tRefCounterPtr, tEditableNavGraphRootEntity );

	///
	/// \class tEditableNavGraphRootEntity
	/// \brief The root node is responsible for packaging the nav graph and holding
	/// any graph-wide properties.
	class tools_export tEditableNavGraphRootEntity : public tEditableNavGraphNodeEntity
	{
		define_dynamic_cast( tEditableNavGraphRootEntity, tEditableNavGraphNodeEntity );
	public:
		tEditableNavGraphRootEntity( tEditableObjectContainer& container );
		tEditableNavGraphRootEntity( tEditableObjectContainer& container, const Sigml::tNavGraphRootObject* ao );
		virtual ~tEditableNavGraphRootEntity( );

		virtual void fConnect( tEditableNavGraphNodeEntity* to, b32 removeConnection = false );

		virtual std::string fGetToolTip( ) const;
		Sigml::tObjectPtr fSerialize( b32 clone ) const;
	};
}

#endif //__tEditableNavGraphNodeEntity__
