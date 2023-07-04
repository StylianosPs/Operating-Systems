# paths
# INCLUDE = include
# MODULES = modules
# PROGRAMS = programs

# compiler
CC = gcc

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = -Wall -Werror -g 
LDFLAGS = -lcrypto

# Αρχεία .o
OBJS1 = P1.c 
OBJS2 = ENC1.c
OBJS3 = CHAN.c
OBJS4 = ENC2.c
OBJS5 = P2.c

# Το εκτελέσιμο πρόγραμμα
EXEC1 = P1
EXEC2 = ENC1
EXEC3 = CHAN
EXEC4 = ENC2
EXEC5 = P2


# Παράμετροι για δοκιμαστική εκτέλεση
ARGS = 

$(EXEC1) $(EXEC2) $(EXEC3) $(EXEC4) $(EXEC5): $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5)
	$(CC) $(OBJS1) -o $(EXEC1) $(LDFLAGS)
	$(CC) $(OBJS2) -o $(EXEC2) $(LDFLAGS)
	$(CC) $(OBJS3) -o $(EXEC3) $(LDFLAGS)
	$(CC) $(OBJS4) -o $(EXEC4) $(LDFLAGS)
	$(CC) $(OBJS5) -o $(EXEC5) $(LDFLAGS)

clean:
	rm -f $(EXEC1) $(EXEC2) $(EXEC3) $(EXEC4) $(EXEC5)


