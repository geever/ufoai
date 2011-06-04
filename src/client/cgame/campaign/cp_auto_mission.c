/**
 * @file cp_auto_mission.c
 * @brief single player automatic (quick, simulated) missions, without going to the battlescape.
 */

/*
Copyright (C) 2002-2011 UFO: Alien Invasion.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "cp_auto_mission.h"
#include "cp_campaign.h"
#include "../../../shared/mathlib_extra.h"
#include "math.h"

/**
 * @brief Clears, initializes, or resets a single auto mission, sets default values.
 * @param[in,out] battle The battle that should be initialized to defaults
 */
void CP_AutoBattleClearBattle (autoMissionBattle_t *battle)
{
	int team;
	for (team = 0; team < MAX_ACTIVETEAM; team++) {
		int otherTeam;
		int soldier;

		battle->teamID[team] = -1;
		battle->teamActive[team] = qfalse;
		battle->nUnits[team] = 0;
		battle->scoreTeamDifficulty[team] = 0.5;
		battle->scoreTeamEquipment[team] = 0.5;
		battle->scoreTeamSkill[team] = 0.5;
		battle->teamAccomplishment[team] = 0;
		battle->resultType = AUTOMISSION_RESULT_NONE;

		for (otherTeam = 0; otherTeam < MAX_ACTIVETEAM; otherTeam++) {
			/* If you forget to set this and run a battle, everyone will just kill each other by default */
			battle->isHostile[team][otherTeam] = qtrue;
		}

		for (soldier = 0; soldier < MAX_SOLDIERS_AUTOMISSION; soldier++) {
			battle->unitHealth[team][soldier] = 0;
			battle->unitHealthMax[team][soldier] = 0;
		}
	}

	battle->winningTeam = -1;
	battle->isRescueMission = qfalse;
	battle->teamToRescue = -1;
	battle->teamNeedingRescue = -1;
	battle->hasBeenFought = qfalse;
}

/**
 * @brief Adds team data for a specified team in an auto-mission object, from a (player) aircraft.
 * @param[in,out] battle The auto mission battle to add team data to
 * @param[in] teamNum The team number in the auto mission instance to update
 * @param[in] aircraft The aircraft to get data from
 * @param[in] campaign The current campaign (for retrieving difficulty level)
 * @note This function actually gets the data from the campaign object, using the aircraft data to
 * find out which of all the employees are on the aircraft (in the mission)
 */
