/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1999 Robert James Kaes <rjkaes@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* See 'buffer.c' for detailed information. */

#ifndef _TINYPROXY_BUFFER_H_
#define _TINYPROXY_BUFFER_H_

#define BUFFER_HEAD(x) (x)->head
#define BUFFER_TAIL(x) (x)->tail

struct bufline_s {
        unsigned char *string;  /* the actual string of data */
        struct bufline_s *next; /* pointer to next in linked list */
        size_t length;          /* length of the string of data */
        size_t pos;             /* start sending from this offset */
};

/*
 * The buffer structure points to the beginning and end of the buffer list
 * (and includes the total size)
 */
struct buffer_s {
        struct bufline_s *head; /* top of the buffer */
        struct bufline_s *tail; /* bottom of the buffer */
        size_t size;            /* total size of the buffer */
};


extern struct buffer_s *new_buffer (void);
extern void delete_buffer (struct buffer_s *buffptr);
extern size_t buffer_size (struct buffer_s *buffptr);

/*
 * Add a new line to the given buffer. The data IS copied into the structure.
 */
extern int add_to_buffer (struct buffer_s *buffptr, unsigned char *data,
                          size_t length);

extern ssize_t read_buffer (int fd, struct buffer_s *buffptr);
extern ssize_t write_buffer (int fd, struct buffer_s *buffptr);

#endif /* __BUFFER_H_ */
