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

#ifndef __BASE_ASSISTANT_H__
#define __BASE_ASSISTANT_H__

/**
 * SECTION: base_assistant
 * @short_description: #BaseAssistant class definition.
 * @include: cact/base-assistant.h
 *
 * This class is derived from BaseWindow class, and serves as a base
 * class for all Caja-Actions assistants.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define BASE_TYPE_ASSISTANT                ( base_assistant_get_type())
#define BASE_ASSISTANT( object )           ( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_TYPE_ASSISTANT, BaseAssistant ))
#define BASE_ASSISTANT_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( klass, BASE_TYPE_ASSISTANT, BaseAssistantClass ))
#define BASE_IS_ASSISTANT( object )        ( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_TYPE_ASSISTANT ))
#define BASE_IS_ASSISTANT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_TYPE_ASSISTANT ))
#define BASE_ASSISTANT_GET_CLASS( object ) ( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_TYPE_ASSISTANT, BaseAssistantClass ))

typedef struct _BaseAssistantPrivate       BaseAssistantPrivate;

typedef struct {
	/*< private >*/
	BaseWindow            parent;
	BaseAssistantPrivate *private;
}
	BaseAssistant;

typedef struct _BaseAssistantClassPrivate  BaseAssistantClassPrivate;

/**
 * BaseAssistantClass:
 * @apply:   apply the result of the assistant.
 * @prepare: prepare a page to be displayed.
 *
 * This defines the virtual method a derived class may, should or must implement.
 */
typedef struct {
	/*< private >*/
	BaseWindowClass            parent;
	BaseAssistantClassPrivate *private;

	/*< public >*/
	/**
	 * apply:
	 * @window: this #BaseAssistant instance.
	 * @assistant: the #GtkAssistant toplevel.
	 *
	 * Invoked when the user has clicked on the 'Apply' button.
	 */
	void ( *apply )  ( BaseAssistant *window, GtkAssistant *assistant );

	/**
	 * prepare:
	 * @window: this #BaseAssistance instance.
	 * @assistant: the #GtkAssistant toplevel.
	 *
	 * Invoked when the Gtk+ runtime is preparing a page.
	 */
	void ( *prepare )( BaseAssistant *window, GtkAssistant *assistant, GtkWidget *page );
}
	BaseAssistantClass;

/**
 * Properties defined by the BaseAssistant class.
 * They should be provided at object instanciation time.
 */
#define BASE_PROP_QUIT_ON_ESCAPE				"base-assistant-quit-on-escape"
#define BASE_PROP_WARN_ON_ESCAPE				"base-assistant-warn-on-escape"

GType base_assistant_get_type( void );

G_END_DECLS

#endif /* __BASE_ASSISTANT_H__ */
