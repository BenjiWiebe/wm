#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define SPOOL_DIR		"/var/spool/msg/"
#define SPOOL_DIR_LEN	16

/* TODO Implement testing of permissions of spooldir/spoolfile on each run, and attempt to fix??
 * To create SPOOL_DIR:
 * [user@host ~]$ sudo mkdir $SPOOL_DIR
 * [user@host ~]$ sudo chmod 700 $SPOOL_DIR
 * [user@host ~]$ sudo chgrp msg $SPOOL_DIR
 */

int logging = 1;
FILE *logfp = NULL;

void ferr(char *func)
{
	perror(func);
	exit(EXIT_FAILURE);
}

char *getspoolname(char *user)
{
	int spoolnamelen = strlen(user) + SPOOL_DIR_LEN;
	char *spoolname = (char*)malloc(spoolnamelen);
	if(!spoolname)
		ferr("malloc");
	strcpy(spoolname, SPOOL_DIR);
	strcat(spoolname, user);
	return spoolname;
}

int can_user_use_wm(char *user)
{
	char *sp = getspoolname(user);
	struct stat s;
	int res = stat(sp, &s);
	free(sp);
	if(res < 0)
		return 0;
	if(s.st_mode != 33200)
		return 0;
	return 1;
}

FILE *open_file(char *user, char *mode)
{
	char *spoolname = getspoolname(user);
	FILE *ret = fopen(spoolname, mode);
	free(spoolname);
	return ret;
}

FILE *open_log_file(void)
{
	struct passwd *p = getpwuid(getuid());
	const char file[] = {".wm_history"};
	char *path = malloc(strlen(p->pw_dir) + 2 + sizeof file);
	if(path == NULL)
		ferr("malloc");
	strcpy(path, p->pw_dir);
	strcat(path, "/");
	strcat(path, file);
	FILE *fp = fopen(path, "a");
	free(path);
	return fp;
}

void check_file(char *user)
{
	char *spoolname = getspoolname(user);
	int n = open(spoolname, O_CREAT, 0600);
	if(n < 0)
		ferr("open");
	else
		close(n);
	free(spoolname);
}

char *getusername(void)
{
	struct passwd *result = getpwuid(getuid());
	if(!result)
		ferr("getpwuid");
	return result->pw_name;
}

void cat(FILE *in, FILE *out, int do_prefix)
{
	char *buf = NULL;
	ssize_t bytesread = 0;
	size_t n = 0;
	errno = 0;
	while((bytesread = getline(&buf, &n, in)) != -1)
	{
		if(do_prefix)
			if(fputs("  ", out) < 0)
				ferr("fputs");
		if(fputs(buf, out) < 0)
			ferr("fputs");
	}
	if(errno)
		ferr("getline");
	free(buf);
}

void read_messages(char *myname)
{
	// Check spool file size; if empty, exit
	struct stat st;
	stat(getspoolname(myname), &st);
	if(!st.st_size)
		exit(EXIT_SUCCESS);

	// Open spool file to read, since it isn't empty
	FILE *spool = open_file(myname, "r+");
	if(!spool)
		ferr("fopen");
	cat(spool, stdout, 0);
	if(logging)
	{
		rewind(spool);
		cat(spool, logfp, 1);
	}
	if(ftruncate(fileno(spool), 0) < 0)
		ferr("ftruncate");
	fclose(spool);
	fclose(logfp);
	exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
	if(argc > 2)
	{
		printf("Usage: %s <user>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *myname = getusername();
	if(!can_user_use_wm(myname))
	{
		printf("Sorry, you aren't registered to use wm.\n");
		exit(EXIT_FAILURE);
	}
	check_file(myname);
	logfp = open_log_file();
	if(argc == 1)
	{
		read_messages(myname);
		exit(EXIT_SUCCESS);
	}
	if(!can_user_use_wm(argv[1]))
	{
		printf("Sorry, %s isn't registered to use wm.\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	check_file(argv[1]);
	FILE *fp = open_file(argv[1], "a");
	if(!fp)
		ferr("fopen");
	unsigned char *msg = NULL;
	size_t n;
	ssize_t msg_size = getline((char**)&msg, &n, stdin);
	if(msg_size == -1)
	{
		if(errno == 0)
			return 0;
		else
			ferr("getline");
	}
	if(msg[msg_size-1] == '\n')
		msg[--msg_size] = 0;
	if(msg_size != 0)
	{
		int i = msg_size;
		while(i--)
			if(msg[i] < 128 && !isprint(msg[i]))
				msg[i] = '?';

		if(logging)
			fprintf(logfp, "->%s: %s\n", argv[1], msg);
		fprintf(fp, "%s: %s\n", myname, msg);
	}
	if(logging)
		fclose(logfp);
	fclose(fp);
	return 0;
}
