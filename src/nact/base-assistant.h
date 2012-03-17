/*
 * Caja Actions
 * A Caja extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The MATE Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009, 2010 Pierre Wieser and others (see AUTHORS)
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this Library; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307, USA.
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
 * @include: nact/base-assistant.h
 *
 * This class is derived from BaseWindow class, and serves as a base
 * class for all Caja Actions assistants.
 *
 * Note: as a work-around to #589745 (Apply message in GtkAssistant),
 * we may trigger "on_assistant_apply" function from the
 * "on_prepare_message" handler.
 * The provided patch has been applied on 2009-08-07, and released in
 * Gtk+ 2.17.7. So, this work-around will can be safely removed when
 * minimal Gtk+ version will be 2.18 or later.
 */

#include "base-window.h"

G_BEGIN_DECLS

#define BASE_ASSISTANT_TYPE					( base_assistant_get_type())
#define BASE_ASSISTANT( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, BASE_ASSISTANT_TYPE, BaseAssistant ))
#define BASE_ASSISTANT_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, BASE_ASSISTANT_TYPE, BaseAssistantClass ))
#define BASE_IS_ASSISTANT( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, BASE_ASSISTANT_TYPE ))
#define BASE_IS_ASSISTANT_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), BASE_ASSISTANT_TYPE ))
#define BASE_ASSISTANT_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), BASE_ASSISTANT_TYPE, BaseAssistantClass ))

typedef struct BaseAssistantPrivate      BaseAssistantPrivate;

typedef struct {
	BaseWindow            parent;
	BaseAssistantPrivate *private;
}
	BaseAssistant;

typedef struct BaseAssistantClassPrivate BaseAssistantClassPrivate;

typedef struct {
	BaseWindowClass            parent;
	BaseAssistantClassPrivate *private;

	/**
	 * apply:
	 * @window: this #BaseAssistance instance.
	 */
	void ( *apply )  ( BaseAssistant *window, GtkAssistant *assistant );

	/**
	 * cancel:
	 * @window: this #BaseAssistance instance.
	 */
	void ( *cancel ) ( BaseAssistant *window, GtkAssistant *assistant );

	/**
	 * close:
	 * @window: this #BaseAssistance instance.
	 */
	void ( *close )  ( BaseAssistant *window, GtkAssistant *assistant );

	/**
	 * prepare:
	 * @window: this #BaseAssistance instance.
	 */
	void ( *prepare )( BaseAssistant *window, GtkAssistant *assistant, GtkWidget *page );
}
	BaseAssistantClass;

/**
 * %BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE:
 *
 * Does the assistant cancel its execution when the user hits the
 * 'Escape' key ?
 *
 * Defaults to %FALSE.
 */
#define BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE	"base-assistant-cancel-on-escape"

/**
 * %BASE_ASSISTANT_PROP_WARN_ON_ESCAPE:
 *
 * Does the user be warned when he/she quits the assistant by hitting
 * the 'Escape' key ? This is only used when previous property
 * %BASE_ASSISTANT_PROP_CANCEL_ON_ESCAPE is set to %TRUE.
 *
 * Defaults to %FALSE.
 */
#define BASE_ASSISTANT_PROP_WARN_ON_ESCAPE		"base-assistant-warn-on-escape"

/**
 * %BASE_ASSISTANT_PROP_WARN_ON_CANCEL:
 *
 * Does the user be warned when he/she cancels the assistant ?
 *
 * Defaults to %FALSE.
 */
#define BASE_ASSISTANT_PROP_WARN_ON_CANCEL		"base-assistant-warn-on-cancel"

GType base_assistant_get_type( void );

void  base_assistant_set_cancel_on_esc( BaseAssistant *window, gboolean cancel );
void  base_assistant_set_warn_on_esc( BaseAssistant *window, gboolean warn_esc );
void  base_assistant_set_warn_on_cancel( BaseAssistant *window, gboolean warn_cancel );

G_END_DECLS

#endif /* __BASE_ASSISTANT_H__ */
