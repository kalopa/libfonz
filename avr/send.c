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

/*
 * Send a command.
 */
int
fp_sendcmd(unsigned char cmd, unsigned char arg1, unsigned char arg2)
{
	struct fonz *fp;

	if ((fp = fp_alloc()) == NULL)
		return(0);
	fp->fp_cmd = cmd;
	fp->fp_arg1 = arg1;
	fp->fp_arg2 = arg2;
	fp_send(fp);
	return(1);
}

/*
 *
 */
void
fp_send(struct fonz *fp)
{
	_fp_addtail(fp, &fp_sendq);
	_sio_txinton();
}

/*
 * We need to send an 8-bit byte over the wire. See if we have anything to
 * send...
 */
int
fp_outdata()
{
	unsigned char ch = 0;
	static struct fonz *sfp = NULL;
	static unsigned char state = FONZ_STATE_WAITHDR, sawesc = 0, cksum = 0;

	/*
	 * Highest priority here is to handle escaped bytes. Make sure
	 * we send the original byte (with the high bits stripped off).
	 */
	if (sawesc) {
		ch = sawesc & ~FONZ_MASK;
		sawesc = 0;
		return(ch);
	}
	/*
	 * If we're not dealing with an existing transmission, it's time
	 * to find a packet and send it.
	 */
	if (sfp == NULL) {
		if ((sfp = _fp_remhead(&fp_sendq)) == NULL) {
			/*
			 * Nothing to send.
			 */
			state = FONZ_STATE_WAITHDR;
			sfp = NULL;
			return(-1);
		}
		/*
		 * Ok, looks like we have work to do. Send a header.
		 */
		state = FONZ_STATE_FIRSTBYTE;
		return(FONZ_HEADER);
	}
	/*
	 * At this point, we need to make sure we don't send anything that
	 * looks like 0xc?, in case it's confused as a header or an escape.
	 * We also need to process each of the bytes in the packet.
	 */
	switch (state) {
	case FONZ_STATE_FIRSTBYTE:
		/*
		 * First byte - send the command byte.
		 */
		ch = sfp->fp_cmd;
		cksum = ch ^ 0xff;
		state = (sfp->fp_cmd & FONZ_RESPONSE) ? FONZ_STATE_WAITARG1 : FONZ_STATE_WAITCSUM;
		break;

	case FONZ_STATE_WAITARG1:
		ch = sfp->fp_arg1;
		cksum ^= ch;
		state++;
		break;

	case FONZ_STATE_WAITARG2:
		ch = sfp->fp_arg2;
		cksum ^= ch;
		state++;
		break;

	case FONZ_STATE_WAITCSUM:
		/*
		 * Last byte - send the checksum. Note that we don't actually
		 * compute the checksum here, it is given to us. We're now
		 * back looking for a packet and we mark this one as not-busy.
		 */
		ch = cksum;
		_fp_addtail(sfp, &fp_freeq);
		state = FONZ_STATE_WAITHDR;
		sfp = NULL;
		break;
	}
	if (ch == FONZ_HEADER || ch == FONZ_ESCAPE) {
		/*
		 * We're trying to send an illegal byte. We need to instead
		 * send an escape byte, and save this for the next time.
		 */
		sawesc = ch;
		ch = FONZ_ESCAPE;
	}
	/*
	 * Hand the character back to the transmit-routine.
	 */
	return(ch);
}
