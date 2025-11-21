/* at - run commands at a specific time		Author: Jan Looyen */
 

/* Examples:	at 2315 Jan 31 myfile	# myfile executed Jan 31 at 11:15 pm
 *		at 0900			# job input read from stdin
 *		at 0711 4 29 		# read from stdin, exec on April 29
 *
 *	To see output, commands must redirect it to e.g., >/dev/tty0.
 */


#define		DIR		"/usr/spool/at/
#define		STARTDAY	0		/*   see ctime(3) 	  */
#define		LEAPDAY		STARTDAY+59
#define		MAXDAYNR	STARTDAY+364
#define		NODAY		-2

#include	<stdio.h>
#include	<sys/types.h>
#include	<time.h>

main(argc, argv, envp)
int  argc;
char **argv, **envp;
{
    int		i, count, ltim, year, getltim(), getlday();
    int		lday = NODAY;
    char	c, buf[10], job[30], *dp, *sp;
    struct tm	*p, *localtime();
    long	clock;
    FILE	*fp, *pin, *popen();

/*-------------------------------------------------------------------------*
 *	check arguments						           *
 *-------------------------------------------------------------------------*/
    if (argc < 2 || argc > 5) {
	fprintf(stderr, "Usage: %s time [month day] [file]\n", argv[0]);
	exit(0);
    }
    if ((ltim = getltim(argv[1])) == -1) {
	fprintf(stderr, "%s: wrong time specification\n", argv[0]);
	exit(0);
    }
    if ((argc==4 || argc==5) && (lday = getlday(argv[2], argv[3]))==-1) {
	fprintf(stderr, "%s: wrong date specification\n", argv[0]);
	exit(0);
    }
    if ((argc==3 || argc==5) && open(argv[argc-1], 0) == -1) {
	fprintf(stderr, "%s: cannot find: %s\n", argv[0], argv[argc-1]);
	exit(0);
    }
/*-------------------------------------------------------------------------*
 *	determine execution time and create 'at' job file		   *
 *-------------------------------------------------------------------------*/
    time(&clock);
    p = localtime(&clock);
    year = p->tm_year;
    if (lday==NODAY) {				   /* no [month day] given */
	lday = p->tm_yday;
	if (ltim <= (p->tm_hour*100 + p->tm_min)) {
	    lday++;
	    if (lday==MAXDAYNR && (year%4) || lday==MAXDAYNR+1) {
		lday = STARTDAY;
		year++;
	    }
	}
    }
    else
	switch (year%4) {
	    case 0: if (lday < p->tm_yday || lday == p->tm_yday &&
			ltim <= (p->tm_hour*100 + p->tm_min)      ) {
			year++;
			if (lday > LEAPDAY) lday-- ;
		    }
		    break;
	    case 1:
	    case 2: if (lday > LEAPDAY) lday-- ;
		    if (lday < p->tm_yday || lday == p->tm_yday &&
			ltim <= (p->tm_hour*100 + p->tm_min)      )
			year++;
		    break;
	    case 3: if (lday < ((lday > LEAPDAY) ? p->tm_yday+1 : p->tm_yday) ||
			lday== ((lday > LEAPDAY) ? p->tm_yday+1 : p->tm_yday) &&
			ltim <= (p->tm_hour*100 + p->tm_min)		    )
			year++;
		    else if (lday > LEAPDAY) lday--;
		    break;
	}
    if ((pin = popen("pwd", "r")) == NULL)
	fprintf(stderr, "%s: cannot open pipe to cmd 'pwd'\n", argv[0]);    
    sprintf(job, DIR%02d.%03d.%04d.%02d", year%100, lday, ltim, getpid()%100);
    if ((fp = fopen(job, "w")) == NULL) {
	fprintf(stderr, "%s: cannot create %s\n", argv[0], job);
	exit(0);
    }
/*-------------------------------------------------------------------------*
 *	write environment and command(s) to 'at'job file		   *
 *-------------------------------------------------------------------------*/
    while (envp[i] != NULL) {
	count = 1;
	dp = buf;
	sp = envp[i];
	while ((*dp++ = *sp++) != '=')
	    count++;
	*--dp = '\0';
	fprintf(fp, "export %s; %s='%s'\n", buf, buf, &envp[i++][count]);
    }
    fprintf(fp, "cd ");
    while ((c = getc(pin)) != EOF)
	putc(c, fp);
    fprintf(fp, "umask %d\n", umask());
    if (argc==3 || argc==5)
	fprintf(fp, "%s\n", argv[argc-1]);
    else 					     /* read from stdinput */
	while ((c = getchar()) != EOF)
	    putc(c, fp); 
    fclose(fp);

    printf("%s: %s created\n", argv[0], job);		
}

/*-------------------------------------------------------------------------*
 *	getltim()							   *
 *-------------------------------------------------------------------------*/

getltim(t)			      /* return( (time OK) ? daytime : -1) */
char *t;
{
    if (t[4] == '\0' && t[3] >= '0' && t[3] <= '9' &&
        t[2] >= '0'  && t[2] <= '5' && t[1] >= '0' && t[1] <= '9' &&
        (t[0] == '0' || t[0] == '1' || t[1] <= '3' && t[0] == '2')   )
	return(atoi(t));
    else
	return(-1);
}

/*-------------------------------------------------------------------------*
 *	getltday()							   *
 *-------------------------------------------------------------------------*/

getlday(m, d)		        /* return ( (date OK) ? date number : -1 ) */
char *m, *d;
{
    int	 month, day;

    month = 0;
    if (m[1]=='\0') {
	if ((month = atoi(m)) < 1 || month > 9) month = 0; 
    }
    else if (m[2]=='\0') {
	if ((month = atoi(m)) < 10 || month > 12) month = 0;
    }
    else if (m[3]=='\0') {
        if (m[0]=='J' || m[0]=='j') {
    	    if (m[1]=='a' && m[2]=='n') month = 1;
    	    else if (m[1]=='u') {
    	       if (m[2]=='n') month = 6;
    	       else if (m[2]=='l') month = 7;
            }
	}  
        else if (m[0]=='M' || m[0]=='m') {
    	    if (m[1]=='a') {
     	        if (m[2]=='r') month = 3;
    	        else if (m[2]=='y') month = 5;
  	    }
	}  
        else if (m[0]=='A' || m[0]=='a') {
    	    if (m[1]=='p' && m[2]=='r') month = 4;
    	    else if (m[1]=='u' && m[2]=='g') month = 8;
        }
        else if ((m[0]=='F' || m[0]=='f') && m[1]=='e' && m[2]=='b')month = 2;
        else if ((m[0]=='S' || m[0]=='s') && m[1]=='e' && m[2]=='p')month = 9;
        else if ((m[0]=='O' || m[0]=='o') && m[1]=='c' && m[2]=='t')month = 10;
        else if ((m[0]=='N' || m[0]=='n') && m[1]=='o' && m[2]=='v')month = 11;
        else if ((m[0]=='D' || m[0]=='d') && m[1]=='e' && m[2]=='c')month = 12;
    }
    if (!month) return (-1);


    day = 0;
    if (d[1] == '\0') {
	if ((day = atoi(d)) <1 || day > 9) return(-1);
    }
    else if (d[2] == '\0') {
	if ((day = atoi(d)) <10 || day > 31) return(-1);
  	else if (month==2 && day > 29) return(-1);
	else if (day==31 && ( month==4 || month==6 || month==9 || month==11))
	    return(-1);
    } 

    if (!STARTDAY)day--;
    switch (month) {
	case  1: return(day);
	case  2: return(day + 31);
	case  3: return(day + 60);
	case  4: return(day + 91);
	case  5: return(day + 121);
	case  6: return(day + 152);
	case  7: return(day + 182);
	case  8: return(day + 213);
	case  9: return(day + 244);
	case 10: return(day + 274);
	case 11: return(day + 305);
	case 12: return(day + 335); 
    }
}
