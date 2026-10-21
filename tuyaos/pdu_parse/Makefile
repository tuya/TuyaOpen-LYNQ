CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99 -O2

TARGET := pdu_parse

ML_DIR := ../tuyaos_adapter/src/ml
ML_INC := $(ML_DIR)/include
ML_SRC := $(ML_DIR)/src

SRC := src/main.c \
       $(ML_SRC)/ml.c \
       $(ML_SRC)/ml_gsm.c \
       $(ML_SRC)/ml_utf8.c \
       $(ML_SRC)/ml_utf16.c \
       $(ML_SRC)/ml_iso8859_1.c \
       $(ML_SRC)/ml_cp936.c

INCLUDES := -I$(ML_INC) -I$(ML_SRC)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	@echo "=== test 1: GSM PDU -> UTF-8 (default) ==="
	./$(TARGET) "099164000339541902F0240BA18152484803F40000624010816050230A31D98C56BBD970391A"
	@echo ""
	@echo "=== test 2: GSM PDU -> GSM ==="
	./$(TARGET) "099164000339541902F0240BA18152484803F40000624010816050230A31D98C56BBD970391A" gsm
	@echo ""
	@echo "=== test 3: GSM PDU -> UTF-16BE ==="
	./$(TARGET) "099164000339541902F0240BA18152484803F40000624010816050230A31D98C56BBD970391A" utf16be