void CP_AutoBattleFillTeamFromAircraft (autoMissionBattle_t *battle, const int teamNum, const aircraft_t *aircraft, const campaign_t *campaign)
{
	employee_t *employee;
	int teamSize;
	int unitsAlive;

	assert(teamNum >= 0 && teamNum < MAX_ACTIVETEAM);
	assert(battle != NULL);
	assert(aircraft != NULL);

	teamSize = 0;
	unitsAlive = 0;
	LIST_Foreach(aircraft->acTeam, employee_t, employee) {
		const character_t *chr = &employee->chr;

		battle->unitHealthMax[teamNum][teamSize] = chr->maxHP;
		battle->unitHealth[teamNum][teamSize] = chr->HP;
		teamSize++;
		if (chr->HP > 0)
			unitsAlive++;

		if (teamSize >= MAX_SOLDIERS_AUTOMISSION)
			break;
	}
	battle->nUnits[teamNum] = teamSize;

	if (teamSize == 0) {
		Com_DPrintf(DEBUG_CLIENT, "Warning: Attempt to add soldiers to an auto-mission from an aircraft with no soldiers onboard.");
		Com_DPrintf(DEBUG_CLIENT, "--- Note: Aliens might win this mission by default because they are un-challenged, with no resistance!");
	}
	if (unitsAlive == 0) {
		Com_DPrintf(DEBUG_CLIENT, "Warning: Attempt to add team to auto battle where all the units on the team are DEAD!");
		Com_DPrintf(DEBUG_CLIENT, "--- Note: This team will LOSE the battle by default.");
	}

	if (teamSize > 0)
		battle->teamActive[teamNum] = qtrue;

	/* Set a few defaults.  These can be overridden later with new values if needed. */
	battle->teamType[teamNum] = AUTOMISSION_TEAM_TYPE_PLAYER;

	/* NOTE:  For now these are hard-coded to values based upon general campaign difficulty.
	 * --- In the future, it might be easier to set this according to a scripted value in a .ufo
	 * --- file, with other campaign info.  Reminder:  Higher floating point values mean better
	 * --- soldiers, and therefore make an easier fight for the player. */
	switch (campaign->difficulty) {
		case 4:
			battle->scoreTeamDifficulty[teamNum] = 0.20;
			break;
		case 3:
			battle->scoreTeamDifficulty[teamNum] = 0.30;
			break;
		case 2:
			battle->scoreTeamDifficulty[teamNum] = 0.40;
			break;
		case 1:
			battle->scoreTeamDifficulty[teamNum] = 0.45;
			break;
		case 0:
			battle->scoreTeamDifficulty[teamNum] = 0.50;
			break;
		case -1:
			battle->scoreTeamDifficulty[teamNum] = 0.55;
			break;
		case -2:
			battle->scoreTeamDifficulty[teamNum] = 0.60;
			break;
		case -3:
			battle->scoreTeamDifficulty[teamNum] = 0.70;
			break;
		case -4:
			battle->scoreTeamDifficulty[teamNum] = 0.80;
			break;
		default:
			battle->scoreTeamDifficulty[teamNum] = 0.50;
	}

	/* Zero is default ID for human player team ID, at least for auto missions. */
	battle->teamID[teamNum] = 0;
}

/* These are "placeholders" for the functions not yet written out, to (hopefully) allow compiling the project without error (for now) */
void CP_AutoBattleFillTeamFromAircraftUGVs (autoMissionBattle_t *battle, const int teamNum, const aircraft_t *aircraft, const campaign_t *campaign)
{
	/** @todo */
}

void CP_AutoBattleFillTeamFromPlayerBase (autoMissionBattle_t *battle, const int teamNum, const int baseNum, const campaign_t *campaign)
{
	/** @todo */
}

void CP_AutoBattleFillTeamFromAlienGroup (autoMissionBattle_t *battle, const int teamNum, const alienTeamGroup_t *alienGroup)
{
	/** @todo */
}

void CP_AutoBattleCreateTeamFromScratch (autoMissionBattle_t *battle, const int teamNum)
{
	/** @todo */
}

void CP_AutoBattleDecideResults (autoMissionBattle_t *battle)
{
	/** @todo */
	if (!battle->hasBeenFought)
		Com_Error(ERR_DROP, "Error:  Attempt to determine winning team from an auto mission that wasn't fought!");

	/* Figure out who's who (determine which team is the player and which one is aliens.) */
	int team;
	int teamPlayer = -1;
	int teamEnemy = -1;
	int teamUGVs = -1;
	int teamDrones = -1;
	for (team = 0; team < MAX_ACTIVETEAM; team++) {
		if (battle->teamType[team] == AUTOMISSION_TEAM_TYPE_PLAYER)
			teamPlayer = team;
		if (battle->teamType[team] == AUTOMISSION_TEAM_TYPE_ALIEN)
			teamEnemy = team;
		if (battle->teamType[team] == AUTOMISSION_TEAM_TYPE_PLAYER_UGV)
			teamUGVs = team;
		if (battle->teamType[team] == AUTOMISSION_TEAM_TYPE_ALIEN_DRONE)
			teamDrones = team;
	}

	assert(teamPlayer >= 0);
	assert(teamEnemy >= 0);

	/* If UGVs or alien drones fought in the battle, add their scores to that of the soldiers and aliens that control them. */
	if (teamUGVs >= 0)
		battle->teamAccomplishment[teamPlayer] += battle->teamAccomplishment[teamUGVs];
	if (teamDrones >= 0)
		battle->teamAccomplishment[teamEnemy] += battle->teamAccomplishment[teamDrones];

	/* Well, if the player team is all dead, we know the player totally lost, and can stop right here. */
	if (battle->teamActive[teamPlayer] == qfalse) {
		battle->resultType = AUTOMISSION_RESULT_FAILED_NO_SURVIVORS;
		battle->winningTeam = teamEnemy;
		return;
	} else {
		battle->winningTeam = teamPlayer;
		/* At least some player soldiers are still standing, but how good did they do? */
		if (battle->teamAccomplishment[teamPlayer] > battle->teamAccomplishment[teamEnemy])
			battle->resultType = AUTOMISSION_RESULT_SUCCESS;
		else
			battle->resultType = AUTOMISSION_RESULT_COSTLY_SUCCESS;
	}
}

