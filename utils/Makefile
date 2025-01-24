
# CC CPP AR
# CFLAGS INCLUDES
# come from top of Makefile in Kiva

# objs #
OBJS := src/node_list.o \

TARGET := libutils.a

All: $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
