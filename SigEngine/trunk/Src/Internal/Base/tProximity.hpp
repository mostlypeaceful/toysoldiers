#ifndef __tProximity__
#define __tProximity__

namespace Sig
{
	///
	/// \brief Represents a filtered spatial query of the scene graph. Encapsulates the use of multiple shapes, treating it in effect as though
	/// it were a single shape - note however that the more shapes a tProximity object is composed of, the more actual underlying scene graph
	/// queries will be made (and hence the more expensive it will be).
	/// Queries are filtered using two mechanisms. The first and simplest is simply a set of tags and properties that can be specified by clients -
	/// entities that intersect the proximity's volumes that contain NONE of those tags and/or properties will be ignored - put another way, entities
	/// that have any of the tags/properties in the proximity's filter will be included. The second mechanism is a user-supplied callback that can
	/// use any custom criteria to further refine the entity list after the spatial query and initial filtering using tags/properties is complete - 
	/// this callback will also be called from the same thread as fRefresh (see tPostFilterFunction).
	/// \note Intended for use in threads (see fRefresh).
	class base_export tProximity : public tRefCounter
	{
	public:

		class tPropertyGroup
		{
		public:
			inline tPropertyGroup( ) : mTags( 0 ), mSet( false ) { }
			inline void fAddTag( tEntityTagMask mask )						{ mTags |= mask; mSet = true; }
			inline void fRemoveTag( tEntityTagMask mask )					{ mTags &= ~mask; mSet = true; }
			
			// The any properties must have any of these flags
			inline void fAddProperty( const tEntityEnumProperty& prop )		{ mPropsAny.fFindOrAdd( prop ); mSet = true; }
			inline void fRemoveProperty( const tEntityEnumProperty& prop )	{ mPropsAny.fFindAndErase( prop ); }
			inline b32 fHasProperty( const tEntityEnumProperty& prop )		{ return mPropsAny.fFind( prop ) != 0; }

			// The all properties must have ALL of these flags
			//	This is is only applied to filter by logics, on the logic filter
			inline void fAddMustHaveProperty( const tEntityEnumProperty& prop )		{ mPropsAll.fFindOrAdd( prop ); mSet = true; }
			inline void fRemoveMustHaveProperty( const tEntityEnumProperty& prop )	{ mPropsAll.fFindAndErase( prop ); }
			
			inline const tEntityEnumPropertyList& fPropsAny( ) const		{ return mPropsAny; }
			inline const tEntityEnumPropertyList& fPropsMustHave( ) const	{ return mPropsAll; }
			
			inline tEntityTagMask fTags( ) const							{ return mTags; }
			inline void fSetTags( const tEntityTagMask& mask ) 				{ mTags = mask; }
			inline b32 fSet( ) const { return mSet; }
		private:
			tEntityEnumPropertyList mPropsAny;
			tEntityEnumPropertyList mPropsAll;
			tEntityTagMask			mTags;
			b32 mSet;
		};

		enum tShapeType { cShapeAabb, cShapeObb, cShapeSphere };

		class base_export tShape
		{
		public:
			explicit tShape( tShapeType type = cShapeAabb, const Math::tAabbf& localSpaceBox = Math::tAabbf( 0.f, 0.f ) );
			inline tShapeType fType( ) const { return mType; }
			inline const Math::tAabbf& fLocalSpaceBox( ) const { return mLocalSpaceBox; }
			inline f32 fRadius( ) const { return mRadius; }
			inline void fSetSphereRadius( f32 newRadius ) { sigassert( mType == cShapeSphere ); mRadius = newRadius; } // this is only safe if you know you have a sphere
			inline Math::tAabbf fToAabb( const Math::tMat3f& objToWorld ) const { 
				const Math::tVec3f center = objToWorld.fGetTranslation( );
				return Math::tAabbf( center + mLocalSpaceBox.mMin, center + mLocalSpaceBox.mMax );
			}
			inline Math::tObbf	fToObb( const Math::tMat3f& objToWorld ) const {
				return Math::tObbf( mLocalSpaceBox, objToWorld );
			}
			inline Math::tSpheref fToSphere( const Math::tMat3f& objToWorld ) const {
				const Math::tVec3f center = objToWorld.fXformPoint( mLocalSpaceBox.fComputeCenter( ) );
				return Math::tSpheref( center, mRadius );
			}

			void fIntersect( const tSceneGraph& sg, const tEntity& parent, const Math::tMat3f& xform, tProximity& proximity );
			void fDebugRender( const tSceneGraph& sg, const Math::tMat3f& xform, const tProximity& proximity ) const;

		private:
			Math::tAabbf	mLocalSpaceBox;
			f32				mRadius;
			tShapeType		mType;
		};
		typedef tGrowableArray< tShape > tShapeList;

		///
		/// \brief This is the signature for a callback function that is invoked after the spatial query - the intention
		/// is that you can further filter the entity list using custom criteria.
		/// \note This function will be called from whatever thread fRefresh is called from, meaning you should be careful!
		/// That said the parameter 'entsToPostFilter' is intended to be modified.
		typedef tDelegate< void ( const tEntity& parent, tProximity& proximity, tGrowableArray<tEntityPtr>& entsToPostFilter ) > tPostFilterFunction;

