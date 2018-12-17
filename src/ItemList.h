#pragma once

#include "Globals.h"

/*
 * Type alias for item identifiers.
 */
using ItemId = unsigned int;

/**
 * A list of items.
 */
class ItemList
{
public:

	// Constructor and destructor
	ItemList();
	~ItemList();

	// It initializes the list with all the items appearing exactly once
	void initializeComplete();

	// Methods to add and remove items to/from the list
	void addItem(ItemId itemId);
	void removeItem(ItemId itemId);

	// It returns the number of items with the given Id
	unsigned int numItemsWithId(ItemId itemId);

	// Returns the total number of items in the list (counting repeated items)
	unsigned int numItems() const;

	// Returns the number of missing items (number of items from 0 to MAX_ITEMS -1 not in the list)
	unsigned int numMissingItems() const;


private:

	// Recomputes the number of missing items after adding or removing
	void recomputeMissingItems();

	int items[MAX_ITEMS] = {};
	unsigned int numberOfItems = 0;
	unsigned int numberOfMissingItems = MAX_ITEMS;
};
