/** Copyright (C) 2022 pisanvs 
 * 	@license GPLv3
 *  @author pisanvs
 *  @website https://pisanvs.cl
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <X11/Xlib.h>

#include "config.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define STATUSLENGTH (LENGTH(blocks) * (CMDOUTLEN + DELIMITERLENGTH) + 1)

#ifdef DEBUG
    #define dbg_printf(...) printf("[DEBUG] "__VA_ARGS__)
#else
    #define dbg_printf(...) do {} while (0)
#endif


#define bool u_int8_t

Display     	*dpy;
static sigset_t sigmask;
bool        	BlockDue[LENGTH(blocks)];
bool        	updatePending = True;
long int    	it = 0;

/**
 * @brief spawn new process `cmd` 
 * 
 * @param cmd string to be executed in shell
 */
void spawn(const char *cmd) {
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		system(cmd);
		exit(0);
	}
}

/**
 * @brief spawn new process `cmd` and get its output
 * 
 * @param cmd string to be executed in shell
 * @param MAXOUT max output buffer size
 * @return char* output of cmd
 */
char* spawn_out(const char *cmd, size_t MAXOUT) {
	FILE *fp;
	char ret[MAXOUT];
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command: %s\n", cmd);
		exit(1);
	}
	while (fgets(ret, sizeof(ret), fp) != NULL) {
		// make new string for return
		char *new_str = malloc(sizeof(char) * (strlen(ret) + 1));
		strcpy(new_str, ret);
		return new_str;
	}
	pclose(fp);
	return NULL;
}

/**
 * @brief Update blocks if due
 */
void updateBlocks() {
	for (int i = 0; i < LENGTH(blocks); i++) {
		if (it % (int) (blocks[i].interval*4) == 0)
			BlockDue[i] = updatePending = True;
		if (!BlockDue[i])
			continue;
		Block *b = &blocks[i];
		dbg_printf("Block: %s\n", b->cmd);
		FILE *fp;
		char ret[CMDOUTLEN];
		fp = popen(b->cmd, "r");
		if (fp == NULL) {
			dbg_printf("Failed to run command\n");
			exit(1);
		}
		char nd[DELIMITERLENGTH] = {delimiter[0], delimiter[1], delimiter[2] - i};
		while (fgets(ret, sizeof(ret), fp) != NULL) {
			dbg_printf("RET: %s\n", ret);
			memset(b->text, 0, sizeof(b->text));
			strncat(b->text, nd, 3);
			strncat(b->text, ret, CMDOUTLEN);
		}
		pclose(fp);
		BlockDue[i] = 0;
		updatePending = True;
	}
}

/**
 * @brief update status message using blocks' text property
 * 
 * @param d X11 display to update
 */
void updateStatus(Display *d) {
	char *s = (char *)malloc(sizeof(char) * STATUSLENGTH);
	memset(s, 0, sizeof(char) * STATUSLENGTH);
	size_t len = 0;

	for (int i = 0; i < LENGTH(blocks); i++) {
		sprintf(s + len, "%s", blocks[i].text);
		len += strlen(blocks[i].text);
	}

	dbg_printf("STATUS UPDATE: %s\n", s);
	XStoreName(d, DefaultRootWindow(d), s);
	XSync(d, False);
	free(s);
}

/**
 * @brief exit gracefully on signal
 * 
 * @param sig signal number
 */
void term_handler(int sig) {
	XCloseDisplay(dpy);
	exit(0);
}

/**
 * @brief signal handler that marks block as due
 * 
 * @param sig signal number 
 * @param info signal info
 */
void sigupdate(int sig, siginfo_t *info, void *context) {
	BlockDue[sig - 1] = True;
}

/**
 * @brief signal handler that handles dwm status clicks
 * 
 * @param sig signal number
 * @param info signal info
 */
void sigclick(int sig, siginfo_t *info, void *context) {
	sig -= SIGRTMIN;
	Block *b = &blocks[SIGCHAR - sig];
	// run command without checking output
	char *cmd = (char *)malloc(strlen(b->cmd)+3); // +1 for space +1 for number +1 for null
	sprintf(cmd, "%s %d", b->cmd, info->si_value.sival_int);
	spawn(cmd);
	free(cmd);
}

/**
 * @brief initialize signal handlers
 */
void signal_init() {
	struct sigaction sa;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGHUP);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, SIGTERM);
	for (int i = 0; i < LENGTH(blocks); i++) {
		if (blocks[i].signal == 0)
			continue;
		if (blocks[i].signal > SIGRTMAX - SIGRTMIN) {
			fprintf(stderr, "Signal %d is too high for block %d\n", blocks[i].signal, i);
			exit(1);
		}
		sigaddset(&sigmask, blocks[i].signal);
	}

	// Handle HUP, INT, TERM with term_handler
	sa.sa_handler = term_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGHUP, &sa, NULL) == -1 ||
	   sigaction(SIGINT, &sa, NULL) == -1 ||
	   sigaction(SIGTERM, &sa, NULL) == -1) {
		fprintf(stderr, "Failed to set signal handler\n");
		exit(1);
	}

	// ignore unused RT signals
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	for (int i = SIGRTMIN; i <= SIGRTMAX; i++) {
		dbg_printf("Ignoring signal %d\n", i);
		sigaddset(&sigmask, i);
	}

	// handle block click signals with sigclick
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sa.sa_sigaction = sigclick;
	for (int i = 0; i < LENGTH(blocks); i++) {
		if (blocks[i].signal == 0)
			continue;
		dbg_printf("Registering signal %d\n", SIGCHAR + 1 - blocks[i].signal);
		if(sigaction(SIGRTMIN + SIGCHAR + 1 - blocks[i].signal, &sa, NULL) == -1) {
			fprintf(stderr, "Failed to set signal handler\n");
			exit(1);
		}
	}

	// handle individual update signals
	sa.sa_flags = SA_NODEFER | SA_RESTART | SA_SIGINFO;
	sa.sa_sigaction = sigupdate;
	for (int i = 0; i < LENGTH(blocks); i++) {
		if (blocks[i].signal == 0)
			continue;
		sigaction(SIGRTMIN + SIGCHAR + blocks[i].signal, &sa, NULL);
	}
}

int main() {
	// initialize X11 connection
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "Could not open display.");
		return 1;
	}

	// initialize vars
	signal_init();
	memset(BlockDue, 1, LENGTH(blocks));

	// main loop
	do {
		updateBlocks();
		dbg_printf("Update pending: %d\n", updatePending);
		if (updatePending) {
			updateStatus(dpy);
			updatePending = False;
		}
		usleep(250000);
		it += 1;
	} while (1);

	// cleanup
	XCloseDisplay(dpy);
	return 0;
}