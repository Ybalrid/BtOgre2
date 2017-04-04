/*
 * =====================================================================================
 *
 *       Filename:  BtOgreGP.h
 *
 *    Description:  The part of BtOgre that handles information transfer from Ogre to
 *                  Bullet (like mesh data for making trimeshes).
 *
 *        Version:  1.0
 *        Created:  27/12/2008 03:29:56 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#pragma once

#include <btBulletDynamicsCommon.h>
#include <Ogre.h>

#include "BtOgreExtras.h"

namespace BtOgre
{
	using BoneIndex = std::map<unsigned char, Vector3Array*>;
	using BoneKeyIndex = std::pair<unsigned short, Vector3Array*>;

	class VertexIndexToShape
	{
	public:
		VertexIndexToShape(const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
		virtual ~VertexIndexToShape();

		Ogre::Real getRadius();
		Ogre::Vector3 getSize();

		///Return a spherical bullet collision shape from this object
		btSphereShape* createSphere();

		///Return a box bullet collision shape from this object
		btBoxShape* createBox();

		///Return a triangular mesh collision shape from this object
		btBvhTriangleMeshShape* createTrimesh();

		///Return a cynlinder collision shape from this object
		btCylinderShape* createCylinder();

		///Return a convex hull  collision shape from this object
		btConvexHullShape* createConvex();

		///Return a capsule shape from this object
		btCapsuleShape* createCapsule();

		///Get the vertex buffer (array of vector 3)
		const Ogre::Vector3* getVertices();

		///Get the vertex count (size of vertex buffer) of the object
		unsigned int getVertexCount();

		///Get the index buffer of the object (array of unsigned ints)
		const unsigned int* getIndices();

		///Get the index count(size of vertex buffer) from this object
		unsigned int getIndexCount();

	protected:

		void addStaticVertexData(const Ogre::v1::VertexData *vertex_data);

		void addAnimatedVertexData(const Ogre::v1::VertexData *vertex_data,
			const Ogre::v1::VertexData *blended_data,
			const Ogre::v1::Mesh::IndexMap *indexMap);

		void addIndexData(Ogre::v1::IndexData *data, const unsigned int offset = 0);

	protected:
		Ogre::Vector3*	    mVertexBuffer;
		unsigned int*       mIndexBuffer;
		unsigned int        mVertexCount;
		unsigned int        mIndexCount;

		Ogre::Vector3		mBounds;
		Ogre::Real		    mBoundRadius;

		BoneIndex           *mBoneIndex;

		Ogre::Matrix4		mTransform;

		Ogre::Vector3		mScale;
	};

	///Shape converter for static (non-animated) meshes.
	class StaticMeshToShapeConverter : public VertexIndexToShape
	{
	public:
		StaticMeshToShapeConverter(Ogre::Renderable *rend, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Creaate a messh converter from am V2 mesh object
		StaticMeshToShapeConverter(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Create a mesh converter from a V1 mesh object
		StaticMeshToShapeConverter(Ogre::v1::Mesh *mesh, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		///Default constructor; You can add a mesh/entity later
		StaticMeshToShapeConverter();

		virtual ~StaticMeshToShapeConverter() = default;

		void addEntity(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

		void addMesh(const Ogre::v1::Mesh *mesh, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

	protected:

		Ogre::v1::Entity*		mEntity;
		Ogre::SceneNode*	mNode;
	};

	///For animated meshes.
	class AnimatedMeshToShapeConverter : public VertexIndexToShape
	{
	public:

		AnimatedMeshToShapeConverter(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
		AnimatedMeshToShapeConverter();
		virtual ~AnimatedMeshToShapeConverter();

		void addEntity(Ogre::v1::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
		void addMesh(const Ogre::v1::MeshPtr &mesh, const Ogre::Matrix4 &transform);

		btBoxShape* createAlignedBox(unsigned char bone,
			const Ogre::Vector3 &bonePosition,
			const Ogre::Quaternion &boneOrientation);

		btBoxShape* createOrientedBox(unsigned char bone,
			const Ogre::Vector3 &bonePosition,
			const Ogre::Quaternion &boneOrientation);

	protected:

		bool getBoneVertices(unsigned char bone,
			unsigned int &vertex_count,
			Ogre::Vector3* &vertices,
			const Ogre::Vector3 &bonePosition);

		bool getOrientedBox(unsigned char bone,
			const Ogre::Vector3 &bonePosition,
			const Ogre::Quaternion &boneOrientation,
			Ogre::Vector3 &extents,
			Ogre::Vector3 *axis,
			Ogre::Vector3 &center);

		Ogre::v1::Entity*		mEntity;
		Ogre::SceneNode*	mNode;

		Ogre::Vector3       *mTransformedVerticesTemp;
		size_t               mTransformedVerticesTempSize;
	};
}