#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>

#include <glib.h>

#include "confdir.h"

/* TODO better errror handling */

static void do_mkdir(const char *path) {
	if(mkdir(path,0777)) {
		perror("Cannot create conf directory, quitting");
	}
}

static gboolean is_directory(const char *path) {
	struct stat st;
	
	if(lstat(path,&st)==-1) {
	  return 0;
	}
	if(S_ISDIR(st.st_mode)) {
	  return 1;
	}
	return 0;
}

static void mkdir_if_needed(const char *path) {
	if(is_directory(path))
		return;
	do_mkdir(path);
	if(!is_directory(path))
		perror("Cannot create conf directory, quitting");
}

/* Below is scarily OS-specific */
static guint32 our_pid() {
	pid_t p;
	
	p=getpid();
	return (guint32)p;
}

static gboolean process_exists(guint32 pid) {
	if(!kill(pid,0))
		return TRUE;
	if(errno==ESRCH)
		return FALSE;
	return TRUE; /* Don't know, err on the side of caution */
}

static gchar ** list_dir(gchar *dirname) {
	GArray * ents;
	GDir *dir;
	const gchar *next;
	gchar **out;
	int i;
	
	ents=g_array_new(FALSE,FALSE,sizeof(const gchar *));
	dir=g_dir_open(dirname,0,0);
	if(!dir)
		g_error("Cannot list directory '%s'",dirname);
	while((next=g_dir_read_name(dir))) {
		g_array_append_val(ents,next);
	}
	out=g_new(gchar *,ents->len+1);
	for(i=0;i<ents->len;i++)
		out[i]=g_strdup(g_array_index(ents,const gchar *,i));
	out[ents->len]=0;
	g_array_free(ents,TRUE);
	g_dir_close(dir);
	return out;
}

static void list_dir_close(gchar **out) {
	gchar **idx;
	
	for(idx=out;*idx;idx++)
		g_free(*idx);	
	g_free(out);
}

static guint32 string_to_int(gchar *str,gboolean *good) {
	guint64 out;
	gchar *end=0;
	
	if(good)
		*good=FALSE;
	out=g_ascii_strtoull(str,&end,16);
	if(out>G_MAXUINT32) {
		return G_MAXUINT32;
	}
	if(*end) {
		return G_MAXUINT32;
	}
	*good=TRUE;
	return (guint32)out;
}

static void destroy_tmp_dir(gchar *tmpdir) {
	gchar **dir,**ent;
	gchar *file;

	g_debug("Destroying tmpdir '%s'",tmpdir);
	dir=list_dir(tmpdir);
	for(ent=dir;*ent;ent++) {
		file=g_strdup_printf("%s/%s",tmpdir,*ent);
		if(is_directory(file))
			destroy_tmp_dir(file);
		else
			g_unlink(file);
		g_free(file);
	}
	list_dir_close(dir);
	g_rmdir(tmpdir);
}

static void tidy_unused_tmp_dirs(gchar *tmpbase) {
	gchar **dir,**ent,*path;
	gint32 pid=0;
	gboolean good=TRUE;
	
	dir=list_dir(tmpbase);
	for(ent=dir;*ent;ent++) {
		pid=string_to_int(*ent,&good);
		if(!good)
			continue;
		if(process_exists(pid)) {
			g_debug("Keeping tmp dir, pid=%d/%8.8X exists",pid,pid);
			continue;
		}
		g_debug("Process pid=%d/%8.8X does not exist. Tidying up",pid,pid);
		path=g_strdup_printf("%s/%s",tmpbase,*ent);
		destroy_tmp_dir(path);
		g_free(path);
	}
	list_dir_close(dir);
}

/* Handle leakiness of Glib, sigh! Helps with valgrind suppression, too */
static const gchar * homedir() {
	static const gchar *home=0;

	if(!home)
		home=g_get_home_dir();
	return home;
}

potash_confdir po_confdir_setup(gchar *basedir) {
	potash_confdir confdir;
	gchar *tmpdir,*tmpfile;
  
	confdir=g_new(struct _potash_confdir,1);
	/* Base directory */
	confdir->basedir=g_strdup_printf("%s/%s",homedir(),basedir);
	g_debug("basedir ::: %s",confdir->basedir);
	mkdir_if_needed(confdir->basedir);
	/* Temporary directory root */
	tmpdir=g_strdup_printf("%s/tmp",confdir->basedir);
	mkdir_if_needed(tmpdir);
	/* Temporary directory for our process */
	confdir->tmpdir=g_strdup_printf("%s/%8.8X",tmpdir,our_pid());
	g_debug("tmpdir ::: %s",confdir->tmpdir);
	mkdir_if_needed(confdir->tmpdir);
	/* Create test file */
	tmpfile=g_strdup_printf("%s/tmpdir.lock",confdir->tmpdir);
	g_file_set_contents(tmpfile,"",-1,0);
	g_free(tmpfile);
	/* Free temporary allocations */
	tidy_unused_tmp_dirs(tmpdir);
	g_free(tmpdir);
	return confdir;
}

void po_confdir_destroy(potash_confdir cd,gboolean tidy_up) {
	if(tidy_up)
		destroy_tmp_dir(cd->tmpdir);
	g_free(cd->tmpdir);
	g_free(cd->basedir);
	g_free(cd);
}

gchar * po_confdir_tmp_file(potash_confdir cd,
									 gchar *base,guint32 index,gchar *suffix) {
	gchar *out,*dir;
	int sub;
	
	sub=index%256;
	dir=g_strdup_printf("%s/%2.2X",cd->tmpdir,sub);
	mkdir_if_needed(dir);
	out=g_strdup_printf("%s/%s-%8.8X.%s",dir,base,index,suffix);
	g_free(dir);
	return out;
}
