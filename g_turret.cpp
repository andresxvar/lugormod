// Copyright (C) 1999-2000 Id Software, Inc.
// New turret code 2020 WX
#include "g_local.h"
#include "q_shared.h"
#include "Lmd_Entities_Public.h"

// misc_turret_pain
void misc_turret_pain(gentity_t *turret, gentity_t *attacker, int damage)
{
	gentity_t *turretBase = turret->target_ent;
	if (turretBase != NULL)
	{
		turretBase->health = turret->health;
		if (turretBase->maxHealth)
			G_ScaleNetHealth(turretBase);
	}

	if (attacker->client && attacker->client->ps.weapon == WP_DEMP2)
	{
		// TODO: figure out what this old code is supposed to do...
		turret->attackDebounceTime = level.time + 800 + random() * 500;
		turret->painDebounceTime = turret->attackDebounceTime;
	}

	// set turret's enemy to the attacker
	if (turret->enemy == NULL && !(attacker->flags & FL_NOTARGET))
	{
		turret->enemy = attacker;
	}
}

// misc_turret_die
void ObjectDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath);
void misc_turret_die(gentity_t *turret, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath)
{
	// turn everything off
	turret->think = NULL;
	turret->use = NULL;
	turret->die = NULL;
	turret->takedamage = qfalse;
	turret->s.health = turret->health = 0;
	turret->s.loopSound = 0;
	turret->s.shouldtarget = qfalse;
	turret->s.weapon = 0; // crosshair code uses this to mark crosshair red

	// play explosion effect
	vec3_t forward = {0, 0, 1}, pos;
	VectorCopy(turret->r.currentOrigin, pos);
	pos[2] += turret->r.maxs[2] * 0.5f;
	G_PlayEffectID(G_EffectIndex("turret/explode"), pos, forward);

	// do splashdamage
	if (turret->splashDamage > 0 && turret->splashRadius > 0)
	{
		G_RadiusDamage(turret->r.currentOrigin, attacker,
					   turret->splashDamage, turret->splashRadius, attacker,
					   NULL, MOD_UNKNOWN);
	}

	// switch to damaged models
	if (turret->s.modelindex2)
	{
		turret->s.modelindex = turret->s.modelindex2;

		gentity_t *turretBottom = turret->target_ent;
		if (turretBottom && turretBottom->s.modelindex2)
			turretBottom->s.modelindex = turretBottom->s.modelindex2;

		// fire target
		if (VALIDSTRING(turret->target))
			G_UseTargets2(turret, attacker, turret->target);
	}
	else
	{
		// when the turret does not have a damaged model
		// it will just fire targets and free itself
		ObjectDie(turret, inflictor, attacker, damage, meansOfDeath);
	}
}

// misc_turret_fire
#define START_DIS 15
void misc_turret_fire(gentity_t *turret, vec3_t start, vec3_t dir)
{
	// create a bolt entity for the turret's shot
	gentity_t *bolt = G_Spawn();

	if (bolt == NULL)
		return;

	// the shot should not start in a solid
	/*if ((g_syscall(G_POINT_CONTENTS, start, ent->s.number)&MASK_SHOT))
		return;*/

	// play muzzle effect
	vec3_t pos;
	VectorMA(start, -START_DIS, dir, pos);
	G_PlayEffectID(turret->genericValue13, pos, dir);

	// bolt attributes
	bolt->s.otherEntityNum2 = turret->genericValue14; // efx
	bolt->s.emplacedOwner = turret->genericValue15;	  // efx
	bolt->s.weapon = turret->s.weapon;
	bolt->r.ownerNum = turret->s.number;
	bolt->damage = turret->damage;
	bolt->alliedTeam = turret->alliedTeam;
	bolt->teamnodmg = turret->teamnodmg;
	bolt->splashDamage = turret->damage;
	bolt->classname = "turret_proj";
	bolt->s.eType = ET_MISSILE;
	//bolt->dflags = DAMAGE_NO_KNOCKBACK;// | DAMAGE_HEAVY_WEAP_CLASS;	// Don't push them around, or else we are constantly re-aiming
	bolt->splashRadius = 100;
	bolt->methodOfDeath = MOD_TARGET_LASER;
	bolt->splashMethodOfDeath = MOD_TARGET_LASER;
	bolt->clipmask = MASK_SHOT; // | CONTENTS_LIGHTSABER;
	VectorSet(bolt->r.maxs, 1.5, 1.5, 1.5);
	VectorScale(bolt->r.maxs, -1, bolt->r.mins);
	bolt->s.pos.trTime = level.time;
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, turret->mass, bolt->s.pos.trDelta);
	if (bolt->s.weapon == WP_THERMAL)
	{
		bolt->s.pos.trType = TR_GRAVITY;
		bolt->s.pos.trDelta[2] += 40.0f; //give a slight boost in the upward direction
	}
	else
		bolt->s.pos.trType = TR_LINEAR;
	
	SnapVector(bolt->s.pos.trDelta); // save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);
	bolt->parent = turret;

	// free bolt after some time
	bolt->nextthink = level.time + 10000;
	bolt->think = G_FreeEntity;
}

