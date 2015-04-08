#
#
#
CFLAGS=	-Wall -O -I../generic

all:	fonz_redis

clean:
	rm -f fonz_redis fonz_redis.o

fonz_redis:	fonz_redis.o
	$(CC) -o fonz_redis fonz_redis.o -L../generic -lfonz -lhiredis -levent
