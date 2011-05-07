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

/**
 * @brief Clears, initializes, or resets a single auto mission, sets default values.
 * @param[in,out] battle The battle that should be init to defaults
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
		battle->scoreTeamDifficulty[team] = 0.5f;
		battle->scoreTeamEquipment[team] = 0.5f;
		battle->scoreTeamSkill[team] = 0.5f;

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
 * @param[in,out] battle  The auto mission battle to add team data to
 * @param[in] teamNum The team number in the auto mission instance to update
 * @param[in] aircraft The aircraft to get data from
 * @param[in] campaign The current campaign (for retrieving difficulty level)
 * @note This function actually gets the data from the campaign ccs object, using the aircraft data to
 * find out which of all the employees are on the aircraft (in the mission)
 */
void CP_AutoBattleFillTeamFromAircraft (autoMissionBattle_t *battle, const int teamNum, const aircraft_t *aircraft, campaign_t *campaign)
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

	battle->scoreTeamDifficulty[teamNum] = 0.5;
	if (campaign->difficulty == -1) battle->scoreTeamDifficulty[teamNum] = 0.55;
	if (campaign->difficulty == -2) battle->scoreTeamDifficulty[teamNum] = 0.6;
	if (campaign->difficulty == -3) battle->scoreTeamDifficulty[teamNum] = 0.65;
	if (campaign->difficulty == -4) battle->scoreTeamDifficulty[teamNum] = 0.75;
	if (campaign->difficulty == 1) battle->scoreTeamDifficulty[teamNum] = 0.45;
	if (campaign->difficulty == 2) battle->scoreTeamDifficulty[teamNum] = 0.4;
	if (campaign->difficulty == 3) battle->scoreTeamDifficulty[teamNum] = 0.35;
	if (campaign->difficulty == 4) battle->scoreTeamDifficulty[teamNum] = 0.25;

	/* Zero is default ID for human player team ID, at least for auto missions. */
	battle->teamID[teamNum] = 0;
}

/* These are "placeholders" for the functions not yet written out, to (hopefully) allow compiling the project without error (for now) */
void CP_AutoBattleFillTeamFromAircraftUGVs (autoMissionBattle_t *battle, const int teamNum, const struct aircraft_s *aircraft, campaign_t *campaign)
{
	/** @todo */
}

void CP_AutoBattleFillTeamFromPlayerBase (autoMissionBattle_t *battle, const int teamNum, const int baseNum, campaign_t *campaign)
{
	/** @todo */
}

void CP_AutoBattleFillTeamFromAlienGroup (autoMissionBattle_t *battle, const int teamNum, const struct alienTeamGroup_s *alienGroup)
{
	/** @todo */
}

void CP_AutoBattleCreateTeamFromScratch (autoMissionBattle_t *battle, const int teamNum)
{
	/** @todo */
}

int CP_AutoBattleGetWinningTeam (const autoMissionBattle_t *battle)
{
	/** @todo */
	return 0;
}

/** @brief Run this on an auto mission battle before the battle is actually simulated with CP_AutoBattleRunBattle(), to set
 * default values for who will attack which team.  If you forget to do this before battle simulation, all teams will default
 * to "free for all" (Everyone will try to kill everyone else).
 * @param[in, out] battle The battle to set up team hostility values for.
 * @param[in] civsInfected Set to "qtrue" if civs have XVI influence, otherwise "qfalse" for a normal mission. */
