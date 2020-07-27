/*
 * binarix - a matrix styled linux console locker
 * Copyright (C) Raffael Himmelreich <raffi@exception.at>
 *
 *  This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General 
 * Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later
 * version.
 *  This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA.
 *
 */

#include <signal.h>
#include <sys/vt.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <fcntl.h>

int vtfh;

void release_vt(int signo) {
	ioctl(vtfh, VT_RELDISP, 0);
}

void acquire_vt(int signo) {
	/* This call is not currently required under Linux, but it won't hurt, *
	 * either...                                                           *
         * Thanks to Michael K. Johnson.                                       */
	ioctl(vtfh, VT_RELDISP, VT_ACKACQ);
}

int setsecurity()
{
	struct vt_mode vtm;
        sigset_t blockmask;
	struct sigaction sa;

        sigfillset(&blockmask);
	sigdelset(&blockmask, SIGUSR1);
	sigdelset(&blockmask, SIGUSR2);
        sigprocmask(SIG_BLOCK, &blockmask, 0);

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = release_vt;
	sigaction(SIGUSR1, &sa, NULL);

	sa.sa_handler = acquire_vt;
	sigaction(SIGUSR2, &sa, NULL);

	vtfh = open("/dev/tty", O_RDWR);

	if((ioctl(vtfh, VT_GETMODE, &vtm)) >= 0)
	{
		vtm.mode   = VT_PROCESS;
		vtm.relsig = SIGUSR1;
		vtm.acqsig = SIGUSR2;
		ioctl(vtfh, VT_SETMODE, &vtm);
	}

	return 0;
}
