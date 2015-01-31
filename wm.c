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
	memset(spoolname, 0, spoolnamelen);
	strcpy(spoolname, SPOOL_DIR);
	strcat(spoolname, user);
	return spoolname;
}

FILE *open_file(char *user, char *mode)
{
	char *spoolname = getspoolname(user);
	FILE *ret = fopen(spoolname, mode);
	free(spoolname);
	return ret;
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

void check_user(char *user)
{
	struct passwd *result = getpwnam(user);
	if(!result)
	{
		fprintf(stderr, "User %s does not exist.\n", user);
		exit(EXIT_FAILURE);
	}
	if(result->pw_uid < 1000)
	{
		fprintf(stderr, "User %s is a reserved system account.\n", user);
		exit(EXIT_FAILURE);
	}
}

char *whoami(void)
{
	struct passwd *result = getpwuid(getuid());
	if(!result)
		ferr("getpwuid");
	return result->pw_name;
}

void cat(FILE *in, FILE *out)
{
	char buf[256];
	int n;
	while((n = fread(buf, 1, 256, in)))
	{
		if(n == 0 && ferror(in))
			ferr("fread");
		if(fwrite(buf, 1, n, out) == 0 && ferror(out))
			ferr("fwrite");
	}
}

int main(int argc, char *argv[])
{
	if(argc > 2)
	{
		printf("Usage: %s <user>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	check_file(whoami());
	if(argc == 1)
	{
		FILE *spool = open_file(whoami(), "r+");
		if(!spool)
			ferr("fopen");
		cat(spool, stdout);
		if(ftruncate(fileno(spool), 0) < 0)
			ferr("ftruncate");
		fclose(spool);
		exit(EXIT_SUCCESS);
	}
	check_file(argv[1]);
	FILE *fp = open_file(argv[1], "a");
		if(!fp)
		ferr("fopen");
	int x;
	fprintf(fp, "%s: ", whoami());
	while((x = fgetc(stdin)))
	{
		if(x == '\r' || x == '\n' || x == EOF || x == 0)
			break;
		if(x > 128 || isprint(x))
		{
			fputc(x, fp);
		}
		else
		{
			char tmp[5];
			tmp[0] = '\\';
			tmp[1] = 'x';
			char f = x / 16;
			f > 9 ? (tmp[2] = 'A' + (f-10)) : (tmp[2] = '0' + f);
			f = x % 16;
			f > 9 ? (tmp[3] = 'A' + (f-10)) : (tmp[3] = '0' + f);
			tmp[4] = 0;
			fputs(tmp, fp);
		}
	}
	fputc((int)'\n', fp);
	fclose(fp);
	return 0;
}
