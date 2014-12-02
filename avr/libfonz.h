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
#define FONZ_HEADER	0xce
#define FONZ_ESCAPE	0xcc
#define FONZ_MASK	0xc0

#define FONZ_REQUEST	0x00
#define FONZ_RESPONSE	0x01

#define FONZ_STATE_WAITHDR	0
#define FONZ_STATE_FIRSTBYTE	1
#define FONZ_STATE_WAITARG1	2
#define FONZ_STATE_WAITARG2	3
#define FONZ_STATE_WAITCSUM	4

struct	fonz	{
	struct	fonz	*fp_next;
	unsigned char	fp_cmd;
	unsigned char	fp_arg1;
	unsigned char	fp_arg2;
};

struct	fonz	*fp_freerxq;
struct	fonz	*fp_freetxq;
struct	fonz	*fp_recvq;
struct	fonz	*fp_sendq;

void		fp_init(int, int);
struct fonz	*fp_alloc();
void		fp_free();
void		fp_send(struct fonz *);
struct fonz	*fp_receive();
int		fp_sendcmd(unsigned char, unsigned char, unsigned char);

void		fp_indata(unsigned char);
int		fp_outdata();
void		_fp_addtail(struct fonz *, struct fonz **);
struct	fonz	*_fp_remhead(struct fonz **);

void		_sio_txinton();
void		_sio_txintoff();
