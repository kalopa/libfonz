/*
 * Copyright (c) 2007, Kalopa Research.  All rights reserved.  This is free
 * software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * It is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this product; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * THIS SOFTWARE IS PROVIDED BY KALOPA RESEARCH "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL KALOPA RESEARCH BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include "libfonz.h"

static	int	_fillbyte(int, unsigned char *, int);

/*
 * Send a command.
 */
int
fp_sendcmd(unsigned char cmd, unsigned char arg1, unsigned char arg2, int prio)
{
	struct fonz *fp;

	if ((fp = fp_alloc()) == NULL)
		return(0);
	fp->cmd = cmd;
	fp->arg1 = arg1;
	fp->arg2 = arg2;
	_fp_addtail(fp, &fp_sendq[prio]);
	return(1);
}

/*
 *
 */
void
fp_send(struct fonz *fp, int prio)
{
	_fp_addtail(fp, &fp_sendq[prio]);
}

/*
 * Fill the output buffer with as many packets as we can find.
 * We need to send an 8-bit byte over the wire. See if we have anything to
 * send...
 */
int
fp_outbuffer(unsigned char *bufferp, int blen)
{
	int n, cksum, tmplen, outlen = 0, prio = 0;
	struct fonz *fp = NULL;

	/*
	 * Fill the buffer, with as much data as will fit.
	 */
	while (blen >= 3) {
		/*
		 * Dequeue a packet for delivery, starting with the
		 * highest priority and working down.
		 */
		for (; fp == NULL && prio < FONZ_PRIORITIES; prio++)
			fp = fp_sendq[prio];
		/*
		 * Anything? If not, return with what we've done so far.
		 */
		if (fp == NULL)
			break;
		/*
		 * Ok, fill the packet.
		 */
		*bufferp++ = FONZ_HEADER;
		blen--;
		tmplen = 1;
		if ((n = _fillbyte(fp->cmd, bufferp, blen)) < 0) {
			/*
			 * Wasn't able to squeeze in the byte. Leave
			 * with what we've done already.
			 */
			break;
		}
		cksum = fp->cmd ^ 0xff;
		bufferp += n;
		tmplen += n;
		blen -= n;
		if (fp->cmd & FONZ_RESPONSE) {
			/*
			 * This command has arguments.
			 */
			if ((n = _fillbyte(fp->arg1, bufferp, blen)) < 0) {
				/*
				 * Wasn't able to squeeze in the
				 * byte. Leave with what we've done
				 * already.
				 */
				break;
			}
			cksum ^= fp->arg1;
			bufferp += n;
			blen -= n;
			tmplen += n;
			if ((n = _fillbyte(fp->arg2, bufferp, blen)) < 0) {
				/*
				 * Wasn't able to squeeze in the
				 * byte. Leave with what we've done
				 * already.
				 */
				break;
			}
			cksum ^= fp->arg2;
			bufferp += n;
			blen -= n;
			tmplen += n;
		}
		if ((n = _fillbyte(cksum & 0xff, bufferp, blen)) < 0) {
			/*
			 * No good. Return with what we have.
			 */
			break;
		}
		/*
		 * OK, we had enough space for the packet. Move outlen
		 * up a bit, and remove the packet from the queue.
		 */
		outlen += tmplen;
		fp_sendq[prio - 1] = fp->next;
		fp = fp->next;
	}
	return(outlen);
}

/*
 * Attempt to fill a byte into the buffer, escaping it where necessary.
 */
static int
_fillbyte(int byte, unsigned char *bufferp, int blen)
{
	if (byte == FONZ_HEADER || byte == FONZ_ESCAPE) {
		if (blen >= 2) {
			*bufferp++ = FONZ_ESCAPE;
			*bufferp++ = byte & 0x7f;
			return(2);
		}
	} else if (blen > 0) {
		*bufferp++ = byte;
		return(1);
	}
	return(-1);
}
