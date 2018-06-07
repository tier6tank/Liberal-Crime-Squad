#include "../includes.h"
/*
Copyright (c) 2002,2003,2004 by Tarn Adams                                            //
																					  //
This file is part of Liberal Crime Squad.                                             //
																					//
	Liberal Crime Squad is free software; you can redistribute it and/or modify     //
	it under the terms of the GNU General Public License as published by            //
	the Free Software Foundation; either version 2 of the License, or               //
	(at your option) any later version.                                             //
																					//
	Liberal Crime Squad is distributed in the hope that it will be useful,          //
	but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the                  //
	GNU General Public License for more details.                                    //
																					//
	You should have received a copy of the GNU General Public License               //
	along with Liberal Crime Squad; if not, write to the Free Software              //
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   02111-1307   USA     //
*/
/*
		This file was created by Chris Johnson (grundee@users.sourceforge.net)
		by copying code from game.cpp.
		To see descriptions of files and functions, see the list at
		the bottom of includes.h in the top src folder.
*/

const string blankString = "";
const string tag_value = "value";
const string tag_attribute = "attribute";
const string tag_skill = "skill";
#include "../creature/creature.h"
#include "../locations/locationsEnums.h"
//#include "../pdcurses/curses.h"
#include "../common/ledgerEnums.h"
#include "../common/ledger.h"
#include "../vehicle/vehicletype.h"
#include "../vehicle/vehicle.h"
#include "../common/translateid.h"
// for  int getsquad(int)
#include "../log/log.h"
// for commondisplay.h
#include "../common/commondisplay.h"
// for makedelimeter
#include "../title/highscore.h"       
//for void savehighscore(char endtype)
#include "../politics/politics.h"
//for int publicmood(int l)
#include "../cursesAlternative.h"
#include "../cursesAlternativeConstants.h"
#include "../customMaps.h"
#include "../set_color_support.h"
#include "../locations/locationsPool.h"
#include "../common/creaturePool.h"
/* end the game and clean up */
void end_game(int err = EXIT_SUCCESS);
extern string singleSpace;
extern string spaceDashSpace;
extern string singleDot;

