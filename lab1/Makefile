# Переменные для компилятора
CC = gcc
CFLAGS = -Wall -Wextra

# Название программы
TARGET = myls

# Цель по умолчанию
all: $(TARGET)

# Правило сборки
$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o

# Правило компиляции object-файлов
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# Очистка сгенерированных файлов
clean:
	rm -f *.o $(TARGET)

# Установка цели как .PHONY для предупреждения проблем с файлами
.PHONY: clean
