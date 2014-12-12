#
# Copyright (c) 2007, Kalopa Research.  All rights reserved.  This is free
# software; you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation;
# either version 2, or (at your option) any later version.
#
# It is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this product; see the file COPYING.  If not, write to the Free
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#
# THIS SOFTWARE IS PROVIDED BY KALOPA RESEARCH "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KALOPA RESEARCH BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ABSTRACT
#
DEVICE?=atmega8

AVR_BINDIR?=/usr/local/bin

AR=$(AVR_BINDIR)/avr-ar
AS=$(AVR_BINDIR)/avr-as
CC=$(AVR_BINDIR)/avr-gcc
GCC=$(AVR_BINDIR)/avr-gcc
LD=$(AVR_BINDIR)/avr-ld
NM=$(AVR_BINDIR)/avr-nm

OBJS=	drain.o send.o init.o alloc.o recv.o queue.o sioint.o

LIB=	libfonz.a

ASFLAGS= -mmcu=$(DEVICE)
CFLAGS=	-I../generic -Wall -O2 -mmcu=$(DEVICE) -DAVR

$(LIB): $(OBJS)

all:	$(LIB)

clean:
	rm -f $(LIB) $(OBJS)

$(LIB):	$(OBJS)
	$(AR) cru $(LIB) $?

$(OBJS): libfonz.h

alloc.o: ../generic/alloc.c
	$(CC) $(CFLAGS) -c -o $@ $<

init.o: ../generic/init.c
	$(CC) $(CFLAGS) -c -o $@ $<

queue.o: ../generic/queue.c
	$(CC) $(CFLAGS) -c -o $@ $<

recv.o: ../generic/recv.c
	$(CC) $(CFLAGS) -c -o $@ $<
