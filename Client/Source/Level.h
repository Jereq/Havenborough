#pragma once
#include "CheckpointSystem.h"
#include "IPhysics.h"
#include "ResourceManager.h"
#include <ActorFactory.h>

class Level
{
public:
private:
	IPhysics* m_Physics;
	ResourceManager* m_Resources;
	ActorFactory* m_ActorFactory;
	Vector3 m_StartPosition;
	Vector3 m_GoalPosition;

	CheckpointSystem m_CheckpointSystem;

public:
	/*
	* Get the starting position for a player in cm.
	*
	* @return an XMFLOAT3 with the coordinates in cm
	*/
	const Vector3 &getStartPosition(void) const;

	/*
	* Sets the starting position for a player in cm.
	*
	* @param p_StartPosition not in relative coordinates in cm
	*/
	void setStartPosition(const Vector3 &p_StartPosition);

	/*
	* Get the position of the goal in cm.
	*
	* @return an XMFLOAT3 with the coordinates in cm
	*/
	const Vector3 &getGoalPosition(void) const;

	/*
	* Set the position of the goal in cm
	*
	* @param p_GoalPosition not in relative coordinate in cm
	*/
	void setGoalPosition(const Vector3 &p_GoalPosition);

	/**
	 * Default constructor
	 */
	Level(){}

	/**
	 * Constructor
	 *
	 * @param p_Resources, Creates a reference to the main resource source. 
	 * @param p_Physics, Creates a reference to the main physic source.
	 **/
	Level(ResourceManager* p_Resources, IPhysics* p_Physics, ActorFactory* p_ActorFactory);

	/**
	 * Destructor
	 **/
	~Level();

	/**
	 * Releases resources and deallocates vector memory.
	 **/
	void releaseLevel();

	/**
	 * Loads a .txl file and send the information to Physics and Graphics, 
	 * the collision map is also a .txl file but loads information about collision.
	 *
	 * @param p_LevelFilePath the complete path to the environment .txl file.
	 */
	bool loadLevel(std::istream& p_LevelData, std::vector<Actor::ptr>& p_ActorOut);

	/**
	 * Checks if the finish line been reached.
	 *
	 * @return true if finish line been reached, false if not
	 */
	bool reachedFinishLine();

	/**
	* Gets the current checkpoint's body handle.
	* @return the BodyHandle
	*/
	BodyHandle getCurrentCheckpointBodyHandle(void);
	
	/**
	* Removes the current checkpoint and sets the next checkpoint in the vector to the current checkpoint with corresponding 
	* tone to be shaded with.
	* @param p_Objects vector with Actor objects for the current moment
	*/
	void changeCheckpoint(std::vector<Actor::ptr> &p_Objects);

private:
	Actor::ptr createObjectActor(std::string p_MeshName, Vector3 p_Position, Vector3 p_Rotation, Vector3 p_Scale);
	Actor::ptr createCollisionActor(std::string p_MeshName, Vector3 p_Translation, Vector3 p_Rotation, Vector3 p_Scale);
};