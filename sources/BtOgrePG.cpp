#include "BtOgrePG.h"

using namespace Ogre;
using namespace BtOgre;

RigidBodyState::RigidBodyState(SceneNode* node, const btTransform& transform, const btTransform& offset) :
	mTransform(transform),
	mCenterOfMassOffset(offset),
	mNode(node)
{
}

RigidBodyState::RigidBodyState(SceneNode* node) :
	mTransform
	(
		node ? Convert::toBullet(node->getOrientation()) : btQuaternion(0, 0, 0, 1),
		node ? Convert::toBullet(node->getPosition()) : btVector3(0, 0, 0)
	),
	mCenterOfMassOffset(btTransform::getIdentity()),
	mNode(node)
{
}

void RigidBodyState::getWorldTransform(btTransform& ret) const
{
	ret = mTransform;
}

void RigidBodyState::setWorldTransform(const btTransform& in)
{
	if (!mNode) return;

	//store transform
	mTransform = in;

	//extract position and orientation
	const auto transform = mTransform * mCenterOfMassOffset;
	const auto rot = transform.getRotation();
	const auto pos = transform.getOrigin();

	//Set to the node
	mNode->_setDerivedOrientation({ rot.w(), rot.x(), rot.y(), rot.z() });
	mNode->_setDerivedPosition({ pos.x(), pos.y(), pos.z() });
}

void RigidBodyState::setNode(SceneNode* node)
{
	mNode = node;
}