/*
 *	Program to read the date/time from the PC/AT's battery-backed-up
 *	clock, suitable for use with date.
 *	Martin C. Atkins, 10/3/87
 *
 *	Copy this freely, but Keep this comment!!
 */
#include <stdio.h>

struct tm {
	int day;
	int month;
	int year;
	int hour;
	int minute;
	int second;
} tm;


main()
{
	int attempt = 10;
	struct tm now, then;

	do {
		readtime(&then);
		readtime(&now);
	} while (--attempt > 0 && differ(&now, &then));

	if(attempt <= 0) {
		fprintf(stderr, "Can't get a consistant time\n");
		exit(1);
	}
	putnum(now.month);
	putnum(now.day);
	putnum(now.year);
	putnum(now.hour);
	putnum(now.minute);
	putnum(now.second);
	putchar('\n');
	fflush(stdout);
	exit(0);
}

readtime(tb)
struct tm *tb;
{
	tb->second = rdclock(0);
	tb->minute = rdclock(2);
	tb->hour = rdclock(4);
	tb->day = rdclock(7);
	tb->month = rdclock(8);
	tb->year = rdclock(9);
}

rdclock(port)
int port;
{
	int v;
	port_out(0x70, port);
	v = port_in(0x71);
	return (v&0x0f) + (v>>4)*10;
}

differ(a, b)
struct tm *a, *b;
{
	if(a->second != b->second || a->minute != b->minute)
		return 1;
	if(a->hour != b->hour || a->day != b->day)
		return 1;
	if(a->month != b->month || a->year != b->year)
		return 1;
	return 0;
}

putnum(v)
int v;
{
	putchar('0' + v/10);
	putchar('0' + v%10);
}
