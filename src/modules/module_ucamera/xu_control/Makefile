
####################################
# CC CPP AR
# CFLAGS INCLUDES
# come from top of Makefile in Kiva
###################################

OBJS := uvc_xu_control.o

TARGET := libuvc_xu.a

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) $(INCLUDES) -lstdc++ -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^
	cp $(TARGET) ../../

All: $(TARGET)
clean:
	rm -f $(TARGET) $(OBJS)

