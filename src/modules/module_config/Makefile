
####################################
# CC CPP AR
# CFLAGS INCLUDES
# come from top of Makefile in Kiva
###################################

#OBJS +=  $(KIVA_ENV_MODULES_DIR)/module_config/module_config.o

OBJS := module_config.o

TARGET := libconfig.a

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^
	cp $(TARGET) ../

All: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)


