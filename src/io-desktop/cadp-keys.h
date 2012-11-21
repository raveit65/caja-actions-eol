/*
 * Caja-Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
 * Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009-2012 Pierre Wieser and others (see AUTHORS)
 *
 * Caja-Actions is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General  Public  License  as
 * published by the Free Software Foundation; either  version  2  of
 * the License, or (at your option) any later version.
 *
 * Caja-Actions is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even  the  implied  warranty  of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  the  GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public  License
 * along with Caja-Actions; see the file  COPYING.  If  not,  see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Frederic Ruaudel <grumz@grumz.net>
 *   Rodrigo Moya <rodrigo@mate-db.org>
 *   Pierre Wieser <pwieser@trychlos.org>
 *   ... and many others (see AUTHORS)
 */

#ifndef __CADP_KEYS_H__
#define __CADP_KEYS_H__

#include <api/na-data-def.h>

G_BEGIN_DECLS

#define CADP_GROUP_DESKTOP							G_KEY_FILE_DESKTOP_GROUP
#define CADP_GROUP_PROFILE							"X-Action-Profile"

#define CADP_KEY_TYPE								G_KEY_FILE_DESKTOP_KEY_TYPE
#define CADP_VALUE_TYPE_ACTION						"Action"
#define CADP_VALUE_TYPE_MENU						"Menu"

#define CADP_KEY_PROFILES							"Profiles"

#define CADP_KEY_EXECUTION_MODE						"ExecutionMode"
#define CADP_VALUE_EXECUTION_MODE_NORMAL			"Normal"
#define CADP_VALUE_EXECUTION_MODE_MINIMIZED			"Minimized"
#define CADP_VALUE_EXECUTION_MODE_MAXIMIZED			"Maximized"
#define CADP_VALUE_EXECUTION_MODE_TERMINAL			"Terminal"
#define CADP_VALUE_EXECUTION_MODE_EMBEDDED			"Embedded"
#define CADP_VALUE_EXECUTION_MODE_DISPLAY_OUTPUT	"DisplayOutput"

#define CADP_KEY_ITEMS_LIST							"ItemsList"

#define CADP_KEY_ONLY_SHOW_IN						G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN
#define CADP_KEY_NOT_SHOW_IN						G_KEY_FILE_DESKTOP_KEY_NOT_SHOW_IN
#define CADP_KEY_NO_DISPLAY							"NoDisplay"
#define CADP_KEY_HIDDEN								G_KEY_FILE_DESKTOP_KEY_HIDDEN
#define CADP_KEY_CAPABILITIES						"Capabilities"
#define CADP_VALUE_CAPABILITY_OWNER					"Owner"
#define CADP_VALUE_CAPABILITY_READABLE				"Readable"
#define CADP_VALUE_CAPABILITY_WRITABLE				"Writable"
#define CADP_VALUE_CAPABILITY_EXECUTABLE			"Executable"
#define CADP_VALUE_CAPABILITY_LOCAL					"Local"

G_END_DECLS

#endif /* __CADP_KEYS_H__ */
