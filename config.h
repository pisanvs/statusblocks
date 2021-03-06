#define CMDOUTLEN 75	 /* max length of command output */

#define SIGCHAR	10	/* Used for clickable blocks, max clickable blocks
					   is n-1 (needs to match dwm setting) */

static const char delimiter[] = { ' ', ' ', SIGCHAR }; /* delimeter as char array */
#define DELIMITERLENGTH  (sizeof delimiter)


// Block definition
typedef struct block_t {
	const char* cmd;
	const float interval;
	const int   signal;
	char        text[CMDOUTLEN + DELIMITERLENGTH + 1];
	size_t      length;
} Block;

// Block array

/* CMD is the command to run, INTERVAL is the interval in seconds,
   SIGNAL is the signal offset to use for the block, SIGNAL + 34 is the
   realtime signal for clicks sent by dwm, SIGNAL + 44 is the realtime
   signal for block updates */

#define CMDPREFIX "~/.local/bin/"
// TODO: signals dont work if not consecutive
Block blocks[] = {
	/* CMD              INTERVAL    SIGNAL */
	{ CMDPREFIX"sb-prices",		120,		1},
	{ CMDPREFIX"sb-forecast",    3600,		2},
	{ CMDPREFIX"sb-datetime",    0.25,		3},
	{ CMDPREFIX"sb-mem",         4,			4},
	{ CMDPREFIX"sb-cpu",         4,			5}
};