// misc_turret_aim
void misc_turret_aim(gentity_t *turret)
{
	vec3_t desiredAngles, deltaAngles;
	float deltaYaw = 0.0f, deltaPitch = 0.0f;
	const float pitchCap = 40.0f;

	// move turret
	BG_EvaluateTrajectory(&turret->s.apos, level.time, turret->r.currentAngles);
	turret->r.currentAngles[YAW] = AngleNormalize180(turret->r.currentAngles[YAW]);
	turret->r.currentAngles[PITCH] = AngleNormalize180(turret->r.currentAngles[PITCH]);
	float turnSpeed = turret->speed;

	if (turret->painDebounceTime > level.time)
	{
		desiredAngles[YAW] = turret->r.currentAngles[YAW] + flrand(-45, 45);
		desiredAngles[PITCH] = turret->r.currentAngles[PITCH] + flrand(-10, 10);

		if (fabs(desiredAngles[PITCH]) > pitchCap)
			desiredAngles[PITCH] = desiredAngles[PITCH] > 0 ? pitchCap : -pitchCap;

		deltaYaw = AngleSubtract(desiredAngles[YAW], turret->r.currentAngles[YAW]);
		deltaPitch = AngleSubtract(desiredAngles[PITCH], turret->r.currentAngles[PITCH]);
		turnSpeed = flrand(-5, 5);
	}
	else if (turret->enemy)
	{
		// aim at the enemy

		// get enemy's position
		vec3_t enemyDir, enemyOrigin;
		VectorCopy(turret->enemy->r.currentOrigin, enemyOrigin);
		enemyOrigin[2] += turret->enemy->r.maxs[2] * 0.5f;
		// walkers are taller TODO: perhaps its better to use enemy's hitbox to generalize where to aim
		if (turret->enemy->s.eType == ET_NPC &&
			turret->enemy->s.NPC_class == CLASS_VEHICLE &&
			turret->enemy->m_pVehicle &&
			turret->enemy->m_pVehicle->m_pVehicleInfo->type == VH_WALKER)
		{
			enemyOrigin[2] += 32.0f;
		}

		// get forward direction to the enemy
		VectorSubtract(enemyOrigin, turret->r.currentOrigin, enemyDir);

		// transform direction to angles for the turret
		vectoangles(enemyDir, desiredAngles);
		desiredAngles[PITCH] = AngleNormalize180(desiredAngles[PITCH]);
		if (desiredAngles[PITCH] < -pitchCap)
			desiredAngles[PITCH] = -pitchCap;
		else if (desiredAngles[PITCH] > pitchCap)
			desiredAngles[PITCH] = pitchCap;
		deltaYaw = AngleSubtract(desiredAngles[YAW], turret->r.currentAngles[YAW]);
		deltaPitch = AngleSubtract(desiredAngles[PITCH], turret->r.currentAngles[PITCH]);
	}
	else
	{
		//FIXME: Pan back and forth in original facing
		// no enemy, so make us slowly sweep back and forth as if searching for a new one
		desiredAngles[YAW] = sin(level.time * 0.0001f + turret->count);
		desiredAngles[YAW] *= 60.0f;
		desiredAngles[YAW] += turret->target_ent->s.angles[YAW];
		desiredAngles[YAW] = AngleNormalize180(desiredAngles[YAW]);
		deltaYaw = AngleSubtract(desiredAngles[YAW], turret->r.currentAngles[YAW]);
		deltaPitch = AngleSubtract(0, turret->r.currentAngles[PITCH]);
		turnSpeed = 1.0f;
	}

	// cap turnspeed
	if (deltaYaw)
	{
		if (fabs(deltaYaw) > turnSpeed)
			deltaYaw = (deltaYaw >= 0 ? turnSpeed : -turnSpeed);
	}

	if (deltaPitch)
	{
		if (fabs(deltaPitch) > turnSpeed)
			deltaPitch = (deltaPitch > 0.0f ? turnSpeed : -turnSpeed);
	}

	VectorSet(deltaAngles, deltaPitch, deltaYaw, 0);
	VectorCopy(turret->r.currentAngles, turret->s.apos.trBase);
	VectorScale(deltaAngles, (1000 / FRAMETIME), turret->s.apos.trDelta);
	turret->s.apos.trTime = level.time;
	turret->s.apos.trType = TR_LINEAR_STOP;
	turret->s.apos.trDuration = FRAMETIME;

	if (deltaPitch || deltaYaw)
		turret->s.loopSound = G_SoundIndex("sound/vehicles/weapons/hoth_turret/turn.wav");
	else
		turret->s.loopSound = 0;
}