void CP_AutoBattleSetDefaultHostilities(autoMissionBattle_t *battle, qboolean civsInfected)
{
	int team;
	int otherTeam;

	qboolean civsInverted = qtrue;
	if (civsInfected == qtrue) civsInverted = qfalse;

	/* Build an array of default values for what types of teams will attack whom. */
	qboolean hostileList[AUTOMISSION_TEAM_TYPE_MAX][AUTOMISSION_TEAM_TYPE_MAX];

	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER][AUTOMISSION_TEAM_TYPE_PLAYER] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER][AUTOMISSION_TEAM_TYPE_PLAYER_UGV] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER][AUTOMISSION_TEAM_TYPE_ALIEN] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER][AUTOMISSION_TEAM_TYPE_ALIEN_DRONE] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER][AUTOMISSION_TEAM_TYPE_CIVILIAN] = qfalse;

	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER_UGV][AUTOMISSION_TEAM_TYPE_PLAYER] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER_UGV][AUTOMISSION_TEAM_TYPE_PLAYER_UGV] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER_UGV][AUTOMISSION_TEAM_TYPE_ALIEN] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER_UGV][AUTOMISSION_TEAM_TYPE_ALIEN_DRONE] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_PLAYER_UGV][AUTOMISSION_TEAM_TYPE_CIVILIAN] = qfalse;

	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN][AUTOMISSION_TEAM_TYPE_PLAYER] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN][AUTOMISSION_TEAM_TYPE_PLAYER_UGV] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN][AUTOMISSION_TEAM_TYPE_ALIEN] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN][AUTOMISSION_TEAM_TYPE_ALIEN_DRONE] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN][AUTOMISSION_TEAM_TYPE_CIVILIAN] = civsInverted;

	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN_DRONE][AUTOMISSION_TEAM_TYPE_PLAYER] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN_DRONE][AUTOMISSION_TEAM_TYPE_PLAYER_UGV] = qtrue;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN_DRONE][AUTOMISSION_TEAM_TYPE_ALIEN] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN_DRONE][AUTOMISSION_TEAM_TYPE_ALIEN_DRONE] = qfalse;
	hostileList[AUTOMISSION_TEAM_TYPE_ALIEN_DRONE][AUTOMISSION_TEAM_TYPE_CIVILIAN] = civsInverted;

	hostileList[AUTOMISSION_TEAM_TYPE_CIVILIAN][AUTOMISSION_TEAM_TYPE_PLAYER] = civsInfected;
	hostileList[AUTOMISSION_TEAM_TYPE_CIVILIAN][AUTOMISSION_TEAM_TYPE_PLAYER_UGV] = civsInfected;
	hostileList[AUTOMISSION_TEAM_TYPE_CIVILIAN][AUTOMISSION_TEAM_TYPE_ALIEN] = civsInverted;
	hostileList[AUTOMISSION_TEAM_TYPE_CIVILIAN][AUTOMISSION_TEAM_TYPE_ALIEN_DRONE] = civsInverted;
	hostileList[AUTOMISSION_TEAM_TYPE_CIVILIAN][AUTOMISSION_TEAM_TYPE_CIVILIAN] = qfalse;

	for (team = 0; team < MAX_ACTIVETEAM; team++) {
		if (battle->teamActive[team] == qtrue) {
			for (otherTeam = 0; otherTeam < MAX_ACTIVETEAM; otherTeam++) {
				if (battle->teamActive[otherTeam] == qtrue)
					battle->isHostile[team][otherTeam] = hostileList[battle->teamType[team]][battle->teamType[otherTeam]];
			}
		}
	}
}

void CP_AutoBattleRunBattle (autoMissionBattle_t *battle)
{
	int unitTotal = 0;
	int isHostileTotal = 0;
	int totalActiveTeams = 0;
	int lastActiveTeam = -1;
	int isHostileCount;
	int count;

	if (battle->hasBeenFought)
		Com_Error(ERR_DROP, "Error: Auto-Battle has already been fought!");

	for (count = 0; count < MAX_ACTIVETEAM; count++) {
		unitTotal += battle->nUnits[count];

		if (battle->teamActive[count]) {
			lastActiveTeam = count;
			totalActiveTeams++;
		}
		for (isHostileCount = 0; isHostileCount < MAX_ACTIVETEAM; isHostileCount++) {
			if (battle->nUnits[isHostileCount] > 0) {
				if (battle->isHostile[count][isHostileCount] && battle->teamActive[count])
					isHostileTotal++;
			}
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
	double teamPooledHealth[MAX_ACTIVETEAM];
	double teamPooledHealthMax[MAX_ACTIVETEAM];
	double teamPooledUnitsHealthy[MAX_ACTIVETEAM];
	double teamPooledUnitsTotal[MAX_ACTIVETEAM];
	int currentUnit;

	for (count = 0; count < MAX_ACTIVETEAM; count++) {
		teamPooledHealth[count] = 0.0;
		teamPooledHealthMax[count] = 0.0;
		teamPooledUnitsHealthy[count] = 0.0;
		teamPooledUnitsTotal[count] = 0.0;
		if (battle->teamActive[count]) {
			for (currentUnit = 0; currentUnit < battle->nUnits[count]; currentUnit++) {
				if (battle->unitHealth[count][currentUnit] > 0) {
					teamPooledHealth[count] += battle->unitHealth[count][currentUnit];
					teamPooledHealthMax[count] += battle->unitHealthMax[count][currentUnit];
					teamPooledUnitsTotal[count] += 1.0;
					if (battle->unitHealth[count][currentUnit] == battle->unitHealthMax[count][currentUnit])
						teamPooledUnitsHealthy[count] += 1.0;
				}
			}
		}
	}
}