/* common - tests if the person is a wanted criminal */
// *JDS* Checks if the character is a criminal
bool iscriminal(Creature &cr)
{
	for (int i = 0; i < LAWFLAGNUM; i++) if (cr.crimes_suspected[i]) return true;
	return false;
}
/* common - determines how long a creature's injuries will take to heal */
int clinictime(Creature &g)
{
	int time = 0;
	for (int w = 0; w < BODYPARTNUM; w++)
		if ((g.wound[w] & WOUND_NASTYOFF) && (g.blood < 100))
			time++;
	if (g.blood <= 10)time++;
	if (g.blood <= 50)time++;
	if (g.blood < 100)time++;
	if (!g.special[SPECIALWOUND_RIGHTLUNG])time++;
	if (!g.special[SPECIALWOUND_LEFTLUNG])time++;
	if (!g.special[SPECIALWOUND_HEART])time += 2;
	if (!g.special[SPECIALWOUND_LIVER])time++;
	if (!g.special[SPECIALWOUND_STOMACH])time++;
	if (!g.special[SPECIALWOUND_RIGHTKIDNEY])time++;
	if (!g.special[SPECIALWOUND_LEFTKIDNEY])time++;
	if (!g.special[SPECIALWOUND_SPLEEN])time++;
	if (g.special[SPECIALWOUND_RIBS] < RIBNUM)time++;
	if (!g.special[SPECIALWOUND_NECK])time++;
	if (!g.special[SPECIALWOUND_UPPERSPINE])time++;
	if (!g.special[SPECIALWOUND_LOWERSPINE])time++;
	return time;
}
/* common - sends somebody to the hospital */
/***************************************************
* *JDS* Hospitalize -- sends current person to the *
* specified clinic or hospital.                    *
***************************************************/
void hospitalize(int loc, Creature &patient)
{
	const string CONST_commonactions008 = "harmful speech";
	const string CONST_commonactions007 = "month";
	const string CONST_commonactions006 = "months";
	const string CONST_commonactions005 = " for ";
	const string CONST_commonactions004 = " will be at ";
	extern Log gamelog;
	extern vector<squadst *> squad;
	// He's dead, Jim
	if (!patient.alive)return;
	int time = clinictime(patient);
	if (time > 0)
	{
		squadst* patientsquad = NULL;
		if (patient.squadid != -1)
			patientsquad = squad[getsquad(patient.squadid)];
		patient.clinic = time;
		patient.squadid = -1;
		patient.location = loc;
		// Inform about the hospitalization
		makedelimiter();
		set_color_easy(WHITE_ON_BLACK_BRIGHT);
		mvaddstrAlt(8, 1, patient.name, gamelog);
		addstrAlt(CONST_commonactions004, gamelog);
		addstrAlt(LocationsPool::getInstance().getLocationName(loc), gamelog);
		addstrAlt(CONST_commonactions005, gamelog);
		addstrAlt(time, gamelog);
		addstrAlt(singleSpace, gamelog);
		if (time > 1)addstrAlt(CONST_commonactions006, gamelog);
		else addstrAlt(CONST_commonactions007, gamelog);
		addstrAlt(singleDot, gamelog);
		gamelog.nextMessage(); //Time for the next message.
		pressAnyKey();
		if (patientsquad)
		{  // Reorganize patient's former squad
			bool flipstart = 0;
			for (int p = 0; p < 6; p++)
			{
				if (patientsquad->squad[p] == &patient)
				{
					patientsquad->squad[p] = NULL;
					flipstart = 1;
				}
				if (flipstart&&p < 5) patientsquad->squad[p] = patientsquad->squad[p + 1];
			}
			if (flipstart) patientsquad->squad[5] = NULL;
		}
	}
}
/* returns the amount of heat associated with a given crime */
int lawflagheat(int lawflag)
{
	// Note that for the purposes of this function, we're not looking at how severe the crime is,
	// but how vigorously it is pursued by law enforcement. This determines how quickly they raid
	// you for it, and how much of a penalty you get in court for it. Some crimes are inflated
	// heat, others are deflated (such as the violent crimes).
	//
	// - Jonathan S. Fox
	switch (lawflag)
	{
	case LAWFLAG_TREASON:return 100;
	case LAWFLAG_TERRORISM:return 100;
	case LAWFLAG_MURDER:return 20;
	case LAWFLAG_KIDNAPPING:return 20;
	case LAWFLAG_BANKROBBERY:return 20;
	case LAWFLAG_ARSON:return 50;
	case LAWFLAG_BURNFLAG:return 0;
	case LAWFLAG_SPEECH:return 0;
	case LAWFLAG_BROWNIES:return 50;
	case LAWFLAG_ESCAPED:return 50;
	case LAWFLAG_HELPESCAPE:return 50;
	case LAWFLAG_JURY:return 0;
	case LAWFLAG_RACKETEERING:return 50;
	case LAWFLAG_EXTORTION:return 20;
	case LAWFLAG_ARMEDASSAULT:return 0;   // XXX: This is on the same level as CONST_commonactions008?
	case LAWFLAG_ASSAULT:return 0;        // Fox: Yes. You get too many assault charges to put heat on it.
	case LAWFLAG_CARTHEFT:return 0;
	case LAWFLAG_CCFRAUD:return 20;
	case LAWFLAG_THEFT:return 0;
	case LAWFLAG_PROSTITUTION:return 0;
	case LAWFLAG_HIREILLEGAL:return 10;
		//case LAWFLAG_GUNUSE:return 1;
		//case LAWFLAG_GUNCARRY:return 0;
	case LAWFLAG_COMMERCE:return 20;
	case LAWFLAG_INFORMATION:return 50;
	case LAWFLAG_BURIAL:return 0;
	case LAWFLAG_BREAKING:return 0;
	case LAWFLAG_VANDALISM:return 0;
	case LAWFLAG_RESIST:return 10;
	case LAWFLAG_DISTURBANCE:return 0;
	case LAWFLAG_PUBLICNUDITY:return 0;
	case LAWFLAG_LOITERING:return 0;
	default:return 0;
	}
}
/* common - applies a crime to a person */
void criminalize(Creature &cr, short crime)
{
	extern short mode;
	extern short cursite;
	if (mode == GAMEMODE_SITE)
	{
		if (LocationsPool::getInstance().isThereASiegeHere(cursite))
		{
			// Do not criminalize the LCS for self-defense against
			// extrajudicial raids
			if (LocationsPool::getInstance().getSiegeType(cursite) != SIEGE_POLICE)
				return;
		}
		else if (LocationsPool::getInstance().getRentingType(cursite) == RENTING_CCS)
			// Do not criminalize the LCS for crimes against the CCS
			return;
	}
	cr.crimes_suspected[crime]++;
	cr.heat += lawflagheat(crime);
}
/* common - applies a crime to everyone in the active party */
void criminalizeparty(short crime)
{
	extern squadst *activesquad;
	if (!activesquad) return;
	for (int p = 0; p < 6; p++)
		if (activesquad->squad[p])
		{
			if (!activesquad->squad[p]->alive) continue;
			criminalize(*(activesquad->squad[p]), crime);
		}
}

