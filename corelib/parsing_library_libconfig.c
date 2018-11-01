/* (C) Copyright 2016
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * SPDX-License-Identifier:     GPL-2.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdbool.h>
#include "generated/autoconf.h"
#include "bsdqueue.h"
#include "util.h"
#include "swupdate.h"
#include "parselib.h"

static void path_libconfig(const char **nodes, char *root, unsigned int rootsize)
{
    const char **node;
    int nbytes, left;
    char *buf;
    const char *concat;
    bool first=true;

    root[0] = '\0';

    for (node = nodes, buf = root, left = rootsize; *node != NULL; node++) {
        concat = first ? "" : ".";
        nbytes = snprintf(buf, left, "%s%s", concat, *node);
        buf += nbytes;
        left -= nbytes;
        first = false;
        if (left ==0)
            break;
    }
}

void get_value_libconfig(const config_setting_t *e, void *dest)
{
	int type = config_setting_type(e);
	switch (type) {
	case CONFIG_TYPE_INT:
		*(int *)dest = config_setting_get_int(e);
		break;
	case CONFIG_TYPE_INT64:
		*(long long *)dest = config_setting_get_int64(e);
		break;
	case CONFIG_TYPE_STRING:
		dest = (void *)config_setting_get_string(e);
		break;
	case CONFIG_TYPE_BOOL:
		*(int *)dest = config_setting_get_bool(e);
		break;
	case CONFIG_TYPE_FLOAT:
		*(double *)dest = config_setting_get_float(e);
		break;
		/* Do nothing, add if needed */
	}
}

void *get_child_libconfig(void *e, const char *name)
{
	return config_setting_get_member(e, name);
}

void iterate_field_libconfig(config_setting_t *e, iterate_callback cb, void *data)
{
	config_setting_t *entry, *elem;
	const char *str;
	int i, j;

	if (!cb)
		return;

	for (i = 0; i < config_setting_length(e); i++) {
		entry = config_setting_get_elem(e, i);
		if (!config_setting_length(entry)) {
			str = config_setting_get_string(entry);
			cb(entry->name, str, data);
		} else {
			for (j = 0; j < config_setting_length(entry); j++) {
				elem = config_setting_get_elem(entry, j);
				str = config_setting_get_string(elem);
				cb(entry->name, str, data);
			}
		}
	}
}

void get_field_cfg(config_setting_t *e, const char *path, void *dest)
{
	config_setting_t *elem;

	if (path)
		elem = config_setting_lookup(e, path);
	else
		elem = e;

	if (!elem)
		return;

	get_value_libconfig(elem, dest);
}

const char *get_field_string_libconfig(config_setting_t *e, const char *path)
{
	config_setting_t *elem;
	const char *str;

	if (path)
		elem = config_setting_lookup(e, path);
	else
		elem = e;

	if (!elem || config_setting_type(elem) != CONFIG_TYPE_STRING)
		return NULL;

	if ( ( ( path) && (config_setting_lookup_string(e, path, &str))  ) ||
	     ( (!path) && ((str = config_setting_get_string(e)) != NULL) ) ) {

		return str;

	}

	return NULL;
}

void *get_node_libconfig(config_t *cfg, const char **nodes)
{
	config_setting_t *setting;
	char root[1024];

	path_libconfig(nodes, root, sizeof(root));
	setting = config_lookup(cfg, root);
	if (setting)
		return setting;

	return NULL;
}

void *find_root_libconfig(config_t *cfg, const char **nodes, unsigned int depth)
{
	config_setting_t *elem;
	char root[1024];
	const char *ref;
	char **tmp = NULL;

	/*
	 * check for deadlock links, block recursion
	 */
	if (!(--depth))
		return NULL;

	path_libconfig(nodes, root, sizeof(root));

	/*
	 * If this is root node for the device,
	 * it is a group and lenght is not 0.
	 * If it is a link, follow it
	 */
	elem = config_lookup(cfg, root);

	if (elem && config_setting_is_group(elem) == CONFIG_TRUE) {
		ref = get_field_string_libconfig(elem, "ref");
		if (ref) {
			if (!set_find_path(nodes, ref, tmp))
				return NULL;
			elem = find_root_libconfig(cfg, nodes, depth);
			free_string_array(tmp);
		}
	}

	return elem;

}
