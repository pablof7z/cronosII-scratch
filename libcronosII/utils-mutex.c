/*  Cronos II - A GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández Navarro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* C2Mutex -- implemented by Bosko <falling@users.sourceforge.net> so we have
 * one set of easy-to-use and not-so-POSIXy mutexs for CronosII :-) 
 * 
 * Inspired by FreeBSD's less-than-kind mutex's ;-)
 */

#include <glib.h>
#include <stdio.h>
#include <unistd.h>

#include "utils-mutex.h"

/**
 * c2_mutex_init
 * @mutex: A pointer to the mutex
 * 
 * Initializes a mutex to be a useful, yet not-so-POSIX
 * mutex, break some rules, and still play nicely. 
 * The purpose of this function is mostly to 
 * overcome differences between the default 
 * mutex behaivor on different systems (i.e.
 * freebsd's mutexs vs. linux's vs. others)
 * 
 * Return Value:
 * 0
 **/
gint 
c2_mutex_init (C2Mutex *mutex)
{	
	mutex->lock = 0;
	mutex->queue = NULL;
	
	return 0;
}

/**
 * c2_mutex_lock
 * @mutex: A pointer to the mutex
 * 
 * Locks a mutex. If the mutex is already locked,
 * it will the lock request will be queued and the
 * c2_mutex_lock() will block until the mutex is 
 * unlocked once for each member in the queue.
 * Works pretty much like a POSIX mutex lock.
 * 
 * Return Value:
 * 0 on success
 * -1 on _strange_ errors (system probably on fire)
 **/
gint
c2_mutex_lock (C2Mutex *mutex)
{	
	if(!mutex->lock)
	{
		mutex->lock = 1;
		return 0;
	}
	
	else
	{
		const char on=0;
		gchar c[2] = {0, 0};
		int pipes[2];
		
		if(pipe(pipes) < 0)
		{
			g_warning("Critical Internal Error: Creating Mutex Pipe\n");
			return -1;
		}
		mutex->queue = g_list_prepend(mutex->queue, GINT_TO_POINTER(pipes[1]));
		if(read(pipes[0], c, 5) < 0)
		{
			g_warning("Critical Internal Error: Using Mutex Pipe\n");
			mutex->queue = g_list_remove(mutex->queue, GINT_TO_POINTER(pipes[1]));
			close(pipes[0]);
			close(pipes[1]);
			return -1;
		}
		
		mutex->queue = g_list_remove(mutex->queue, GINT_TO_POINTER(pipes[1]));
		close(pipes[0]);
		close(pipes[1]);
		return 0;
	}
}

/**
 * c2_mutex_unlock
 * @mutex: A pointer to the mutex
 * 
 * Unlocks a mutex. If there are queued lock
 * functions, it lets the next one in queue
 * continue. If the queue is empty, the muetex
 * is simply unlocked. Works just like a
 * POSIX mutex unlock except that any thread
 * can unlock the mutex, even one that had
 * not originally locked it.
 * 
 * Return Value:
 * 0 on success, 
 * -1 on _strange_ errors (system probably on fire)
 **/
gint
c2_mutex_unlock (C2Mutex *mutex)
{
	if(!mutex->lock)
	{
		g_warning("Attempting to unlock an already unlocked mutex\n");
		return -1;
	}
	
	if(!mutex->queue)
	{
		mutex->lock = FALSE;
		return 0;
	}
	else
	{
		gchar *c = "*";
		gint pipe;
		
		pipe = GPOINTER_TO_INT
			(g_list_nth_data(mutex->queue, g_list_length(mutex->queue) - 1));
		
		if(write(pipe, c, 1) < 0)
		{
			g_warning("Critical Internal Error: Writing to Mutex Pipe\n");
			return -1;
		}
		return 0;
	}
}


/**
 * c2_mutex_unlock
 * @mutex: A pointer to the mutex
 * 
 * Attempts to lock a mutex. Fails if 
 * mutex is already locked.
 * 
 * Return Value:
 * 0 on lock success
 * -1 if mutex is already locked
 **/
gint
c2_mutex_trylock (C2Mutex *mutex)
{
	if(mutex->lock)
		return -1;
	
	mutex->lock = 1;
	return 0;
}

/*
 * c2_mutex_destroy
 * @mutex: A pointer to the mutex
 * 
 * Locks a mutex, preparing it to be 
 * destroyed or free()'ed.
 * 
 * Return Value:
 * 0 on success on lock,
 * -1 if mutex has lock requests queud behind the 
 *    final *_destroy() lock request.
 **/
gint
c2_mutex_destroy (C2Mutex *mutex)
{
	c2_mutex_lock(mutex);
	
	if(g_list_length(mutex->queue) != 0)
		return -1;
	else
		return 0;
}
