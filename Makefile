#CPU			:= atmega328p
CPU				:= attiny24a
FREQ			:= 8000000L
#PARTNO			:= m328p
PARTNO			:= t24
#PROGRAMMER		:= arduino
PROGRAMMER		:= stk500v1
PORT			:= COM16
#BAUDRATE		:= 115200
BAUDRATE		:= 19200
AVRDUDE_CONF	:= 'C:\Program Files (x86)\Arduino\hardware\tools\avr\etc\avrdude.conf'

CC			= avr-gcc -c
CC_OPT		= -mmcu=$(CPU) \
				-DF_CPU=$(FREQ) \
				-Os \
				-Wall \
				-Wextra \
				-fno-exceptions \
				-ffunction-sections \
				-fdata-sections \
				-flto
#CC_DEF		= -DTEST -DTEST_2 -DDEBUG
CC_DEF		= -DDEBUG

LD			= avr-gcc
LD_OPT		= -mmcu=$(CPU) \
				-Os \
				-Wall \
				-Wextra \
				-flto \
				-fuse-linker-plugin \
				-Wl,--gc-sections
			
OBJCOPY		= avr-objcopy
BIN_OPT		= -v \
				-O ihex \
				-R .eeprom
EEP_OPT		= -v \
				-O ihex \
				-j .eeprom \
				--set-section-flags=.eeprom=alloc,load \
				--no-change-warnings \
				--change-section-lma .eeprom=0

AVRDUDE			= avrdude
AVRDUDE_OPT		= -v \
					-C$(AVRDUDE_CONF) \
					-p$(PARTNO) \
					-c$(PROGRAMMER) \
					-P$(PORT) \
					-b$(BAUDRATE)

INCS		= include
LIBS		=
DEPS		=
OBJS		= $(addsuffix .o, $(basename $(wildcard src/*.c)))

BIN			= bin/lih.bin
EEP			= bin/lih.eep
ELF			= bin/lih.elf

upload: all
	$(AVRDUDE) $(AVRDUDE_OPT) -Uflash:w:$(BIN):i -Ueeprom:w:$(EEP):i

all: clean bin eep

clean:
	rm -rf $(BIN) $(EEP) $(ELF) $(OBJS)

bin: elf
	$(OBJCOPY) $(BIN_OPT) $(ELF) $(BIN)
	chmod 755 $(BIN)
	
eep: elf
	$(OBJCOPY) $(EEP_OPT) $(ELF) $(EEP)
	chmod 755 $(EEP)

elf: $(OBJS)
	mkdir -p $(dir $(ELF))
	$(LD) $(LD_OPT) -o $(ELF) $?
	chmod 755 $(ELF)

%.o: %.c
	$(CC) $(CC_OPT) $(CC_DEF) -I $(INCS) -o $@ $<

read_fuse:
	$(AVRDUDE) $(AVRDUDE_OPT) -Ulfuse:r:low.hex:h -Uhfuse:r:high.hex:h -Uefuse:r:extend.hex:h
	
write_fuse:
#	$(AVRDUDE) $(AVRDUDE_OPT) -Ulfuse:w:0xd2:m -Uhfuse:w:0xde:m -Uefuse:w:0xff:m
	$(AVRDUDE) $(AVRDUDE_OPT) -Ulfuse:w:0xe2:m -Uhfuse:w:0xdf:m -Uefuse:w:0xff:m
	
.PHONY: upload all clean bin eep elf read_fuse write_fuse