// TODO: misc_turret_turnOff
// TODO: misc_turret_sleep

// misc_turret_findEnemies
// called from turret_base_think when the turret
// doesn't have an enemy assigned.
// returns boolean to state wether an enemy was found
qboolean isBuddy(gentity_t *ent, gentity_t *other);
qboolean misc_turret_findEnemies(gentity_t *turret)
{
	qboolean found = qfalse;

	trace_t tr;
	gentity_t *entity_list[MAX_GENTITIES] = {0};
	gentity_t *target = NULL, *bestTarget = NULL;
	vec3_t aimDir, start, end;
	int count = 0;
	float bestDist = turret->radius * turret->radius;
	float enemyDist;

	// log aim timestamp
	if (turret->aimDebounceTime > level.time &&
		turret->timestamp < level.time)
	{
		turret->timestamp = level.time + 1000;
	}

	VectorCopy(turret->r.currentOrigin, start);
	// list damagable enetities in radius
	count = G_RadiusList(start, turret->radius, turret, qtrue, entity_list, qfalse);

	for (int i = 0; i < count; i++)
	{
		target = entity_list[i];

		// check valid target
		if (!target->client || target->health <= 0 || (target->flags & FL_NOTARGET) || target->client->sess.sessionTeam == TEAM_SPECTATOR || target->client->tempSpectate >= level.time)
		{
			continue;
		}

		// check turret team specifications
		if (turret->alliedTeam && (target->client->sess.sessionTeam == turret->alliedTeam || target->teamnodmg == turret->alliedTeam))
		{
			continue;
		}

		// check turret activator
		if (turret->activator && (target == turret->activator || isBuddy(turret->activator, target)))
		{
			continue;
		}

		// check pvs
		if (!trap_InPVS(start, target->r.currentOrigin))
			continue;

		// check for clear shot
		VectorCopy(target->r.currentOrigin, end);
		end[2] += target->r.maxs[2] * 0.5f;
		trap_Trace(&tr, start, NULL, NULL, end, turret->s.number, MASK_SHOT);
		if (!tr.allsolid && !tr.startsolid && (tr.fraction == 1.0 || tr.entityNum == target->s.number))
		{
			VectorSubtract(target->r.currentOrigin, turret->r.currentOrigin, aimDir);
			enemyDist = VectorLengthSquared(aimDir);
			if (enemyDist < bestDist)
			{
				if (turret->attackDebounceTime < level.time)
					turret->attackDebounceTime = level.time + 1400;
				bestTarget = target;
				bestDist = enemyDist;
				found = qtrue;
			}
		}
	}
	if (found)
	{
		if (VALIDSTRING(turret->target2))
			G_UseTargets2(turret, turret, turret->target2);
		turret->enemy = bestTarget;
	}
	return found;
}

// misc_turret_think
void misc_turret_think(gentity_t *turret)
{
	qboolean turnOff = qtrue;

	turret->nextthink = level.time + FRAMETIME;

	if (turret->enemy == NULL)
	{
		if (misc_turret_findEnemies(turret))
			turnOff = qfalse;
	}
	else if (turret->enemy->client && (turret->enemy->client->sess.sessionTeam == TEAM_SPECTATOR || turret->enemy->client->tempSpectate >= level.time))
	{
		turret->enemy = NULL;
	}
	else
	{
		// 25% chance of still firing
		if (turret->painDebounceTime > level.time && Q_irand(0, 3))
			return;

		//fire
		if (turret->enemy && turret->setTime < level.time && turret->attackDebounceTime < level.time)
		{
			turret->setTime = level.time + turret->wait; // next fire time
			vec3_t fwd, muzzlePos;
			VectorCopy(turret->r.currentOrigin, muzzlePos);
			muzzlePos[2] += turret->r.maxs[2] - 8;

			AngleVectors(turret->r.currentAngles, fwd, NULL, NULL);
			VectorMA(muzzlePos, START_DIS, fwd, muzzlePos);
			misc_turret_fire(turret, muzzlePos, fwd);
			turret->fly_sound_debounce_time = level.time; // lastShotTime
		}
	}

	if (turnOff)
	{
		// bounceCount is used to keep the thing from ping-ponging from on to off
		if (turret->bounceCount < level.time)
		{
			if (turret->enemy != NULL)
			{
				// make turret play ping sound for 5 seconds
				turret->aimDebounceTime = level.time + 5000;
				turret->enemy = NULL;
			}
		}
	}
	else
	{
		// keep our enemy for a minimum of 2 seconds from now
		turret->bounceCount = level.time + 2000 + random() * 150;
	}

	misc_turret_aim(turret);
}