// *JDS* Scarefactor is the severity of the case against you; if you're a really
// nasty person with a wide variety of major charges against you, then scarefactor
// can get up there
int scare_factor(int lawflag, int crimenumber) {
	return lawflagheat(lawflag) * crimenumber;
}
void addjuice(Creature &cr, long juice, long cap);
/* common - gives juice to everyone in the active party */
void juiceparty(long juice, long cap)
{
	extern squadst *activesquad;
	if (activesquad != NULL)
		for (int p = 0; p < 6; p++)
			if (activesquad->squad[p] != NULL)
				if (activesquad->squad[p]->alive)
					addjuice(*activesquad->squad[p], juice, cap);
}
/* common - removes the liberal from all squads */
void removesquadinfo(Creature &cr)
{
	extern vector<squadst *> squad;
	if (cr.squadid != -1)
	{
		long sq = getsquad(cr.squadid);
		if (sq != -1)
		{
			bool flipstart = 0;
			for (int pt = 0; pt < 6; pt++)
			{
				if (squad[sq]->squad[pt] == &cr)flipstart = 1;
				if (flipstart&&pt < 5)squad[sq]->squad[pt] = squad[sq]->squad[pt + 1];
			}
			if (flipstart)squad[sq]->squad[5] = NULL;
		}
		cr.squadid = -1;
	}
}

