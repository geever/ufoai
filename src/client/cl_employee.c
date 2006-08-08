/**
 * @file cl_employee.c
 * @brief Single player employee stuff
 */

/*
Copyright (C) 2002-2006 UFO: Alien Invasion team.

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

#include "client.h"
#include "cl_global.h"

/* holds the current active employee category */
static int employeeCategory = 0;
/* how many employees in current list (changes on every catergory change, too) */
static int employeesInCurrentList = 0;

/*****************************************************
 * VISUAL/GUI STUFF
  *****************************************************/

/**
 * @brief Will fill the list with employees
 * @note is the init function in employee menu
 */
static void E_EmployeeList (void)
{
	int i, j;
	employee_t* employee;

	/* can be called from everywhere without a started game */
	if (!baseCurrent ||!curCampaign)
		return;

	if (Cmd_Argc() < 2) {
		Com_Printf("Usage: employee_init <category>\n");
		return;
	}
	employeeCategory = atoi(Cmd_Argv(1));
	if (employeeCategory > MAX_EMPL || employeeCategory < 0)
		employeeCategory = EMPL_SOLDIER;

	employeesInCurrentList = 0;

	for (j=0, employee=gd.employees[employeeCategory]; j<gd.numEmployees[employeeCategory]; j++, employee++) {
		Cvar_ForceSet(va("mn_name%i", employeesInCurrentList), employee->chr.name);
		/* TODO; Check whether he is assigned to aircraft and/or carries weapons */
		/* change the buttons */
		if (employee->hired)
			Cbuf_AddText(va("employeeadd%i\n", employeesInCurrentList));

		employeesInCurrentList++;
		/* we can't display more than 19 employees */
		if (employeesInCurrentList>=(int)cl_numnames->value)
			break;
	}
	i = employeesInCurrentList;
	for (;i<(int)cl_numnames->value;i++) {
		Cvar_ForceSet(va("mn_name%i", i), "");
		Cbuf_AddText(va("employeedisable%i\n", i));
	}
}


/*****************************************************
 * EMPLOYEE BACKEND STUFF
 *****************************************************/

/**
 * @brief Convert string to employeeType_t
 * @param type Pointer to employee type string
 * @return employeeType_t
 */
employeeType_t E_GetEmployeeType(char* type)
{
	assert(type);
	if ( Q_strncmp(type, "EMPL_SCIENTIST", 14 ) )
		return EMPL_SCIENTIST;
	else if ( Q_strncmp(type, "EMPL_SOLDIER", 12 ) )
		return EMPL_SOLDIER;
	else if ( Q_strncmp(type, "EMPL_WORKER", 11 ) )
		return EMPL_WORKER;
	else if ( Q_strncmp(type, "EMPL_MEDIC", 10 ) )
		return EMPL_MEDIC;
	else if ( Q_strncmp(type, "EMPL_ROBOT", 10 ) )
		return EMPL_ROBOT;
	else {
		Sys_Error("Unknown employee type '%s'\n", type);
		return MAX_EMPL;
	}
}

/**
 * @brief Reads in information about employees and assigns them to the correct rooms in a base.
 *
 * This should be called after setting up the first base.
 * @todo This right now assumes that there are not more employees than free quarter space ... but it will not puke if there are.
 * @sa CL_GameInit
 */
void E_InitEmployees(void)
{
}

/**
 * @brief Returns true if the employee is only assigned to quarters but not to labs and workshops.
 *
 * @param[in] employee The employee_t pointer to check
 * @return qboolean
 * @sa E_EmployeeIsUnassinged
 */
qboolean E_EmployeeIsFree(employee_t * employee)
{
	return (employee->buildingID < 0 && employee->hired);
}

/**
 * @brief Returns true if the employee is _only_ listed in the global list.
 *
 * @param[in] employee The employee_t pointer to check
 * @return qboolean
 * @sa E_EmployeeIsFree
 */
