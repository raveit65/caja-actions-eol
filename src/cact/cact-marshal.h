
#ifndef __cact_cclosure_marshal_MARSHAL_H__
#define __cact_cclosure_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:BOOLEAN,INT,INT,INT (/home/pierre/data/eclipse/caja-actions/src/cact/cact-marshal.def:2) */
extern void cact_cclosure_marshal_VOID__BOOLEAN_INT_INT_INT (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);

/* VOID:POINTER,UINT (/home/pierre/data/eclipse/caja-actions/src/cact/cact-marshal.def:6) */
extern void cact_cclosure_marshal_VOID__POINTER_UINT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* VOID:POINTER,STRING (/home/pierre/data/eclipse/caja-actions/src/cact/cact-marshal.def:9) */
extern void cact_cclosure_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

G_END_DECLS

#endif /* __cact_cclosure_marshal_MARSHAL_H__ */

