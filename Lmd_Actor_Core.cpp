#include "g_local.h"

extern vec3_t playerMins;
extern vec3_t playerMaxs;

void SetupGameGhoul2Model(gentity_t *ent, char *modelname, char *skinName);

typedef struct actorInstance_s
{
	struct
	{
		gentity_t *entDest;
		vec3_t vecDest;
		int radius;
	} destination;
} actorInstance_t;
//TODO: handle attributes, see NPC_ParseParms

void Actor_SetOrigin(gentity_t *actor, vec3_t origin)
{
	VectorCopy(origin, actor->s.origin);
	G_SetOrigin(actor, origin);
	VectorCopy(origin, actor->client->ps.origin);
	actor->s.pos.trType = TR_INTERPOLATE;
	actor->s.pos.trTime = level.time;
}

void Actor_SetAngles(gentity_t *actor, vec3_t angles)
{	
	SetClientViewAngle(actor, angles);
}

void Actor_SetModel(gentity_t *actor, char *model, char *skin)
{
	//Make sure we don't free the model if we aren't changing the pointer which might happen if we change skin but not model).
	if (actor->model != model)
	{
		G_Free(actor->model);
		actor->model = G_NewString2(model);
	}
	SetupGameGhoul2Model(actor, model, skin);
}

void scaleEntity(gentity_t *scaleEnt, int scale);
void Actor_SetScale(gentity_t *actor, float scale)
{
	scaleEntity(actor, scale * 100);
}

void Actor_SetAnimation_Torso(gentity_t *actor, int animID, int length)
{
	G_SetAnim(actor, SETANIM_TORSO, animID, SETANIM_FLAG_RESTART | SETANIM_FLAG_HOLD | SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLDLESS, 0);
	if (length < 0)
		actor->client->ps.torsoTimer = Q3_INFINITE;
	else if (length > 0)
		actor->client->ps.torsoTimer = length;
}

void Actor_SetAnimation_Legs(gentity_t *actor, int animID, int length)
{
	G_SetAnim(actor, SETANIM_LEGS, animID, SETANIM_FLAG_RESTART | SETANIM_FLAG_HOLD | SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLDLESS, 0);
	if (length < 0)
		actor->client->ps.legsTimer = Q3_INFINITE;
	else if (length > 0)
		actor->client->ps.legsTimer = length;
}

void Actor_SetAnimation_Both(gentity_t *actor, int animID, int length)
{
	G_SetAnim(actor, SETANIM_BOTH, animID, SETANIM_FLAG_HOLD | SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLDLESS, 0);
	if (length < 0)
		actor->client->ps.torsoTimer = actor->client->ps.legsTimer = Q3_INFINITE;
	else if (length > 0)
		actor->client->ps.torsoTimer = actor->client->ps.legsTimer = length;
}

void Actor_SetSpeed(gentity_t *actor, int speed)
{
	actor->speed = speed;
}
void Actor_SetDestination(gentity_t *actor, vec3_t dest, int radius)
{
	VectorCopy(dest, actor->s.origin2);
	if (radius <= 0)
		radius = 15;
	actor->radius = radius;
}

void ClientThink(int clientNum, usercmd_t *ucmd);
void Actor_Think(gentity_t *actor)
{
	actor->client->pers.cmd.serverTime = level.time - 50;

	if (actor->speed > 0)
	{
		actor->client->Lmd.customSpeed.time = Q3_INFINITE;
		actor->client->Lmd.customSpeed.value = actor->speed;
	}
	else
		actor->client->Lmd.customSpeed.time = 0;

	vec3_t desiredLocation;
	VectorCopy(actor->s.origin2, desiredLocation);

	if (VectorCompare(desiredLocation, vec3_origin) == qfalse)
	{
		vec3_t direction;
		VectorSubtract(desiredLocation, actor->r.currentOrigin, direction);
		if (VectorLength(direction) >= actor->radius)
		{
			VectorNormalize(direction);
			Actor_SetAngles(actor, direction);
			VectorScale(direction, 25, direction);
			actor->client->pers.cmd.forwardmove = direction[0];
			actor->client->pers.cmd.rightmove = direction[1];
		}
	}

	ClientThink(actor->s.number, &actor->client->pers.cmd);
	actor->nextthink = level.time + FRAMETIME / 2;
}

gentity_t *Actor_Create(char *model, char *skin, vec3_t origin, vec3_t angles)
{
	gentity_t *actor = G_Spawn();
	if (!actor)
		return NULL;

	actor->classname = "Actor";
	actor->s.eType = ET_NPC;

	actor->client = (gclient_t *)G_Alloc(sizeof(gclient_t));
	if (actor->client == NULL)
	{
		Com_Printf("ERROR: ACTOR G_Alloc client failed\n");
		G_FreeEntity(actor);
		return NULL;
	}

	//actor->client->NPC_class = CLASS_PRISONER; //CLASS_NONE;

	actor->playerState = &actor->client->ps;
	actor->client->ps.clientNum = actor->s.number;
	actor->client->ps.weapon = WP_NONE;
	actor->client->ps.pm_type = PM_FREEZE;

	actor->client->ps.persistant[PERS_TEAM] = actor->client->sess.sessionTeam = TEAM_FREE;

	Actor_SetOrigin(actor, origin);
	Actor_SetAngles(actor, angles);
	Actor_SetModel(actor, model, skin);

	VectorCopy(playerMins, actor->r.mins);
	VectorCopy(playerMaxs, actor->r.maxs);
	actor->client->ps.crouchheight = CROUCH_MAXS_2;
	actor->client->ps.standheight = DEFAULT_MAXS_2;

	actor->client->renderInfo.headYawRangeLeft = 80;
	actor->client->renderInfo.headYawRangeRight = 80;
	actor->client->renderInfo.headPitchRangeUp = 45;
	actor->client->renderInfo.headPitchRangeDown = 45;
	actor->client->renderInfo.torsoYawRangeLeft = 60;
	actor->client->renderInfo.torsoYawRangeRight = 60;
	actor->client->renderInfo.torsoPitchRangeUp = 30;
	actor->client->renderInfo.torsoPitchRangeDown = 50;
	actor->client->renderInfo.lookTarget = ENTITYNUM_NONE;
	VectorCopy(actor->r.currentOrigin, actor->client->renderInfo.eyePoint);

	actor->r.contents = CONTENTS_BODY;
	actor->clipmask = MASK_NPCSOLID;

	actor->s.groundEntityNum = ENTITYNUM_NONE;

	actor->mass = 10;
	actor->s.owner = ENTITYNUM_NONE; // yellow crosshair

	//Need to do this, else we cant move
	actor->client->ps.stats[STAT_HEALTH] = actor->health = 100;

	G_SetAnim(actor, SETANIM_BOTH, BOTH_STAND1, SETANIM_FLAG_NORMAL, 0);

	trap_LinkEntity(actor);

	actor->think = Actor_Think;
	Actor_Think(actor);

	return actor;
}