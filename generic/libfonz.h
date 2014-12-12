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
#define FONZ_REQUEST	0x00
#define FONZ_RESPONSE	0x01

#define FONZ_PRIORITIES		8

#define FONZ_COMMAND_PING	0
#define FONZ_COMMAND_MAGIC	1

#define FONZ_MAGIC_MASTER	0x55
#define FONZ_MAGIC_SLAVE	0xa0

struct	fonz	{
	struct	fonz	*next;
	unsigned char	cmd;
	unsigned char	arg1;
	unsigned char	arg2;
};

void		fp_init(int, int);
struct fonz	*fp_alloc();
void		fp_free();
struct fonz	*fp_receive();
#ifdef AVR
void		fp_send(struct fonz *);
int		fp_sendcmd(unsigned char, unsigned char, unsigned char);
#else
void		fp_send(struct fonz *, int);
int		fp_sendcmd(unsigned char, unsigned char, unsigned char, int);
#endif

void		fp_indata(unsigned char);
void		fp_inbuffer(unsigned char *, int);
int		fp_outbuffer(unsigned char *, int);