// Picks a random option, based on the weights provided
int choose_one(const int * weight_list, int number_of_options, int default_value)
{
	int weight_total = 0;
	for (int option = 0; option < number_of_options; option++)
		weight_total += weight_list[option];
	if (weight_total < 1) return default_value; // No acceptable results; use default
	int choose = LCSrandom(weight_total);
	for (int option = 0; option < number_of_options; option++)
	{
		choose -= weight_list[option];
		if (choose < 0)return option;
	}
	return number_of_options;
}
/* common - assigns a new base to all members of a squad */
void basesquad(squadst *st, long loc)
{
	for (int p = 0; p < 6; p++) if (st->squad[p] != NULL) st->squad[p]->base = loc;
}
/* common - shifts public opinion on an issue */
void change_public_opinion(int v, int power, char affect, char cap)
{
	extern short attitude[VIEWNUM];
	extern short public_interest[VIEWNUM];
	extern short background_liberal_influence[VIEWNUM];
	// First note this in the liberal influence -- mostly for the
	// sake of the nice visual intelligence report entry
	if (v < VIEWNUM - 5)
	{
		background_liberal_influence[v] += power * 10;
	}
	if (v == VIEW_LIBERALCRIMESQUAD)affect = 0;
	if (v == VIEW_LIBERALCRIMESQUADPOS)affect = 0;
	if (v == VIEW_LIBERALCRIMESQUADPOS)
	{
		int mood = publicmood(-1);
		if (cap > mood + 40)cap = mood + 40;
	}
	int effpower = power;
	// Affect is whether the LCS is publicly known to be behind
	// the circumstances creating the public opinion change
	if (affect == 1)
	{
		// Aff is the % of people who know/care about the LCS
		int aff = attitude[VIEW_LIBERALCRIMESQUAD];
		// Rawpower is the amount of the action proportional
		// to the people who, not having heard of the LCS,
		// do not allow the LCS' reputation to affect their
		// opinions
		int rawpower = (int)((float)power * (float)(100 - aff) / 100.0f);
		// Affected power is the remainder of the action besides
		// rawpower, the amount of the people who know of the LCS
		// and have it alter their opinion
		int affectedpower = power - rawpower;
		if (affectedpower > 0)
		{
			// Dist is a combination of the relative popularity of the LCS
			// to the issue and the absolute popularity of the LCS. Very
			// popular LCS on a very unpopular issue is very influential.
			// Very unpopular LCS on a very popular issue has the ability
			// to actually have a reverse effect.
			int dist = attitude[VIEW_LIBERALCRIMESQUADPOS] - attitude[v] +
				attitude[VIEW_LIBERALCRIMESQUADPOS] - 50;
			// Affected power is then scaled by dist -- if the LCS is
			// equally popular as the issue, it's equally powerful as
			// the rawpower. For every 10% up or down past there, it's
			// 10% more or less powerful.
			affectedpower = (int)(((float)affectedpower*(100.0 + (float)dist)) / 100.0f);
		}
		// Effpower is then the sum of the rawpower (people who don't know
		// about the LCS) and the affectedpower (people who do know
		// about the LCS and had their judgment swayed by their opinion
		// of it).
		effpower = rawpower + affectedpower;
	}
	else if (affect == -1)
	{
		// Simplifed algorithm for affect by CCS respect
		effpower = power * (100 - attitude[VIEW_CONSERVATIVECRIMESQUAD]) / 100;
	}
	if (v == VIEW_LIBERALCRIMESQUAD)
	{
		//Only half the country will ever hear about the LCS at one time,
		//and people will only grudgingly lose fear of it
		if (effpower < -5)effpower = -5;
		if (effpower > 50)effpower = 50;
	}
	else if (v == VIEW_LIBERALCRIMESQUADPOS)
	{
		//Only 50% of the country can be swayed at once in their views
		//of the LCS negatively, 5% positively
		if (effpower < -50)effpower = -50;
		if (effpower > 5)effpower = 5;
	}
	//Scale the magnitude of the effect based on how much
	//people are paying attention to the issue
	effpower = (int)(effpower*(1 + (float)public_interest[v] / 50));
	//Then affect public interest
	if (public_interest[v] < cap || (v == VIEW_LIBERALCRIMESQUADPOS && public_interest[v] < 100))
		public_interest[v] += abs(effpower);
	if (effpower > 0)
	{
		//Some things will never persuade the last x% of the population.
		//If there's a cap on how many people will be impressed, this
		//is where that's handled.
		if (attitude[v] + effpower > cap)
		{
			if (attitude[v] > cap)effpower = 0;
			else effpower = cap - attitude[v];
		}
	}
	//Finally, apply the effect.
	attitude[v] += effpower;
	if (attitude[v] < 0)attitude[v] = 0;
	if (attitude[v] > 100)attitude[v] = 100;
}
// Determines the number of subordinates a creature may command
int maxsubordinates(const Creature& cr)
{
	//brainwashed recruits can't recruit normally
	if (cr.flag & CREATUREFLAG_BRAINWASHED)return 0;
	int recruitcap = 0;
	//Cap based on juice
	if (cr.juice >= 500)      recruitcap += 6;
	else if (cr.juice >= 200) recruitcap += 5;
	else if (cr.juice >= 100) recruitcap += 3;
	else if (cr.juice >= 50)  recruitcap += 1;
	//Cap for founder
	if (cr.hireid == -1 && cr.align == 1) recruitcap += 6;
	return recruitcap;
}
int loveslaves(const Creature& cr);
// Determines the number of love slaves a creature may recruit,
// based on max minus number they already command
int loveslavesleft(const Creature& cr)
{
	// Get maximum lovers
	int loveslavecap = cr.get_skill(SKILL_SEDUCTION) / 2 + 1;
	// -1 if they're a love slave (their boss is a lover)
	if (cr.flag & CREATUREFLAG_LOVESLAVE) loveslavecap--;
	// Subtract number of love slaves they have
	loveslavecap -= loveslaves(cr);
	// If they can have more, return that number
	if (loveslavecap > 0) return loveslavecap;
	// If they're at 0 or less, return 0
	else return 0;
}