	public:
		explicit tProximity( );
		explicit tProximity( const tStringPtr& groupName, u32 maxHitsPerFrame );
		~tProximity( );

		///
		/// \brief This is the primary "OnTick" function for a tProximity object - it will increment internal timers,
		/// checking against refresh frequency to see whether it is time to update its cached spatial query.
		/// \note It is intended that this method be called from another thread, though obviously it can be called from the primary thread as well.
		/// \returns true if the entity count was refreshed.
	private:
		b32		fRefreshMT( f32 dt, const tEntity& parent, const Math::tMat3f *explicitXform );
	public:
		inline b32		fRefreshMT( f32 dt, const tEntity& parent, const Math::tMat3f& explicitXform ) { return fRefreshMT( dt, parent, &explicitXform ); }
		inline b32		fRefreshMT( f32 dt, const tEntity& parent ) { return fRefreshMT( dt, parent, NULL ); }
		
		/// \brief Causes the next call to fRefreshMT to actually update.
		void	fClearRefreshDelay( ) { mElapsedTime = 0.f; }
		///
		/// \brief Allows a client to explicitly debug render the volumes in this proximity.
		void	fDebugRender( const tSceneGraph& sg, const Math::tMat3f& xform ) const;

		///
		/// \brief If you store a tProximity object as a member, or if your tProximity object persists beyond a single function call some other way,
		/// then be sure you call this method once per game update loop from a single threaded run list, before fRefreshMT called.
		void	fCleanST( );

		///
		/// \brief This should only be called from single threaded code, usually in tLogic::fOnDelete
		void	fReleaseAllEntityRefsST( );

		///
		/// \brief Pause the proximity object, meaning that each fRefresh call will have no effect. Unpausing will
		/// cause an immediate refresh on the next call of fRefresh.
		void	fPause( b32 pause );

		///
		/// \brief If true, will not update the proximity query if the group max hit has been reached.
		inline void	fSetRespectGroupMax( b32 respect ) { mRespectGroupMax = respect; }
		inline b32 fRespectGroupMax( ) const { return mRespectGroupMax; }

		// refresh frequency (in seconds)
		f32		fRefreshFrequency( )										{ return mRefreshFrequency; }
		void	fSetRefreshFrequency( f32 frequency, f32 freqRandDeviation = 0.f );

		// if no spatial set indices are specified, the proximity will query all spatial sets - however you
		// can improve performance by specifying the indices of the spatial sets you care about
		const tDynamicArray<u32>& fSpatialSetIndices( ) const				{ return mSpatialSetIndices; }
		void fSetSpatialSetIndices( const tDynamicArray<u32>& indices )		{ mSpatialSetIndices = indices; }
		void fSetSpatialSetIndices( const u32 indices[], u32 count )		{ mSpatialSetIndices.fInitialize( indices, count ); }

		// control the tags/properties filter
		const tPropertyGroup&	fFilter( ) const							{ return mFilter; }
		tPropertyGroup&			fFilter( )									{ return mFilter; }
		
		// the LogicFilter is applied after "filter by logic types" has found the logic entity
		const tPropertyGroup&	fLogicFilter( ) const						{ return mLogicFilter; }
		tPropertyGroup&			fLogicFilter( )								{ return mLogicFilter; }

		tPropertyGroup* fFilterForScript( )									{ return &mFilter; }
		tPropertyGroup* fLogicFilterForScript( )							{ return &mLogicFilter; }


		// control the filter callback
		const tPostFilterFunction&	fPostFilterFunction( ) const					{ return mPostFilterFunction; }
		void	fSetPostFilterFunction( const tPostFilterFunction& postFilterFunc ) { mPostFilterFunction = postFilterFunc; }

		// fFilterByLogicEnts: If true, when adding it will find the first ancestor with logic; default is false
		b32		fFilterByLogicEnts( ) const									{ return mFilterByLogicEnts; }
		void	fSetFilterByLogicEnts( b32 useLogic )						{ mFilterByLogicEnts = useLogic; }

		// fTrackNewEnts: If true, when adding not clear the filtered list and push new entities into mNewEnts
		b32		fTrackNewEnts( ) const										{ return mTrackNewEnts; }
		void	fSetTrackNewEnts( b32 trackNew )							{ mTrackNewEnts = trackNew; }

		// fQueryInheritedProperties: If true, when filtering it will search all ancestors for each entity; default is false
		b32		fQueryInheritedProperties( ) const							{ return mQueryInheritedProperties; } 
		void	fSetQueryInheritedProperties( b32 inherited )				{ mQueryInheritedProperties = inherited; }

