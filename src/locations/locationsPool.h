#include "../items/itemPool.h"
class LocationsPool : ItemTypePool {
private:
	static bool locationPoolInitiated;
public:
	static LocationsPool getInstance();
	const int lenpool();
	int getStoresAmount(int l);
	void delete_and_clear_pool();
	bool canBeUpgraded(int cursite);
	const char isThereASiegeHere(int cursite);
	const int isThisPlaceHighSecurity(int cursite);
	void isThereASiegeHere(int cursite, char newCondition);
	void isThisPlaceHighSecurity(int cursite, int newCondition);
	const char getLocationType(int cursite);
	const short getSiegeType(int cursite);
	void spawnATank(int cursite, int num);
	const short getSiegeEscalationState(int cursite);
	const int getRentingType(int cursite);
	void initSite(int loc);
	void eraseAndReplaceGraffiti(int cursite, int locx, int locy, int locz);
	const string getLocationName(int cursite);
	const int getLocationCity(int cursite);
	void hideCCSSafehouses();
	void addHeat(int cursite, int heat);
	void clearHeat(int cursite);
	void clearunderattack(int cursite);
	void deleteTankTraps(int l);
	int getTimeUntilSiege(int loc, int type);
	void setTimeUntilSiege(int loc, int type, int time);
	const int getHeat(int cursite);
	const void findAllLootTypes(vector<bool>& havetype, vector<int>& loottypeindex, const vector<string>& dox);
	const bool isLocationMapped(int cursite);
	const bool isLocationHidden(int cursite);
	void setLocationMappedAndUnhidden(int cursite);
	const string getLocationNameWithGetnameMethod(int cursite, signed char a, bool b);
	const string getLocationNameWithGetnameMethod(int cursite, signed char a);
	void removeTank(int cursite);
	void addSiegeKill(int cursite);
	const int getCompoundWalls(int cursite);
	void setTimeUntilSiege(int cursite, int time);
	const bool isNewRental(int cursite);
	void evictLCSFrom(int l);
	const int isThisSiteClosed(int cursite);
	const char doesThisPlaceNeedACar(int cursite);
	const int findTravelLocation();
	const int getLocationParent(int cursite);
	int deleteSpecialItem(int slot, vector<int> loottypeindex);
	void getAssetValues(long & weaponValue, long & armorValue, long & clipValue, long & lootValue);
	const bool canBeFortified(int cursite);
	void initLocation(int cursite);
	void equipLoc(int, int);
	void stashThisLootHere(const string&, int);
	void stashThisWeaponHere(int itemindex, int shelter);
	void setSiegetimeuntillocated(int cursite, int timer);
	void stashThisArmorHere(int itemindex, int shelter);
	void captureSite(int cursite);
	void closeSite(int cursite, int sitecrime);
	bool siteHasCameras(int cursite);
	int lenloot(int cursite);
	char isThisUnderAttack(int cursite);
	void setRenting(int cursite, int renting);
	char isThisAFront(int cursite);
	string getFrontName(int cursite);
	void tickAttackTime(int cursite);
	int getAttackTime(int cursite);
	void resetAttackTime(int cursite);
	int doWeHaveTankTraps(int cursite);
	void spawnATank(int cursite);
	int getSiegeKills(int cursite);
	int getSiegeTanks(int cursite);
	void turnOffSiege(int cursite);
	bool hasTraps(int loc);
	bool lightsOff(int loc);
};