/* common - random issue by public interest */
int randomissue(bool core_only)
{
	extern char newscherrybusted;
	extern char endgamestate;
	extern short public_interest[VIEWNUM];
	int interest_array[VIEWNUM], total_interest = 0;
	int numviews = core_only ? VIEWNUM - 5 : ((endgamestate >= ENDGAME_CCS_DEFEATED || newscherrybusted < 2) ? VIEWNUM - 1 : VIEWNUM);
	for (int i = 0; i < numviews; i++)
	{
		interest_array[i] = public_interest[i] + total_interest + 25;
		total_interest += public_interest[i] + 25;
	}
	int roll = LCSrandom(total_interest);
	for (int i = 0; i < numviews; i++) if (roll < interest_array[i]) return i;
	return VIEW_MOOD;
}

/* common - Sort a list of creatures.*/
inline bool sort_name(const Creature* first, const Creature* second) { return strcmp(first->name, second->name) < 0; }
bool sort_locationandname(const Creature* first, const Creature* second)
{
	return first->location < second->location
		|| (first->location == second->location
			&&sort_name(first, second));
}
bool sort_squadorname(const Creature* first, const Creature* second)
{
	extern vector<squadst *> squad;
	// Use getsquad to treat members of a new squad being assembled as if not in a squad.
	bool first_in_squad = getsquad(first->squadid) != -1;
	bool second_in_squad = getsquad(second->squadid) != -1;
	bool a = ((first_in_squad && !second_in_squad) //Squad member should come before squadless.
		|| (first_in_squad
			&& first->squadid < second->squadid) //Older squads above newer.
		|| (!first_in_squad && !second_in_squad
			&& sort_name(first, second))); //Sort squadless by name.
										   //Sort members of same squad in the order they are in the squad.
	if (first_in_squad && first->squadid == second->squadid)
		for (int j = 0; j < 6; j++)
			if (squad[getsquad(first->squadid)]->squad[j]->id == first->id) return true;
			else if (squad[getsquad(first->squadid)]->squad[j]->id == second->id) return false;
			return a;
}
bool sort_none(const Creature* first, const Creature* second);
void sortliberals(std::vector<Creature *>& liberals, short sortingchoice, bool dosortnone)
{
	if (!dosortnone&&sortingchoice == SORTING_NONE) return;
	switch (sortingchoice)
	{
	case SORTING_NONE: sort(liberals.begin(), liberals.end(), sort_none); break;
	case SORTING_NAME: sort(liberals.begin(), liberals.end(), sort_name); break;
	case SORTING_LOCATION_AND_NAME: sort(liberals.begin(), liberals.end(), sort_locationandname); break;
	case SORTING_SQUAD_OR_NAME: sort(liberals.begin(), liberals.end(), sort_squadorname); break;
	}
}
map<short, string> trainingActivitySorting;
vector<string> methodOfSorting;
const string mostlyendings = "mostlyendings\\";
const string CONST_commonactions009 = "methodOfSorting.txt";
vector<file_and_text_collection> common_text_file_collection = {
customText(&methodOfSorting, mostlyendings + CONST_commonactions009),
};
/* common - Prompt to decide how to sort liberals.*/
void sorting_prompt(short listforsorting)
{
	const string CONST_commonactions010 = "Choose how to sort list of ";
	extern short activesortingchoice[SORTINGCHOICENUM];
	eraseAlt();
	set_color_easy(WHITE_ON_BLACK);
	mvaddstrAlt(1, 1, CONST_commonactions010);
	if (trainingActivitySorting.count(listforsorting)) {
		addstrAlt(trainingActivitySorting[listforsorting]);
	}
	else {
		addstrAlt(trainingActivitySorting[SORTINGCHOICENUM]);
	}
	for (int i = 0; i < len(methodOfSorting); i++) {
		mvaddstrAlt(3 + i, 2, methodOfSorting[i]);
	}
	while (true)
	{
		int c = getkeyAlt();
		if (c == 'a')
		{
			activesortingchoice[listforsorting] = SORTING_NONE; break;
		}
		else if (c == 'b')
		{
			activesortingchoice[listforsorting] = SORTING_NAME; break;
		}
		else if (c == 'c')
		{
			activesortingchoice[listforsorting] = SORTING_LOCATION_AND_NAME; break;
		}
		else if (c == 'd')
		{
			activesortingchoice[listforsorting] = SORTING_SQUAD_OR_NAME; break;
		}
		else if (c == 'x' || c == ENTER || c == ESC || c == SPACEBAR) break;
	}
}
/* common - Returns appropriate sortingchoice enum value for a reviewmode enum value.
Is currently unnecessary unless the enums are changed.*/
short reviewmodeenum_to_sortingchoiceenum(short reviewmode)
{
	switch (reviewmode)
	{
	case REVIEWMODE_LIBERALS: return SORTINGCHOICE_LIBERALS;
	case REVIEWMODE_HOSTAGES: return SORTINGCHOICE_HOSTAGES;
	case REVIEWMODE_CLINIC: return SORTINGCHOICE_CLINIC;
	case REVIEWMODE_JUSTICE: return SORTINGCHOICE_JUSTICE;
	case REVIEWMODE_SLEEPERS: return SORTINGCHOICE_SLEEPERS;
	case REVIEWMODE_DEAD: return SORTINGCHOICE_DEAD;
	case REVIEWMODE_AWAY: return SORTINGCHOICE_AWAY;
	default: return 0;//-1;
	}
}
string selectA;
string selectAn;
string enterDash;
/* common - Displays options to choose from and returns an int corresponding
to the index of the option in the vector. */
int choiceprompt(const string &firstline, const string &secondline,
	const vector<string> &option, const string &optiontypename,
	bool allowexitwochoice, const string &exitstring)
{
	extern short interface_pgup;
	extern short interface_pgdn;
	int page = 0;
	while (true)
	{
		eraseAlt();
		set_color_easy(WHITE_ON_BLACK_BRIGHT);
		mvaddstrAlt(0, 0, firstline);
		set_color_easy(WHITE_ON_BLACK);
		mvaddstrAlt(1, 0, secondline);
		//Write options
		for (int p = page * 19, y = 2; p < len(option) && p < page * 19 + 19; p++, y++)
		{
			mvaddcharAlt(y, 0, 'A' + y - 2); addstrAlt(spaceDashSpace);
			addstrAlt(option[p]);
		}
		set_color_easy(WHITE_ON_BLACK);
		moveAlt(22, 0);
		switch (optiontypename[0])
		{
		case 'a': case 'e': case 'i': case 'o': case 'u':
		case 'A': case 'E': case 'I': case 'O': case 'U':
			addstrAlt(selectAn + optiontypename); break;
		default:
			addstrAlt(selectA + optiontypename); break;
		}
		mvaddstrAlt(23, 0, addpagestr());
		moveAlt(24, 0);
		if (allowexitwochoice) addstrAlt(enterDash + exitstring);
		int c = getkeyAlt();
		//PAGE UP
		if ((c == interface_pgup || c == KEY_UP || c == KEY_LEFT) && page > 0) page--;
		//PAGE DOWN
		if ((c == interface_pgdn || c == KEY_DOWN || c == KEY_RIGHT) && (page + 1) * 19 < len(option)) page++;
		if (c >= 'a'&&c <= 's')
		{
			int p = page * 19 + c - 'a';
			if (p < len(option)) return p;
		}
		if (allowexitwochoice && (c == 'x' || c == ENTER || c == ESC || c == SPACEBAR)) break;
	}
	return -1;
}
/* common - Displays a list of things to buy and returns an int corresponding
to the index of the chosen thing in the nameprice vector. */
int buyprompt(const string &firstline, const string &secondline,
	const vector< pair<string, int> > &nameprice, int namepaddedlength,
	const string &producttype, const string &exitstring)
{
	const string CONST_commonactions012 = "$";
	extern short interface_pgup;
	extern short interface_pgdn;
	extern class Ledger ledger;
	int page = 0;
	while (true)
	{
		eraseAlt();
		set_color_easy(WHITE_ON_BLACK);
		mvaddstrAlt(0, 0, firstline);
		mvaddstrAlt(1, 0, secondline);
		//Write wares and prices
		for (int p = page * 19, y = 2; p < len(nameprice) && p < page * 19 + 19; p++)
		{
			if (nameprice[p].second > ledger.get_funds())
				set_color_easy(BLACK_ON_BLACK_BRIGHT);
			else set_color_easy(WHITE_ON_BLACK);
			mvaddcharAlt(y, 0, 'A' + y - 2); addstrAlt(spaceDashSpace);
			addstrAlt(nameprice[p].first);
			moveAlt(y++, namepaddedlength + 4); //Add 4 for start of line, eg A - .
			addstrAlt(CONST_commonactions012 + tostring(nameprice[p].second));
		}
		set_color_easy(WHITE_ON_BLACK);
		moveAlt(22, 0);
		switch (producttype[0])
		{
		case 'a': case 'e': case 'i': case 'o': case 'u':
		case 'A': case 'E': case 'I': case 'O': case 'U':
			addstrAlt(selectAn + producttype); break;
		default:
			addstrAlt(selectA + producttype); break;
		}
		mvaddstrAlt(23, 0, addpagestr());
		mvaddstrAlt(24, 0, enterDash + exitstring);
		int c = getkeyAlt();
		//PAGE UP
		if ((c == interface_pgup || c == KEY_UP || c == KEY_LEFT) && page > 0) page--;
		//PAGE DOWN
		if ((c == interface_pgdn || c == KEY_DOWN || c == KEY_RIGHT) && (page + 1) * 19 < len(nameprice)) page++;
		if (c >= 'a'&&c <= 's')
		{
			int p = page * 19 + c - 'a';
			if (p < len(nameprice) && nameprice[p].second <= ledger.get_funds())
				return p;
		}
		if (c == 'x' || c == ENTER || c == ESC || c == SPACEBAR) break;
	}
	return -1;
}
/* tells how many total members a squad has (including dead members) */
int squadsize(const squadst *st)
{
	int partysize = 0;
	if (st) for (int p = 0; p < 6; p++) if (st->squad[p]) partysize++;
	return partysize;
}
/* tells how many members a squad has who are alive */
int squadalive(const squadst *st)
{
	int partyalive = 0;
	if (st) for (int p = 0; p < 6; p++) if (st->squad[p]) if (st->squad[p]->alive) partyalive++;
	return partyalive;
}