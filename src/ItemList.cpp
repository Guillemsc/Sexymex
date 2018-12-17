#include "ItemList.h"
#include "Globals.h"
#include <cassert>
#include <algorithm>


ItemList::ItemList()
{
}


ItemList::~ItemList()
{
}

void ItemList::initializeComplete()
{
	for (ItemId itemId = 0; itemId < MAX_ITEMS; ++itemId)
	{
		items[itemId] = 1;
	}

	numberOfItems = MAX_ITEMS;
	numberOfMissingItems = 0;
}

void ItemList::addItem(ItemId itemId)
{
	assert(itemId < MAX_ITEMS && "ItemsList::addItem() - itemId out of bounds.");
	items[itemId]++;
	numberOfItems++;
	recomputeMissingItems();
}

void ItemList::removeItem(ItemId itemId)
{
	assert(itemId < MAX_ITEMS && "ItemsList::removeItem() - itemId out of bounds.");
	assert(items[itemId] > 0 && "ItemsList::removeItem() - the list does not contain this item.");
	items[itemId]--;
	numberOfItems--;
	recomputeMissingItems();
}

unsigned int ItemList::numItemsWithId(ItemId itemId)
{
	assert(itemId < MAX_ITEMS && "ItemsList::numItemsWithId() - itemId out of bounds.");
	return items[itemId];
}

unsigned int ItemList::numItems() const
{
	return numberOfItems;
}

unsigned int ItemList::numMissingItems() const
{
	return numberOfMissingItems;
}

void ItemList::recomputeMissingItems()
{
	numberOfMissingItems = 0;
	for (ItemId itemId = 0; itemId < MAX_ITEMS; ++itemId) {
		if (items[itemId] == 0) {
			numberOfMissingItems++;
		}
	}
}
