#pragma once
#include <cinttypes>

// Constants ///////////////////////////////////////////////////////////

/** Hostname (or IP address) of the YellowPages process. */
static const char *HOSTNAME_YP = "localhost";

/** Listen port used by the YellowPages process. */
static const uint16_t LISTEN_PORT_YP = 8000;

/** Listen port used by the multi-agent application. */
static const uint16_t LISTEN_PORT_AGENTS = 8001;

/**
 * Constant used to specify that a message was sent to,
 * or received from no agent. This is the case when
 * communicating with the YellowPages. Yellow pages is
 * a global service that contains information about
 * contributor agents, but uses no agents to work.
 */
static const uint16_t NULL_AGENT_ID = 0;

/*
 * RANDOM INITIALIZATION:
 * Whether or not perform a random initialization of items among nodes.
 * The initialzation takes place at ModuleNodeCluster::startSystem().
 */
#define RANDOM_INITIALIZATION

/**
 * MAX_ITEMS:
 * This constant defines which is the maximum number of items of the catalogue.
 * Items will be identified by an index between 0 and MAX_ITEMS - 1.
 *
 * MAX_NODES:
 * This constant defines which is the maximum number of items of the catalogue.
 * Items will be identified by an index between 0 and MAX_ITEMS - 1.
 */

#if defined(RANDOM_INITIALIZATION)

static const unsigned int MAX_ITEMS = 10U;
static const unsigned int MAX_NODES = 10U;

#else

static const unsigned int MAX_ITEMS = 4U;
static const unsigned int MAX_NODES = 4U;

#endif

/**
 * MAX_SEARCH_DEPTH
 * This the maximum depth of the search performed by MCP/UCP agents.
 * If MAX_SEARCH_DEPTH == 0, only bilateral exchanges will be found.
 * If MAX_SEARCH_DEPTH == 1, trilateral exchanges will be also found.
 * etc.
 */
#define MAX_SEARCH_DEPTH 4
