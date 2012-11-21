
#ifndef __na_cclosure_marshal_MARSHAL_H__
#define __na_cclosure_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:POINTER,BOOLEAN (/home/pierre/data/eclipse/caja-actions/src/core/na-marshal.def:3) */
extern void na_cclosure_marshal_VOID__POINTER_BOOLEAN (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* VOID:STRING,STRING,POINTER,BOOLEAN (/home/pierre/data/eclipse/caja-actions/src/core/na-marshal.def:6) */
extern void na_cclosure_marshal_VOID__STRING_STRING_POINTER_BOOLEAN (GClosure     *closure,
                                                                     GValue       *return_value,
                                                                     guint         n_param_values,
                                                                     const GValue *param_values,
                                                                     gpointer      invocation_hint,
                                                                     gpointer      marshal_data);

G_END_DECLS

#endif /* __na_cclosure_marshal_MARSHAL_H__ */

