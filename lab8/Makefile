# Имя исполняемого файла
TARGET = threads

# Компилятор и флаги
CC = gcc
CFLAGS = -pthread -Wall -Wextra -O2

# Исходные файлы
SRC = threads.c

# Правило по умолчанию
all: $(TARGET)

# Правило сборки
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Очистка
clean:
	rm -f $(TARGET)
