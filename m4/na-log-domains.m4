dnl define three distinct log domains, respectively for common code,
dnl plugin and NACT user interface - log handlers will be disabled
dnl when not in development mode

# serial 2 define NA-runtime log domain

AC_DEFUN([NA_LOG_DOMAINS],[
	AC_SUBST([NA_LOGDOMAIN_IO_DESKTOP],[NA-io-desktop])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_IO_DESKTOP],["NA-io-desktop"],[Log domain of desktop I/O Provider])

	AC_SUBST([NA_LOGDOMAIN_IO_MATECONF],[NA-io-mateconf])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_IO_MATECONF],["NA-io-mateconf"],[Log domain of MateConf I/O Provider])

	AC_SUBST([NA_LOGDOMAIN_IO_XML],[NA-io-xml])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_IO_XML],["NA-io-xml"],[Log domain of XML I/O])

	AC_SUBST([NA_LOGDOMAIN_CORE],[NA-core])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_CORE],["NA-core"],[Log domain of core library])

	AC_SUBST([NA_LOGDOMAIN_NACT],[NA-nact])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_NACT],["NA-nact"],[Log domain of NACT user interface])

	AC_SUBST([NA_LOGDOMAIN_PLUGIN_MENU],[NA-plugin-menu])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_PLUGIN_MENU],["NA-plugin-menu"],[Log domain of Caja Menu plugin])

	AC_SUBST([NA_LOGDOMAIN_PLUGIN_TRACKER],[NA-plugin-tracker])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_PLUGIN_TRACKER],["NA-plugin-tracker"],[Log domain of Caja Tracker plugin])

	AC_SUBST([NA_LOGDOMAIN_TEST],[NA-test])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_TEST],["NA-test"],[Log domain of test programs])

	AC_SUBST([NA_LOGDOMAIN_UTILS],[NA-utils])
	AC_DEFINE_UNQUOTED([NA_LOGDOMAIN_UTILS],["NA-utils"],[Log domain of utilities])
])