#define AM_IsPlayer(type) ((type) == AUTOMISSION_TEAM_TYPE_PLAYER || (type) == AUTOMISSION_TEAM_TYPE_PLAYER_UGV)
#define AM_IsAlien(type)  ((type) == AUTOMISSION_TEAM_TYPE_ALIEN || (type) == AUTOMISSION_TEAM_TYPE_ALIEN_DRONE)
#define AM_IsCivilian(type) ((type) == AUTOMISSION_TEAM_TYPE_CIVILIAN)
#define AM_SetHostile(battle, team, otherTeam, value) (battle)->isHostile[(team)][(otherTeam)] = (value)

/**
 * @brief Run this on an auto mission battle before the battle is actually simulated with @c CP_AutoBattleRunBattle, to set
 * default values for who will attack which team.  If you forget to do this before battle simulation, all teams will default
 * to "free for all" (Everyone will try to kill everyone else).
 * @param[in, out] battle The battle to set up team hostility values for.
 * @param[in] civsInfected Set to @c true if civs have XVI influence, otherwise @c false for a normal mission.
 */
void CP_AutoBattleSetDefaultHostilities (autoMissionBattle_t *battle, const qboolean civsInfected)
{
	int team;
	int otherTeam;
	qboolean civsInverted = !civsInfected;

	for (team = 0; team < MAX_ACTIVETEAM; team++) {
		if (!battle->teamActive[team])
			continue;

		for (otherTeam = 0; otherTeam < MAX_ACTIVETEAM; otherTeam++) {
			if (!battle->teamActive[otherTeam])
				continue;

			const autoMissionTeamType_t teamType = battle->teamType[team];
			const autoMissionTeamType_t otherTeamType = battle->teamType[otherTeam];
			if (AM_IsPlayer(teamType)) {
				if (AM_IsAlien(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qtrue);
				else if (AM_IsPlayer(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qfalse);
				else if (AM_IsCivilian(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qfalse);
			} else if (AM_IsAlien(teamType)) {
				if (AM_IsAlien(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qfalse);
				else if (AM_IsPlayer(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qtrue);
				else if (AM_IsCivilian(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, civsInverted);
			} else if (AM_IsCivilian(teamType)) {
				if (AM_IsAlien(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qfalse);
				else if (AM_IsPlayer(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, civsInfected);
				else if (AM_IsCivilian(otherTeamType))
					AM_SetHostile(battle, team, otherTeam, qfalse);
			}
		}
	}
}

static void CP_AutoBattleSetup (autoMissionBattle_t *battle)
{
	int unitTotal = 0;
	int isHostileTotal = 0;
	int totalActiveTeams = 0;
	int lastActiveTeam = -1;
	int isHostileCount;
	int team;

	/* Sums of various values */
	double teamPooledHealth[MAX_ACTIVETEAM];
	double teamPooledHealthMax[MAX_ACTIVETEAM];
	double teamPooledUnitsHealthy[MAX_ACTIVETEAM];
	double teamPooledUnitsTotal[MAX_ACTIVETEAM];
	/* Ratios */
	double teamRatioHealthyUnits[MAX_ACTIVETEAM];
	double teamRatioHealthTotal[MAX_ACTIVETEAM];
	int currentUnit;

	if (battle->hasBeenFought)
		Com_Error(ERR_DROP, "Error: Auto-Battle has already been fought!");

	for (team = 0; team < MAX_ACTIVETEAM; team++) {
		unitTotal += battle->nUnits[team];

		if (battle->teamActive[team]) {
			lastActiveTeam = team;
			totalActiveTeams++;
		}
		for (isHostileCount = 0; isHostileCount < MAX_ACTIVETEAM; isHostileCount++) {
			if (battle->nUnits[isHostileCount] <= 0)
				continue;

			if (battle->isHostile[team][isHostileCount] && battle->teamActive[team])
				isHostileTotal++;
		}
	}

	/* sanity checks */
	if (unitTotal == 0)
		Com_Error(ERR_DROP, "Grand total of ZERO units are fighting in auto battle, something is wrong.");

	if (unitTotal < 0)
		Com_Error(ERR_DROP, "Negative number of total units are fighting in auto battle, something is VERY wrong!");

	if (isHostileTotal <= 0)
		Com_Error(ERR_DROP, "No team has any other team hostile toward it, no battle is possible!");

	if (totalActiveTeams <= 0)
		Com_Error(ERR_DROP, "No Active teams detected in Auto Battle!");

	if (totalActiveTeams == 1)
		Com_DPrintf(DEBUG_CLIENT, "Note: Only one active team detected, this team will win the auto mission battle by default.");

	/* Quick easy victory check */
	if (totalActiveTeams == 1) {
		battle->winningTeam = lastActiveTeam;
		battle->hasBeenFought = qtrue;
		return;
	}

	/* Set up teams */
	for (team = 0; team < MAX_ACTIVETEAM; team++) {
		teamPooledHealth[team] = 0.0;
		teamPooledHealthMax[team] = 0.0;
		teamPooledUnitsHealthy[team] = 0.0;
		teamPooledUnitsTotal[team] = 0.0;

		if (battle->teamActive[team]) {
			/* Used for (t)emporary math (calc)ulation stuff */
			double skillAdjCalc;
			double skillAdjCalcAbs;

			for (currentUnit = 0; currentUnit < battle->nUnits[team]; currentUnit++) {
				if (battle->unitHealth[team][currentUnit] <= 0)
					continue;

				teamPooledHealth[team] += battle->unitHealth[team][currentUnit];
				teamPooledHealthMax[team] += battle->unitHealthMax[team][currentUnit];
				teamPooledUnitsTotal[team] += 1.0;
				if (battle->unitHealth[team][currentUnit] == battle->unitHealthMax[team][currentUnit])
					teamPooledUnitsHealthy[team] += 1.0;
			}
			/* We shouldn't be dividing by zero here. */
			assert(teamPooledHealthMax[team] > 0.0);
			assert(teamPooledUnitsTotal[team] > 0.0);

			teamRatioHealthTotal[team] = teamPooledHealth[team] / teamPooledHealthMax[team];
			teamRatioHealthyUnits[team] = teamPooledUnitsHealthy[team] / teamPooledUnitsTotal[team];

			/* In DEBUG mode, these should help with telling where things are at what time, for bug-hunting purposes. */
			/* Note (Destructavator):  Is there a better way to implement this?  Is there a set protocol for this type of thing? */
			Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Team %i has calculated ratio of healthy units of %lf",
					team, teamRatioHealthyUnits[team]);
			Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Team %i has calculated ratio of health values of %lf",
					team, teamRatioHealthTotal[team]);

			/** @todo speaking names please */
			skillAdjCalc = (teamRatioHealthyUnits[team] + teamRatioHealthTotal[team]);
			skillAdjCalc *= 0.50;
			skillAdjCalc = FpCurve1D_u_out(skillAdjCalc, 0.750, 0.50);
			skillAdjCalc -= 0.50;
			skillAdjCalcAbs = fabs(skillAdjCalc);
			if (skillAdjCalc > 0.0)
				battle->scoreTeamSkill[team] = FpCurveUp (battle->scoreTeamSkill[team], skillAdjCalcAbs);
			if (skillAdjCalc < 0.0)
				battle->scoreTeamSkill[team] = FpCurveDn (battle->scoreTeamSkill[team], skillAdjCalcAbs);
			/* if (skillAdjCalc == exact 0.0), no change to team's skill. */

			Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Team %i has adjusted skill rating of %lf",
					team, battle->scoreTeamSkill[team]);
		}
	}
}

static void CP_AutoBattleCheckFriendlyFire (autoMissionBattle_t *battle, int eTeam, const int currTeam, const int currUnit, const double effective)
{
	int eUnit;

	/* Check for "friendly" fire */
	for (eUnit = 0; eUnit < MAX_SOLDIERS_AUTOMISSION; eUnit++) {
		if (battle->unitHealth[eTeam][eUnit] > 0) {
			const double calcRand = frand();
			if (calcRand < (0.250 - (effective * 0.250))) {
				const int strikeDamage = (int) (20.0 * (1.0 - battle->scoreTeamDifficulty[currTeam]) * calcRand);

				battle->unitHealth[eTeam][eUnit] = max(0, battle->unitHealth[eTeam][eUnit] - strikeDamage);

				Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Unit %i on team %i hits unit %i on team %i for %i damage via friendly fire!",
						currUnit, currTeam, eUnit, eTeam, strikeDamage);

				if (battle->unitHealth[eTeam][eUnit] == 0)
					Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Friendly Unit %i on team %i is killed in action!",
							eUnit, eTeam);

				battle->teamAccomplishment[currTeam] -= strikeDamage;
			}
		}
	}
}

static void CP_AutoBattleCheckFire (autoMissionBattle_t *battle, int eTeam, const int currTeam, const int currUnit, const double effective)
{
	int eUnit;

	for (eUnit = 0; eUnit < MAX_SOLDIERS_AUTOMISSION; eUnit++) {
		if (battle->unitHealth[eTeam][eUnit] > 0) {
			const double calcRand = frand();
			if (calcRand >= effective) {
				const int strikeDamage = (int) (20.0 * battle->scoreTeamDifficulty[currTeam] * (effective - calcRand) / effective);

				battle->unitHealth[eTeam][eUnit] = max(0, battle->unitHealth[eTeam][eUnit] - strikeDamage);

				Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Unit %i on team %i strikes unit %i on team %i for %i damage!",
						currUnit, currTeam, eUnit, eTeam, strikeDamage);

				if (battle->unitHealth[eTeam][eUnit] == 0)
					Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Unit %i on team %i is killed in action!", eUnit, eTeam);

				battle->teamAccomplishment[currTeam] += strikeDamage;
			}
		}
	}
}

static qboolean CP_AutoBattleUnitAttackEnemies (autoMissionBattle_t *battle, const int currTeam, const int currUnit, const double effective)
{
	int eTeam;
	int count = 0;

	Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Unit %i on team %i attacks!", currUnit, currTeam);

	for (eTeam = 0; eTeam < MAX_ACTIVETEAM; eTeam++) {
		if (!battle->isHostile[currTeam][eTeam])
			continue;

		if (battle->teamActive[eTeam]) {
			count++;
			CP_AutoBattleCheckFire(battle, eTeam, currTeam, currUnit, effective);
		} else {
			CP_AutoBattleCheckFriendlyFire(battle, eTeam, currTeam, currUnit, effective);
		}
	}

	/* If there's no one left to fight, the battle's OVER. */
	if (count == 0)
		return qfalse;

	return qtrue;
}

static void CP_AutoBattleDoFight (autoMissionBattle_t *battle)
{
	/* Setup is done.  Now, FIGHT! */
	qboolean combatActive = qtrue;

	while (combatActive) {
		int team;
		for (team = 0; team < MAX_ACTIVETEAM; team++) {
			int aliveUnits;
			int currentUnit;

			if (!battle->teamActive[team])
				continue;

			aliveUnits = 0;
			/* Is this unit still alive (has any health left?) */
			for (currentUnit = 0; currentUnit < MAX_SOLDIERS_AUTOMISSION; currentUnit++) {
				/* Wounded units don't fight quite as well */
				const double hpLeftRatio = battle->unitHealth[team][currentUnit] / battle->unitHealthMax[team][currentUnit];
				const double effective = FpCurveDn(battle->scoreTeamSkill[team], hpLeftRatio * 0.50);
				if (battle->unitHealth[team][currentUnit] <= 0)
					continue;

				Com_DPrintf(DEBUG_CLIENT, "(Debug/value track) Unit %i on team %i has adjusted attack rating of %lf",
						currentUnit, team, battle->scoreTeamSkill[team]);

				aliveUnits++;
				combatActive = CP_AutoBattleUnitAttackEnemies(battle, team, currentUnit, effective);
			}

			if (aliveUnits == 0)
				battle->teamActive[team] = qfalse;
		}
	}
}

void CP_AutoBattleRunBattle (autoMissionBattle_t *battle)
{
	CP_AutoBattleSetup(battle);
	CP_AutoBattleDoFight(battle);
	CP_AutoBattleDecideResults(battle);
	CP_AutoBattleDisplayResults(battle);
}

/**
 * @brief This will display on-screen, for the player, results of the auto mission.
 */
void CP_AutoBattleDisplayResults (const autoMissionBattle_t *battle)
{
	/** @todo */
}

void CP_AutoBattleFillTeamFromBattleParams (autoMissionBattle_t *battle, const struct battleParam_s *missionParams)
{
	assert(battle);
	assert(missionParams);

	int numAliens = missionParams->aliens;
	int numAlienDrones = int (frand() * numAliens);
	int numCivs = missionParams->civilians;

	/* Alines will go on team 2, alien drones on 3, civs on 4 (player soldiers are 0 and UGVs are 1). */
	battle->nUnits[2] = numAliens;
	battle->nUnits[3] = numAlienDrones;
	battle->nUnits[4] = numCivs;

	int unit;
	int health;
	int healthMax;

	/* Populate the teams */

	/* Aliens */
	for (unit = 0; unit < numAliens; unit++) {
		/* Quick, ugly way of deciding alien health scores.  Eventually we'll need something better. */
		healthMax = int (frand() * 40.f) + 20.f;
		health = int (frand() * (healthMax - 10)) + 10;
		battle->unitHealthMax[2] = healthMax;
		battle->unitHealth[2] = health;
	}
	battle->teamActive[2] = qtrue;
	battle->teamType[2] = AUTOMISSION_TEAM_TYPE_ALIEN;
	battle->scoreTeamSkill[2] = (frand() * 0.6f) + 0.2f;

	if (numAlienDrones > 0) {
		for (unit = 0; unit < numAlienDrones; unit++) {
			/* Quick, ugly way of deciding alien drone health scores.  Eventually we'll need something better. */
			healthMax = int (frand() * 60.f) + 30.f;
			health = int (frand() * (healthMax - 10)) + 10;
			battle->unitHealthMax[3] = healthMax;
			battle->unitHealth[3] = health;
		}
		battle->teamActive[3] = qtrue;
		battle->teamType[3] = AUTOMISSION_TEAM_TYPE_ALIEN_DRONE;
		battle->scoreTeamSkill[3] = (frand() * 0.6f) + 0.3f;
	}

	/* Civilians (if any) */
	if (numCivs > 0) {
		for (unit = 0; unit < numCivs; unit++) {
			/* Quick, ugly way of deciding alien drone health scores.  Eventually we'll need something better. */
			healthMax = int (frand() * 20.f) + 5.f;
			health = int (frand() * (healthMax - 4)) + 4;
			battle->unitHealthMax[4] = healthMax;
			battle->unitHealth[4] = health;
		}
		battle->teamActive[4] = qtrue;
		battle->teamType[4] = AUTOMISSION_TEAM_TYPE_CIVILIAN;
		battle->scoreTeamSkill[4] = (frand() * 0.5f) + 0.05f;
	}
}
