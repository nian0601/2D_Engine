#pragma once

enum FW_CollisionDirection
{
	CLEAR,
	LEFTSIDE,
	RIGHTSIDE,
	TOPSIDE,
	BOTTOMSIDE,
	COLLISION,
};

struct FW_CollisionMessage
{
	FW_EntityID myEntity;
	FW_EntityID myEntityThatHitMe;

	FW_CollisionDirection myCollisionDirection;
};

//////////////////////////////////////////////////////////////////////////

struct FW_EntityCreatedMessage
{
	FW_EntityID myEntity;
	Vector2f myPosition;
};

//////////////////////////////////////////////////////////////////////////

struct FW_PreEntityRemovedMessage
{
	FW_EntityID myEntity;
};