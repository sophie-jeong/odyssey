
/*
 * odissey.
 *
 * PostgreSQL connection pooler and request router.
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <flint.h>
#include <soprano.h>

#include "od_macro.h"
#include "od_list.h"
#include "od_log.h"
#include "od_scheme.h"
#include "od_lex.h"
#include "od_config.h"
#include "od_server.h"
#include "od_server_pool.h"
#include "od_route_id.h"
#include "od_route.h"
#include "od_route_pool.h"

void od_routepool_init(odroute_pool_t *pool)
{
	od_listinit(&pool->list);
	pool->count = 0;
}

void od_routepool_free(odroute_pool_t *pool)
{
	odroute_t *route;
	odlist_t *i, *n;
	od_listforeach_safe(&pool->list, i, n) {
		route = od_container_of(i, odroute_t, link);
		od_routefree(route);
	}
}

odroute_t*
od_routepool_new(odroute_pool_t *pool, odscheme_route_t *scheme,
                 odroute_id_t *id)
{
	odroute_t *route = od_routealloc();
	if (route == NULL)
		return NULL;
	int rc;
	rc = od_routeid_copy(&route->id, id);
	if (rc == -1) {
		od_routefree(route);
		return NULL;
	}
	route->scheme = scheme;
	od_listappend(&pool->list, &route->link);
	pool->count++;
	return route;
}

void od_routepool_unlink(odroute_pool_t *pool, odroute_t *route)
{
	assert(pool->count > 0);
	pool->count--;
	od_listunlink(&route->link);
	od_routefree(route);
}

odroute_t*
od_routepool_match(odroute_pool_t *pool, odroute_id_t *key)
{
	odroute_t *route;
	odlist_t *i;
	od_listforeach(&pool->list, i) {
		route = od_container_of(i, odroute_t, link);
		if (od_routeid_compare(&route->id, key))
			return route;
	}
	return NULL;
}