		// shapes
		tShapeList&			fShapes( )										{ return mShapes; }
		const tShapeList&	fShapes( ) const								{ return mShapes; }
		void				fClearShapes( )									{ mShapes.fSetCount( 0 ); }
		void				fAddAabb( const Math::tAabbf& shape );
		void				fAddObb( const Math::tAabbf& shape );
		void				fAddSphere( const Math::tSpheref& shape );

		// filtered entity list
		u32					fEntityCount( ) const							{ return mFilteredEnts.fCount( ); }
		tEntity*			fGetEntity( u32 i ) const						{ return mFilteredEnts[ i ].fGetRawPtr( ); }
		u32					fNewEntityCount( ) const						{ return mNewEnts.fCount( ); }
		tEntity*			fGetNewEntity( u32 i ) const					{ return mNewEnts[ i ].fGetRawPtr( ); }		
		void				fAddEntity( tEntity* ent ); // adds entity to filtered entity list - shouldn't be called externally
		const tGrowableArray<tEntityPtr>& fEntityList( ) const					{ return mFilteredEnts; }

		if_devmenu( const Math::tVec4f& fDebugColor( ) const { return mDebugColor; } );



		///
		/// \brief Visit all the entities in proximity whose bounds intersect the specified ray. You can define your
		/// own tRayCastCallback type to perform custom functionality.
		template<class tRayCastOperator>
		void fRayCast( const Math::tRayf& ray, const tRayCastOperator& rayCastCallback ) const
		{
			sigassert( !fFilterByLogicEnts( ) );
			for( u32 i = 0; i < mFilteredEnts.fCount( ); ++i )
				rayCastCallback( ray, static_cast< tSpatialEntity* >( mFilteredEnts[ i ].fGetRawPtr( ) )->fToSpatialSetObject( ) );
		}

	private:
		void fRefreshEntityList( const tEntity& parent, const Math::tMat3f *explicitXform );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tDynamicArray<u32>			mSpatialSetIndices;
		tGrowableArray<tEntityPtr>	mFilteredEnts, mFilteredEntsToDelete;
		tGrowableArray<tEntityPtr>	mNewEnts, mNewEntsToDelete;
		tGrowableArray<tEntityPtr>	mLastFilteredEnts; //this is for track new ents to know which ents to remove
		tPropertyGroup				mFilter;
		tPropertyGroup				mLogicFilter;
		tPostFilterFunction			mPostFilterFunction;

		tShapeList					mShapes;

		f32							mRefreshFrequency;
		f32							mElapsedTime;

		b8							mFilterByLogicEnts;
		b8							mQueryInheritedProperties;
		b8							mTrackNewEnts;
		b8							mPaused;
		b16							mRespectGroupMax;
		b16							mPad0;
		tStringPtr					mGroupName;

		if_devmenu( Math::tVec4f mDebugColor; )

		struct tProximityGroup
		{
			u32 mHits;
			u32 mMaxHits;

			tProximityGroup( u32 maxHits = ~0 ) : mHits( 0 ), mMaxHits( maxHits ) { }
		};
		typedef tHashTable< tStringPtr, tProximityGroup > tProximityGroupList;
		static tProximityGroupList sGroups;
		static b32 sGroupsNeedCleaned;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tProximity );

	typedef tGrowableArray< tProximityPtr > tProximityList;

	/// tMovingProximity encapsulate the logic to have a moving proximity query
	///  that updates only when the entity has moved outside of it.
	///  This is used for heavy raycasters such as characters and vehicles.
	class tMovingProximity
	{
	public:
		explicit tMovingProximity( );
		explicit tMovingProximity( const tStringPtr& groupName, u32 maxHitsPerFrame );
		void fOnDelete( );

		// Object space bounding box.
		void fSetBounds( const Math::tAabbf& bounds );

		// Proximity will move when any of its probe points move outside the bounds.
		tGrowableArray< Math::tVec3f >& fProbePoints( ) { return mProbePoints; }

		// Call this from where ever you'd ideally run the prox query.
		//  zdir is the zDirection you want the box to align to. you may want to use the velocity.
		//  worldSpaceFutureOffset is the position delta between now and when UpdateCritical is called.
		void fUpdateIdeal( tLogic* logic, const Math::tVec3f& zDir, const Math::tVec3f& worldSpaceFutureOffset );

		// Call this from where you need the proxy to be valid, but not necessarily the ideal place.
		//  zdir is the zDirection you want the box to align to. you may want to use the velocity.
		void fUpdateCritical( tLogic* logic, const Math::tVec3f& zDir );

		// Use this accessor to configure the proximity and to run ray casts on it.
		inline tProximity& fProximity( ) { return mProximity; }
		inline const tProximity& fProximity( ) const { return mProximity; }

		// Call this once per frame to clean up the proximity results.
		void fCleanST( );

	private:	
		void fMoveProximityMT( tLogic* logic, const Math::tMat3f& currentXform );

		tGrowableArray<Math::tVec3f> mProbePoints;
		tProximity		mProximity;
		Math::tAabbf	mLocalBounds;
		Math::tObbf		mWorldBounds;
		b16				mProxInitialized;
		b16				pad0;
	};
}

#endif // __tProximity