qboolean E_EmployeeIsUnassinged(employee_t * employee)
{
	return (employee->buildingID < 0);
}

/**
 * @brief Creates an entry of a new employee in the global list and assignes it to no building/base.
 *
 * @param[in] type Tell the function what type of employee to create.
 * @return Pointer to the newly created employee in the global list. NULL if something goes wrong.
 * @sa E_DeleteEmployee
 */
employee_t* E_CreateEmployee(employeeType_t type)
{
	employee_t* employee = NULL;

	if (type == MAX_EMPL) return NULL;

	if (gd.numEmployees[type] >= MAX_EMPLOYEES) {
		Com_Printf("E_CreateEmployee: MAX_EMPLOYEES exceeded\n");
		return NULL;
	}

	employee = &gd.employees[type][gd.numEmployees[type]++];

	if (!employee)
		return NULL;

	employee->hired = qfalse;
	employee->buildingID = -1;

	switch (type) {
		case EMPL_SOLDIER:
			/* TODO: create random data for the employees depending on type and skill-min/max */
			/* employee->combat_stats = CL_GenerateCharacter(Cvar_VariableString("team"), NULL, ET_ACTOR); */
			CL_GenerateCharacter(employee, Cvar_VariableString("team"), ET_ACTOR);
			break;
		case EMPL_SCIENTIST:
		case EMPL_WORKER:
			/* TODO: create random data for the employees depending on type and skill-min/max */
			/* employee->combat_stats = CL_GenerateCharacter(Cvar_VariableString("team"), NULL, ET_ACTOR); */
			CL_GenerateCharacter(employee, Cvar_VariableString("team"), ET_ACTOR);
			employee->speed = 100;
			break;
		/*case EMPL_MEDIC: break; */
		/*case EMPL_ROBOT: break; */
		default:
			break;
		}
	return employee;
}

/**
 * @brief Removes the employee compeltely from the game (buildings + global list).
 *
 * @param[in] employee The pointer to the employee you want to remove.
 * @return True if the employee was removed sucessfully, otherwise false.
 * @sa E_CreateEmployee
 */
qboolean E_DeleteEmployee(employee_t *employee, employeeType_t type)
{
	int i;
	qboolean found;

	if (!employee)
		return qfalse;

	if (employee->buildingID) {
/*		if (E_DeleteEmployeeFromBuilding(employee, employee->quarters))
			employee->quarters = -1;*/
	}

	employee->baseIDHired = -1;

	/* remove the employee from the global list */
	for (i = 0; i < gd.numEmployees[type] - 1; i++) {
		if (gd.employees[type][i].idx == employee->idx) {
			/* TODO: delete me */
			/* THIS IS NEEDED, too */
			/*
			(baseCurrent->teamInv[i]).c[csi.idFloor] = NULL;
			Com_DestroyInventory(&baseCurrent->teamInv[i]);
			memset(&baseCurrent->teamInv[j], 0, sizeof(inventory_t));
			*/
			found = qtrue;
		}
		if (found)
			gd.employees[type][i] = gd.employees[type][i + 1];
	}

	if (found) {
		gd.numEmployees[type]--;
	} else {
		Com_DPrintf("E_DeleteEmployee: Employee wasn't in the global list.\n");
		return qfalse;
	}

	return qtrue;
}

/**
 * @brief Assigns an employee to a building.
 *
 * There are two cases of assignment:
 *	1.) From global list to quarters (building_dest is a quarter). This will search for compeltely unassigned employee of the given type in the global list gd.employees and assign them to the quarter if it has free space. The employee will only be linked to a quarter.
 *	2.) From quarters to 'any' other building (i.e. lab + workshop for now). This will search for a free (i.e not yet assigned to a building other than quarters) employee of the given type in a quarter in the same base building_dest is located in and. The employee will be linked to its quarter and the assinged building.
 *
 * @todo Add check for destination building vs. employee_type and abort if they do not match.
 *
 * @param[in] building_dest Which building to assign the employee to.
 * @param[in] employee_type	What type of employee to assign to the building.
 * @sa E_RemoveEmployee
 * @return Returns true if adding was possible/sane otherwise false. In the later case nothing will be changed.
 */
