#include "BtOgreExtras.h"

using namespace Ogre;
using namespace BtOgre;

btQuaternion Convert::toBullet(const Quaternion& q)
{
	return { q.x, q.y, q.z, q.w };
}

btVector3 Convert::toBullet(const Vector3& v)
{
	return { v.x, v.y, v.z };
}

Quaternion Convert::toOgre(const btQuaternion& q)
{
	return { q.w(), q.x(), q.y(), q.z() };
}

Vector3 Convert::toOgre(const btVector3& v)
{
	return { v.x(), v.y(), v.z() };
}

LineDrawer::LineDrawer(SceneNode* node, String datablockId, SceneManager* smgr) :
	attachNode(node),
	datablockToUse(datablockId),
	manualObject(nullptr),
	smgr(smgr),
	index(0)
{
}

LineDrawer::~LineDrawer()
{
	clear();
	if (manualObject)
		smgr->destroyManualObject(manualObject);
}

void LineDrawer::clear()
{
	if (manualObject) manualObject->clear();
	lines.clear();
}

void LineDrawer::addLine(const Vector3& start, const Vector3& end, const ColourValue& value)
{
	lines.push_back({ start, end, value });
}

void LineDrawer::checkForMaterial() const
{
	const auto hlmsUnlit = static_cast<HlmsUnlit*>(Root::getSingleton().getHlmsManager()->getHlms(HLMS_UNLIT));
	const auto datablock = hlmsUnlit->getDatablock(datablockToUse);

	if (datablock) return;
	LogManager::getSingleton().logMessage("BtOgre's datablock not found, creating...");
	auto createdDatablock = hlmsUnlit->createDatablock(datablockToUse, datablockToUse, {}, {}, {}, true, BLANKSTRING, DebugDrawer::BtOgre21ResourceGroup);

	if (!createdDatablock) throw std::runtime_error(std::string("BtOgre Line Drawer failed to create HLMS Unlit datablock ") + datablockToUse);
}

void LineDrawer::update()
{
	if (!manualObject)
	{
		LogManager::getSingleton().logMessage("Create manual object");
		manualObject = smgr->createManualObject(SCENE_STATIC);
		manualObject->setCastShadows(false);
		attachNode->attachObject(manualObject);
	}

	checkForMaterial();
	manualObject->begin(datablockToUse, OT_LINE_LIST);

	index = 0;
	for (const auto& l : lines)
	{
		manualObject->position(l.start);
		manualObject->colour(l.vertexColor);
		manualObject->index(index++);

		manualObject->position(l.end);
		manualObject->colour(l.vertexColor);
		manualObject->index(index++);
	}

	manualObject->end();
}

DebugDrawer::DebugDrawer(SceneNode* node, btDynamicsWorld* world, String smgrName) :
	mNode(node->createChildSceneNode(SCENE_STATIC)),
	mWorld(world),
	mDebugOn(true),
	unlitDatablockId(unlitDatablockName),
	unlitDiffuseMultiplier(1),
	stepped(false),
	scene(smgrName),
	smgr(Ogre::Root::getSingleton().getSceneManager(smgrName)),
	drawer(mNode, unlitDatablockName, smgr)
{
	init();
}

DebugDrawer::DebugDrawer(SceneNode* node, btDynamicsWorld* world, SceneManager* smgr) :
	mNode(node->createChildSceneNode(SCENE_STATIC)),
	mWorld(world),
	mDebugOn(true),
	unlitDatablockId(unlitDatablockName),
	unlitDiffuseMultiplier(1),
	stepped(false),
	scene("nonamegiven"),
	smgr(smgr),
	drawer(mNode, unlitDatablockName, smgr)
{
	init();
}

void DebugDrawer::init()
{
	if (!ResourceGroupManager::getSingleton().resourceGroupExists(BtOgre21ResourceGroup))
		ResourceGroupManager::getSingleton().createResourceGroup(BtOgre21ResourceGroup);
}

void DebugDrawer::setUnlitDiffuseMultiplier(float value)
{
	if (value >= 1) unlitDiffuseMultiplier = value;
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	if (stepped)
	{
		drawer.clear();
		stepped = false;
	}

	const auto ogreFrom = Convert::toOgre(from);
	const auto ogreTo = Convert::toOgre(to);

	ColourValue ogreColor{ color.x(), color.y(), color.z(), 1.0f };
	ogreColor *= unlitDiffuseMultiplier;

	drawer.addLine(ogreFrom, ogreTo, ogreColor);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LogManager::getSingleton().logMessage(warningString);
}

void DebugDrawer::setDebugMode(int isOn)
{
	mDebugOn = isOn;

	if (!mDebugOn)
		drawer.clear();
}

int DebugDrawer::getDebugMode() const
{
	return mDebugOn;
}

void DebugDrawer::step()
{
	if (mDebugOn)
	{
		mWorld->debugDrawWorld();
		drawer.update();
	}
	else
	{
		drawer.clear();
	}
	stepped = true;
}