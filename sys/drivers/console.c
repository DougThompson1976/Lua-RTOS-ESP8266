/*
 * Whitecat console driver
 *
 * Copyright (C) 2015 - 2016
 * IBEROXARXA SERVICIOS INTEGRALES, S.L. & CSS IBÉRICA, S.L.
 * 
 * Author: Jaume Olivé (jolive@iberoxarxa.com / jolive@whitecatboard.org)
 * 
 * All rights reserved.  
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#if USE_CONSOLE

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/drivers/uart.h>
#include <sys/drivers/console.h>
#include <sys/drivers/gpio.h>

#include <preprocessor.h>

#define CONSOLE_TTY      PREP_TOSTRING(PREP_CONCAT(/dev/tty, CONSOLE_UART))
#define CONSOLE_SWAP_TTY PREP_TOSTRING(PREP_CONCAT(/dev/tty, CONSOLE_SWAP_UART))

void _console_init() {	
	// Open UART's related to the console
    uart_init(CONSOLE_UART, CONSOLE_BR, 0, CONSOLE_BUFFER_LEN);
    uart_init_interrupts(CONSOLE_UART);
	
	//if (CONSOLE_SWAP_UART) {
		uart_init(CONSOLE_SWAP_UART, CONSOLE_BR, 0, CONSOLE_BUFFER_LEN);		
		//}
	
	uart0_default();	
}

void console_default() {
	// Close standard file descriptor
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

    // Open standard file descriptors, using device related to manin console UART unit
    open(CONSOLE_TTY, O_RDONLY); // stdin
    open(CONSOLE_TTY, O_WRONLY); // stdout
    open(CONSOLE_TTY, O_WRONLY); // stderr
}

void console_swap() {
	// Close standard file descriptor
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

    // Open standard file descriptors, using device related to alternate console UART unit
    open(CONSOLE_SWAP_TTY, O_RDONLY); // stdin
    open(CONSOLE_SWAP_TTY, O_WRONLY); // stdout
    open(CONSOLE_SWAP_TTY, O_WRONLY); // stderr				
}

void console_put(const char *c) {
    while (*c) {
        uart_write(CONSOLE_UART, *c++);
    }   
}

void console_clear() {
	printf("\033[2J\033[1;1H");
}

void console_size(int *rows, int *cols) {
    char buf[6];
    char *cbuf;
    char c;
    
    // Save cursor position
    console_put("\033[s");

    // Set cursor out of the screen
    console_put("\033[999;999H");

    // Get cursor position
    console_put("\033[6n");

    // Return to saved cursor position
    console_put("\033[u"); 

    // Skip scape sequence
    while (uart_read(CONSOLE_UART, &c, 100) && (c != '\033')) {
	}

    while (uart_read(CONSOLE_UART, &c, 100) && (c != '[')) {
	}
    
    // Read rows
    c = '\0';
    cbuf = buf;
    while (uart_read(CONSOLE_UART, &c, 100) && (c != ';')) {
	    *cbuf++ = c;
	}
    *cbuf = '\0';
    
    if (*buf != '\0') {
        *rows = atoi(buf);
    }

    // Read cols
    c = '\0';
    cbuf = buf;
    while (uart_read(CONSOLE_UART, &c, 100) && (c != 'R')) {
		*cbuf++ = c;
	}
    *cbuf = '\0';

    if (*buf != '\0') {
        *cols = atoi(buf);
    }
}

void console_gotoxy(int col, int line) {
    printf("\033[%d;%dH", line + 1, col + 1);
}

void console_statusline(const char *text1, const char *text2) {
    int rows = 0;
    int cols = 0;
    
    console_size(&rows, &cols);
    
    console_gotoxy(0, rows);
    printf("\033[1m\033[7m%s%s\033[K\033[0m", text1, text2);
}

void console_clearstatusline() {
    int rows = 0;
    int cols = 0;
    
    console_size(&rows, &cols);
    
    console_gotoxy(0, rows);
    printf("\033[K\033[0m");
}

void console_erase_eol() {
    console_put("\033[K");
}

void console_erase_sol() {
    console_put("\033[1K");
}

void console_erase_l() {
    console_put("\033[2K");
}
#endif