// misc_turret_use
// toggles turret on and off
void misc_turret_use(gentity_t *turret, gentity_t *other, gentity_t *activator)
{

	turret->flags ^= FL_DONT_SHOOT;

	if (turret->flags & FL_DONT_SHOOT)
		turret->nextthink = Q3_INFINITE;
	else
	{
		turret->activator = activator;
		turret->nextthink = level.time + FRAMETIME * 5;
	}
}

// misc_turret_free
// removes the turret base whenever the Turret is freed
void misc_turret_free(gentity_t *turret)
{
	gentity_t *turretBase = turret->target_ent;

	if (turretBase)
		G_FreeEntity(turretBase);
}

/*QUAKED misc_turret (1 0 0) (-48 -48 0) (48 48 144) START_OFF
Large 2-piece turbolaser turret

START_OFF - Starts off

radius - How far away an enemy can be for it to pick it up (default 1024)
wait	- Time between shots (default 300 ms)
dmg	- How much damage each shot does (default 100)
health - How much damage it can take before exploding (default 3000)
speed - how fast it turns (default 10)

splashDamage - How much damage the explosion does (300)
splashRadius - The radius of the explosion (128)

shotspeed - speed at which projectiles will move

targetname - Toggles it on/off
target - What to use when destroyed
target2 - What to use when it decides to start shooting at an enemy

showhealth - set to 1 to show health bar on this entity when crosshair is over it

teamowner - crosshair shows green for this team, red for opposite team
0 - none
1 - red
2 - blue

alliedTeam - team that this turret won't target
0 - none
1 - red
2 - blue

teamnodmg - team that turret does not take damage from
0 - none
1 - red
2 - blue

"icon" - icon that represents the objective on the radar
*/

const entityInfoData_t misc_turret_spawnflags[] =
	{
		{"1", "Start disabled."},
		NULL};

const entityInfoData_t misc_turret_keys[] =
	{
		{"radius", "How far away an enemy can be for it to pick it up.  Default: 1024."},
		{"wait", "Time between shots in milliseconds. Default: 300."},
		{"dmg", "How much damage each shot does. Default: 100."},
		{"health", "How much damage it can take before exploding.  Default: 3000."},
		{"speed", "How fast the turret can aim.  Default: 10."},
		{"splashDamage", "How much splash damage is done by the projectiles.  Default: 300."},
		{"splashRadius", "The radius of the projectile explosion.  Default: 128"},
		{"shotspeed", "How fast the projectile moves."},
		{"targetname", "Toggles on / off when triggered."},
		{"target", "The targetname to fire when the turret is destroyed."},
		{"target2", "The targetname to fire when the turret locks on to an enemy."},

		{NULL, NULL}};

entityInfo_t misc_turret_info =
	{
		"A Hoth style turret.  Uses 2 entities.",
		misc_turret_spawnflags,
		misc_turret_keys};

