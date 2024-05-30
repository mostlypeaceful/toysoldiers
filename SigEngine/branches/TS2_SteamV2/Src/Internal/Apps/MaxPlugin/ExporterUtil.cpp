#include "ExporterUtil.hpp"
#include "MaxPlugin.hpp"

namespace Sig { namespace MaxPlugin { namespace ExporterUtil
{
	static const u32 START_UV_CHANNEL	= 1;
	static const u32 NUM_UV_CHANNELS	= 8;
	static const u32 START_AUX_CHANNEL	= 10;
	static const u32 NUM_AUX_CHANNELS	= 10;
	static const u32 SAMPLES_PER_SECOND = 60;

	void fExportUserProps( tXmlFile& output_file, tXmlNode node, Interface* maxIface, INode* inode )
	{
		TSTR buf;
		inode->GetUserPropBuffer( buf );
		node.fSetContents( buf.data( ) );
	}

	void fExportKeyFrames( tXmlFile& output_file, tXmlNode node, Interface* maxIface, INode* inode )
	{
		const TimeValue t_start = maxIface->GetAnimRange( ).Start( );
		const TimeValue t_end	= maxIface->GetAnimRange( ).End( );
		const f32 num_seconds = fToSeconds( t_end - t_start );
		const u32 num_samples = ( u32 )( SAMPLES_PER_SECOND * num_seconds + 0.49999f );

		std::stringstream ss;
		for( u32 i = 0; i < num_samples; ++i )
		{
			const f32 s = i / ( num_samples - 1.f );
			const TimeValue t = t_start + fToTimeValue( s );

			const Matrix3 tm = inode->GetNodeTM( t );

			// transpose as we output
			for( u32 col = 0; col < 3; ++col )
				for( u32 row = 0; row < 4; ++row )
					ss << tm[row][col] << ' ';

			ss << std::endl;
		}
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportVerts( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		std::stringstream ss;
		for( u32 i = 0; i < mesh->getNumVerts( ); ++i )
		{
			ss <<	mesh->verts[i].x << ' ' <<
					mesh->verts[i].y << ' ' <<
					mesh->verts[i].z << std::endl;
		}
		node.fGetAttribute( output_file, "count", mesh->getNumVerts( ) );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportVertTris( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		std::stringstream ss;
		for( u32 i = 0; i < mesh->getNumFaces( ); ++i )
		{
			ss <<	mesh->faces[i].v[0] << ' ' <<
					mesh->faces[i].v[1] << ' ' <<
					mesh->faces[i].v[2] << std::endl;
		}
		node.fGetAttribute( output_file, "count", mesh->getNumFaces( ) );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportMtlIds( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		std::stringstream ss;
		for( u32 i = 0; i < mesh->getNumFaces( ); ++i )
			ss << mesh->getFaceMtlIndex( i ) << std::endl;
		node.fGetAttribute( output_file, "count", mesh->getNumFaces( ) );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportSmoothingGroups( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		std::stringstream ss;
		for( u32 i = 0; i < mesh->getNumFaces( ); ++i )
			ss << mesh->faces[i].smGroup << std::endl;
		node.fGetAttribute( output_file, "count", mesh->getNumFaces( ) );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportTriNormals( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		std::stringstream ss;
		for( u32 i = 0; i < mesh->getNumFaces( ); ++i )
		{
			Point3 n = mesh->FaceNormal( i, TRUE );
			ss	<< n.x << ' '
				<< n.y << ' '
				<< n.z << std::endl;
		}
		node.fGetAttribute( output_file, "count", mesh->getNumFaces( ) );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportUvwVerts( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel )
	{
		const u32 numUvVerts = mesh->getNumMapVerts( ithChannel );
		UVVert* uvVerts = mesh->mapVerts( ithChannel );
		assert( uvVerts );

		std::stringstream ss;
		for( u32 i = 0; i < numUvVerts; ++i )
		{
			ss <<	uvVerts[i].x << ' ' <<
					uvVerts[i].y << ' ' <<
					uvVerts[i].z << std::endl;
		}
		node.fGetAttribute( output_file, "count", numUvVerts );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportUvwTris( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel )
	{
		TVFace* uvFaces = mesh->mapFaces( ithChannel );
		assert( uvFaces );

		std::stringstream ss;
		for( u32 i = 0; i < mesh->getNumFaces( ); ++i )
		{
			ss <<	uvFaces[i].t[0] << ' ' <<
					uvFaces[i].t[1] << ' ' <<
					uvFaces[i].t[2] << std::endl;
		}
		node.fGetAttribute( output_file, "count", mesh->getNumFaces( ) );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportUvwChannel( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel )
	{
		fExportUvwVerts(	output_file, node.fCreateChild( output_file, "uvw_verts" ), mesh, ithChannel );
		fExportUvwTris(	output_file, node.fCreateChild( output_file, "uvw_tris" ),	mesh, ithChannel );

		node.fGetAttribute( output_file, "channel", ithChannel );
	}

	void fExportAuxChannel( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel )
	{
		fExportUvwVerts(	output_file, node.fCreateChild( output_file, "aux_verts" ), mesh, ithChannel );
		fExportUvwTris(	output_file, node.fCreateChild( output_file, "aux_tris" ),	mesh, ithChannel );

		node.fGetAttribute( output_file, "channel", ithChannel );
	}

	void fExportUvws( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		for( u32 i = START_UV_CHANNEL; i < START_UV_CHANNEL + NUM_UV_CHANNELS; ++i )
		{
			if( !mesh->mapSupport( i ) )
				continue;
			fExportUvwChannel( output_file, node.fCreateChild( output_file, "uvw_channel" ), mesh, i );
		}
	}

	void fExportVertColors( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		if( !mesh->mapSupport( 0 ) )
			return;
		fExportUvwVerts(	output_file, node.fCreateChild( output_file, "color_verts" ),	mesh, 0 );
		fExportUvwTris(	output_file, node.fCreateChild( output_file, "color_tris" ),	mesh, 0 );
	}

	void fExportVertAlphas( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		if( !mesh->mapSupport( MAP_ALPHA ) )
			return;
		fExportUvwVerts(	output_file, node.fCreateChild( output_file, "alpha_verts" ),	mesh, MAP_ALPHA );
		fExportUvwTris(	output_file, node.fCreateChild( output_file, "alpha_tris" ),	mesh, MAP_ALPHA );
	}

	void fExportAuxChannels( tXmlFile& output_file, tXmlNode node, Mesh* mesh )
	{
		for( u32 i = START_AUX_CHANNEL; i < START_AUX_CHANNEL + NUM_AUX_CHANNELS; ++i )
		{
			if( !mesh->mapSupport( i ) )
				continue;
			fExportAuxChannel( output_file, node.fCreateChild( output_file, "aux_channel" ), mesh, i );
		}
	}

	struct tSkin
	{
		tSkin( ) : mSkinMod(0), mSkinContext(0) { }
		ISkin*				mSkinMod;
		ISkinContextData*	mSkinContext;
	};

	tSkin fGetSkin( INode* inode )
	{
		Object* object = inode->GetObjectRef( );

		// Is derived object ?
		while( object && object->SuperClassID() == GEN_DERIVOB_CLASS_ID )
		{
			// Yes -> Cast.
			IDerivedObject* derivedObject = ( IDerivedObject* ) ( object );

			// Iterate over all entries of the modifier stack.
			for( int i = 0; i < derivedObject->NumModifiers(); ++i )
			{
				// Get current modifier.
				Modifier* mod = derivedObject->GetModifier( i );

				// check for Skin modifier
				if( mod->ClassID() == SKIN_CLASSID )
				{
					tSkin s;

					s.mSkinMod = ( ISkin* )mod->GetInterface( I_SKIN );
					if ( !s.mSkinMod )
						continue;

					s.mSkinContext = s.mSkinMod->GetContextInterface( inode );
					if( !s.mSkinContext )
						continue;

					return s;
				}
			}

			object = derivedObject->GetObjRef( );
		}

		return tSkin( );
	}

	void fExportVertexWeights( tXmlFile& output_file, tXmlNode node, tSkin s, u32 ivtx )
	{
		const u32 boneCount = s.mSkinContext->GetNumAssignedBones( ivtx );
		u32 realBoneCount=0;

		std::stringstream ss;
		for( u32 i = 0; i < boneCount; ++i )
		{
			INode* bone = s.mSkinMod->GetBone( s.mSkinContext->GetAssignedBone( ivtx, i ) );
			if( !bone )
				continue;

			const f32 weight = s.mSkinContext->GetBoneWeight( ivtx, i );
			if( _isnan( weight ) )
				continue;

			std::string safeName;
			fMakeINodeNameSafeForXml( safeName, bone->GetName( ) );

			ss	<< safeName.c_str( ) << ' ' 
				<< weight << std::endl;

			++realBoneCount;
		}

		node.fGetAttribute( output_file, "count", realBoneCount );
		node.fSetContents( ss.str( ).c_str( ) );
	}

	void fExportBoneWeights( tXmlFile& output_file, tXmlNode node, Mesh* mesh, INode* inode )
	{
		tSkin s = fGetSkin( inode );

		if( !s.mSkinMod || !s.mSkinContext )
			return;

		for( u32 i = 0; i < mesh->getNumVerts( ); ++i )
		{
			fExportVertexWeights( output_file, node.fCreateChild( output_file, "vertex" ), s, i );
		}
	}

}}}