qboolean E_AssignEmployee(base_t *base, employeeType_t type)
{
	switch (type) {
	case EMPL_SOLDIER:
		break;
	case EMPL_SCIENTIST:
		/* see also RS_AssignScientist2 */
		if (base->hasLab) {
			return qtrue;
		}
		return qfalse;
	default:
		Com_DPrintf("Unhandled employee type: %i\n", type);
		break;
	}
	return qfalse;
}

/**
 * @brief Remove one employee from building.
 *
 * @todo Add check for destination building vs. employee_type and abort if they do not match.
 * @sa E_AssignEmployee
 * @param[in] building Which building to remove the employee from. Can be any type of building that has employees in it. If quarters are given the employee will be removed from every other building as well.
 *
 * @return Returns true if adding was possible/sane otherwise false.
 */
qboolean E_RemoveEmployee(base_t* base, employeeType_t type, int num)
{
	int i;
	for (i=0; i<gd.numEmployees[type];i++)
		if (i==num) {

			gd.numEmployees[type]--;
			break;
		}
	for (;i<gd.numEmployees[type];i++) {

	}
	return qfalse;
}


/**
 * @brief Gets an unassigned employee of a given type from the global list.
 *
 * @todo E_GetFreeEmployee
 * @param[in] type The type of employee to search.
 * @return employee_t
 * @sa E_EmployeeIsUnassinged
 * @sa E_EmployeesInBase2
 */
employee_t * E_GetUnassingedEmployee(employeeType_t type)
{
	int i;
	employee_t *employee = NULL;

	for (i = 0; i < gd.numEmployees[type]; i++) {
		employee = &gd.employees[type][i];
		if (E_EmployeeIsUnassinged(employee))
			return employee;
	}
	return NULL;
}

/**
 * @brief Gets the number of unassigned employees of a given type from the global list.
 *
 * @param[in] type The type of employee to search.
 * @return int number of found employees.
 * @sa E_EmployeeIsUnassinged
 * @sa E_GetUnassingedEmployee
*/
int E_GetUnassingedEmployeesByType(employeeType_t type)
{
	int i;
	int amount;
	employee_t *employee = NULL;

	for (i = 0; i < gd.numEmployees[type]; i++) {
		employee = &gd.employees[type][i];
		if (E_EmployeeIsUnassinged(employee))
			amount++;
	}
	return amount;
}

/**
 * @brief Function that does the real employee adding
 * @note This will only add to quarters right now.
 * @param[in] b Pointer to building
 * @param[in] amount Amount of employees to assign
 * @sa E_BuildingAddEmployees
 * @sa E_BuildingAddEmployees_f
 * @todo This function should look for unassinged employees for the give type in the global list (abort if not enough are found) and assign them to the building.
 */
qboolean E_BuildingAddEmployees(building_t *building, employeeType_t type, int amount)
{
	employees_t *employees_in_building = NULL;
	employee_t *employee = NULL;
	int unassinged_empl;
	int i;

	if (!building) return qfalse;

	if ( building->type != B_QUARTERS) {
		Com_Printf("Buulding is not a quarter.\n");
		return qfalse;
	}

	if ( type >= MAX_EMPL )
		return qfalse;

	unassinged_empl = E_GetUnassingedEmployeesByType(type);

	if ( unassinged_empl < amount) {
		Com_Printf("Not enough employees free of the given type (only %i instead of %i).\n", unassinged_empl, amount);
		return qfalse;
	}

	employees_in_building = &building->assigned_employees;
	if (employees_in_building->maxEmployees <= 0) {
		Com_Printf("No employees for this building: '%s'\n", building->id);
		return qfalse;
	}

	if ((employees_in_building->numEmployees + amount) >  employees_in_building->maxEmployees) {
		Com_Printf("The given amount of employees will not fit into this building: '%s'\n", building->id);
		return qfalse;
	}

	for (i = 0; i < amount; i++) {
		/* Get unassigned empl of given type. */
		employee = E_GetUnassingedEmployee(type)
		/* Assign empl to building. */
		if (!employee || !E_AssignEmployee(building,type)) {
			Com_Printf("Assignment of one employee went wrong: '%s'\n", building->id);
		}
	}
	return qtrue;
}