void SP_misc_turret(gentity_t *turret)
{
	char *out = NULL; // spawnstring variables

	gentity_t *turretBase = G_Spawn();

	if (turretBase == NULL)
	{ // turret base failed to spawn
		G_FreeEntity(turret);
		return;
	}

	// set entity type for both turret top and base
	turretBase->s.eType = turret->s.eType = ET_GENERAL;

	// turret top model
	if (turret->model)
	{
		turret->s.modelindex = G_ModelIndex(turret->model);
		G_SpawnInt("modelscale", "0", &turret->s.iModelScale);
	}
	else
	{
		turret->s.modelindex = G_ModelIndex("models/map_objects/hoth/turret_top_new.md3");
		// turret top destroyed model
		turret->s.modelindex2 = G_ModelIndex("models/map_objects/hoth/turret_top.md3");
	}

	// turret base model
	if (turret->model2)
	{
		turretBase->s.modelindex = G_ModelIndex(turret->model2);
	}
	else
	{
		turretBase->s.modelindex = G_ModelIndex("models/map_objects/hoth/turret_base.md3");
		// turret base destroyed model
		turretBase->s.modelindex2 = G_ModelIndex("models/map_objects/hoth/turret_bottom.md3");
	}

	// precache special effects and sounds
	G_SoundIndex("sound/vehicles/weapons/hoth_turret/turn.wav");
	G_EffectIndex("turret/explode");
	G_EffectIndex("sparks/spark_exp_nosnd");
	G_EffectIndex("turret/hoth_muzzle_flash");
	turret->genericValue13 = G_EffectIndex("turret/hoth_muzzle_flash"); // muzzle flash
	G_SpawnString("shotfx", "turret/hoth_shot", &out);					// shot fx
	turret->genericValue14 = G_EffectIndex(out);
	G_SpawnString("impactfx", "turret/hoth_impact", &out); // shot impact fx
	turret->genericValue15 = G_EffectIndex(out);

	// turret top physical attributes
	turret->material = MAT_METAL;
	turret->r.contents = CONTENTS_BODY;
	VectorSet(turret->r.maxs, 48.0f, 48.0f, 16.0f);
	VectorSet(turret->r.mins, -48.0f, -48.0f, 0.0f);

	// turret bottom physical attributes
	turretBase->r.contents = CONTENTS_BODY;
	VectorSet(turretBase->r.maxs, 32.0f, 32.0f, 128.0f);
	VectorSet(turretBase->r.mins, -32.0f, -32.0f, 0.0f);
	//base->r.svFlags |= SVF_NO_TELEPORT|SVF_NONNPC_ENEMY|SVF_SELF_ANIMATING;

	// positioning
	G_SetOrigin(turretBase, turret->s.origin);
	turret->s.origin[2] += 128; // move top above the base
	G_SetOrigin(turret, turret->s.origin);
	G_SetAngles(turret, turret->s.angles);
	G_SetAngles(turretBase, turret->s.angles);

	// link top and base
	turretBase->target_ent = turret;
	turretBase->r.ownerNum = turret->s.number;
	turret->target_ent = turretBase;
	turret->r.ownerNum = turretBase->s.number;

	// team attributes
	if (VALIDSTRING(turret->team) && !turret->teamnodmg)
		turret->teamnodmg = atoi(turret->team);
	if (!turret->s.teamowner)
		turret->s.teamowner = turret->alliedTeam;
	// team attributes of the base TODO: some of these are not necessary to set
	turretBase->team = NULL;
	turretBase->teamnodmg = turret->teamnodmg;
	turretBase->alliedTeam = turret->alliedTeam;
	turretBase->s.teamowner = turret->s.teamowner;

	// damage attributes
	turret->s.shouldtarget = qtrue;
	if (!turret->health)
		turret->health = 3000;
	turretBase->s.shouldtarget = qtrue;
	turretBase->health = turret->health;
	int tmp = 0;
	G_SpawnInt("showhealth", "0", &tmp);
	if (tmp)
	{
		turret->maxHealth = turret->health;
		G_ScaleNetHealth(turret);
		turretBase->maxHealth = turretBase->health;
		G_ScaleNetHealth(turretBase);
	}
	turretBase->takedamage = qtrue;
	turret->takedamage = qtrue;
	turretBase->damageRedirect = qtrue;
	turretBase->damageRedirectTo = turret->s.number;
	turret->pain = misc_turret_pain;
	turret->die = misc_turret_die;

	// shooting attributes
	turret->count = random() * 9000; // time offset for enemy search mode
	G_SpawnFloat("shotspeed", "1100", &turret->mass);
	if (!turret->radius)
		turret->radius = 1024; // search radius
	if (!turret->wait)
		turret->wait = 300 + random() * 55; // shot fire rate
	if (!turret->splashDamage)
		turret->splashDamage = 300;
	if (!turret->splashRadius)
		turret->splashRadius = 128;
	if (!turret->damage)
		turret->damage = 100; // shot damage
	if (!turret->speed)
		turret->speed = 20; // turning speed
	G_SpawnInt("weapon", "17", &turret->s.weapon);

	// usability
	turret->use = misc_turret_use;
	if (turret->spawnflags & 1)
	{
		turret->think = NULL;
		turret->nextthink = Q3_INFINITE;
	}
	else
	{
		turret->think = misc_turret_think;
		turret->nextthink = level.time + FRAMETIME * 5;
	}

	// free entity
	turret->freeEntity = misc_turret_free;

	trap_LinkEntity(turret);
	trap_LinkEntity(turretBase);
}