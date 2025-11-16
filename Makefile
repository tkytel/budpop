TARGET=	budpop
OBJS=	budpop.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

clean:
	$(RM) $(TARGET) $(OBJS)
