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

unsigned char	fprecv_havepkt = 0;
unsigned char	fprecv_lostpkt = 0;

/*
 * Get any received packets.
 */
struct fonz *
fp_receive()
{
	return(_fp_remhead(&fp_recvq));
}

/*
 * We have received an 8-bit byte over the wire. Time to do something useful
 * with it. We try to decode a Fonz Packet from the byte sequence.
 */
void
fp_indata(unsigned char ch)
{
	static struct fonz *rfp = NULL;
	static unsigned char state = FONZ_STATE_WAITHDR, sawesc = 0, cksum = 0;

	if (ch == FONZ_HEADER) {
		/*
		 * Highest priority is a header.
		 */
		state = FONZ_STATE_FIRSTBYTE;
		if (rfp != NULL)
			fprecv_lostpkt++;
		return;
	}
	/*
	 * Ignore all traffic until we have seen a header.
	 */
	if (state == FONZ_STATE_WAITHDR)
		return;
	/*
	 * Deal with any escape codes.
	 */
	if (ch == FONZ_ESCAPE) {
		/*
		 * An escape code. Save it, for the next time.
		 */
		sawesc = FONZ_MASK;
		return;
	}
	ch |= sawesc;
	sawesc = 0;
	/*
	 * Find an empty packet to receive the data.
	 */
	if (rfp == NULL && (rfp = _fp_remhead(&fp_freeq)) == NULL) {
		/*
		 * No space - drop the packet.
		 */
		state = FONZ_STATE_WAITHDR;
		fprecv_lostpkt++;
		return;
	}
	/*
	 * Now, deal with the actual data.
	 */
	switch (state) {
	case FONZ_STATE_FIRSTBYTE:
		/*
		 * Store the command, and init the checksum.
		 */
		rfp->fp_cmd = ch;
		cksum = ch ^ 0xff;
		rfp->fp_arg1 = rfp->fp_arg2 = 0;
		state = (rfp->fp_cmd & FONZ_RESPONSE) ? FONZ_STATE_WAITARG1 : FONZ_STATE_WAITCSUM;
		break;

	case FONZ_STATE_WAITARG1:
		/*
		 * One of the optional arguments. Save it, and add it to the
		 * checksum.
		 */
		rfp->fp_arg1 = ch;
		cksum ^= ch;
		state++;
		break;

	case FONZ_STATE_WAITARG2:
		/*
		 * One of the optional arguments. Save it, and add it to the
		 * checksum.
		 */
		rfp->fp_arg2 = ch;
		cksum ^= ch;
		state++;
		break;

	case FONZ_STATE_WAITCSUM:
		/*
		 * Last byte of a packet. That'll be the checksum. Make sure
		 * it is correct. If so, the packet is saved. Otherwise,
		 * wait for the next one.
		 */
		if (ch == cksum)
			_fp_addtail(rfp, &fp_recvq);
		rfp = NULL;
		state = FONZ_STATE_WAITHDR;
		break;
	}
}