/**
 * @brief Script function for adding new employees to a building.
 * @note This will only add to quarters right now.
 * @sa E_BuildingAddEmployees
 */
void E_BuildingAddEmployees_f ( void )
{
	int i;
	int amount;
	employeeType_t type;

	Com_DPrintf("E_BuildingAddEmployees_f started\n");
	/* can be called from everywhere - so make a sanity check here */
	if (!baseCurrent || !baseCurrent->buildingCurrent)
		return;

	if (Cmd_Argc() < 3) {
		Com_Printf("Usage: building_add_employees <type> <amount>\n");
		return;
	}

	type = E_GetEmployeeType(Cmd_Argv(1));
	Com_DPrintf("E_BuildingAddEmployees_f %i/%i\n", type, MAX_EMPL);
	if (type == MAX_EMPL)
		return;

	amount = atoi(Cmd_Argv(2));

	if ( (gd.numEmployees + amount) < gd.maxEmployees ) {
		for (i = 0; i < amount; i++) {
			E_CreateEmployee(type);
		}

		if (!E_BuildingAddEmployees(baseCurrent->buildingCurrent, type, amount))
			Com_DPrintf("No Employees added.\n");
		Com_DPrintf("E_BuildingAddEmployees_f finished.\n");
	} else {
		Com_DPrintf("E_BuildingAddEmployees_f: maxEmployees reached (num=$i, max=$i, new=$i).\n",gd.numEmployees, gd.maxEmployees, ammount);
	}
}

/**
 * @brief
 */
void E_EmployeeHire_f (void)
{
	int num;

	/* check syntax */
	if (Cmd_Argc() < 2) {
		Com_Printf("Usage: hire <num>\n");
		return;
	}
	num = atoi(Cmd_Argv(1));

	/* some sanity checks */
	if (employeeCategory >= MAX_EMPL && employeeCategory < EMPL_SOLDIER )
		return;

	if (num >= employeesInCurrentList || num < 0)
		return;

	if (gd.employees[employeeCategory][num].hired)
		gd.employees[employeeCategory][num].hired = qfalse;
	else
		gd.employees[employeeCategory][num].hired = qfalse;
	/* update list */
	E_EmployeeList();
}

/**
  * @brief
  */
static void E_EmployeeSelect_f(void)
{
	int num;

	/* check syntax */
	if (Cmd_Argc() < 2) {
		Com_Printf("Usage: employee_select <num>\n");
		return;
	}
	num = atoi(Cmd_Argv(1));

	if (num >= gd.numEmployees[employeeCategory])
		return;

	/* console commands */
	Cbuf_AddText(va("employeedeselect%i\n", (int) cl_selected->value));
	Cbuf_AddText(va("employeeselect%i\n", num));
	Cvar_ForceSet("cl_selected", va("%i", num));

	/* set info cvars */
	CL_CharacterCvars(&(gd.employees[employeeCategory][num].chr));
}

/**
  * @brief This is more or less the initial
  * Bind some of the functions in this file to console-commands that you can call ingame.
  * Called from MN_ResetMenus resp. CL_InitLocal
  */
void E_ResetEmployee(void)
{
	/* add commands */
	Cmd_AddCommand("employee_init", E_EmployeeList);
	Cmd_AddCommand("employee_hire", E_EmployeeHire_f);
	Cmd_AddCommand("employee_select", E_EmployeeSelect_f);
}
