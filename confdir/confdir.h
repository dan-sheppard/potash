#ifndef POTASH_CONFDIR_H
#define POTASH_CONFDIR_H

typedef struct _potash_confdir {
	gchar *basedir;
	gchar *tmpdir;
} *potash_confdir;

potash_confdir po_confdir_setup(gchar *basedir);
void po_confdir_destroy(potash_confdir dir,gboolean tidy_up);

gchar * po_confdir_tmp_file(potash_confdir cd,
									 gchar *base,guint32 index,gchar *suffix);

#endif
