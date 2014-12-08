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
#include <unistd.h>
#include <stdlib.h>

#include "libfonz.h"

void	runcmd();

/*
 *
 */
int
main()
{
	int i;
	struct fonz *fp;

	printf("Testing FONZ...\n");
	fp_init(4);
	fp_sendcmd(0x00, 0, 0);
	runcmd(fp);
	fp_sendcmd(0xc1, 0xaa, 0x55);
	runcmd(fp);
	fp_sendcmd(0xce, 0xaa, 0x55);
	runcmd(fp);
	fp_sendcmd(0x31, 0, 0);
	runcmd(fp);
	exit(0);
}

/*
 * 
 */
void
runcmd()
{
	int i, n, ch;
	unsigned char data[10];
	struct fonz *fp;

	printf("RUNCMD:\n");
	for (n = 0; n < 10 && (ch = fp_outdata()) != -1; n++) {
		printf("XMIT:0x%02x\n", ch);
		data[n] = ch;
	}
	printf("XMIT %d bytes\n", n);
	for (i = 0; i < n; i++) {
		fp_indata(data[i]);
		if ((fp = fp_receive()) != NULL) {
			printf("GOT A PACKET!\n");
			printf("cmd:0x%02x,arg1:0x%02x,arg2:0x%02x\n", fp->cmd, fp->arg1, fp->arg2);
			fp_free(fp);
		}
	}
}
