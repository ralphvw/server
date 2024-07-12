# Load .env file variables
include .env
export

# Compiler options
CC = gcc
CFLAGS = -Wall -g

# PostgreSQL library and includes
LIBS = -L/usr/local/opt/postgresql@14/lib/postgresql@14 -lpq
INCLUDES = -I/usr/local/opt/postgresql@14/include/postgresql@14

# Source files and executable
SOURCES = sock.c utils.c handlers.c
EXECUTABLE = server

# Directory containing migration SQL files
MIGRATIONS_DIR = migrations

# Extract database connection parameters from DATABASE_URL
DBUSER := $(shell echo $(DATABASE_URL) | sed -n 's/.*\/\/\(.*\):.*@.*/\1/p')
DBNAME := $(shell echo $(DATABASE_URL) | sed -n 's/.*\/\(.*\)?ssl.*/\1/p')
DBHOST := $(shell echo $(DATABASE_URL) | sed -n 's/.*@\(.*\):.*/\1/p')
DBPORT := $(shell echo $(DATABASE_URL) | sed -n 's/.*:\(.*\)\/.*/\1/p')

# Default target
all: $(EXECUTABLE) migrations

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(EXECUTABLE) $(SOURCES) $(LIBS)

migrations:
	@echo "Running migrations for DATABASE_URL: $(DATABASE_URL)"
	for file in $(MIGRATIONS_DIR)/*.sql; do \
		echo "Executing $$file"; \
		psql $$DATABASE_URL -f $$file; \
		if [ $$? -ne 0 ]; then \
			echo "Failed to execute $$file"; \
			exit 1; \
		fi; \
	done
	@echo "Migrations completed."

clean:
	rm -f $(EXECUTABLE)

.PHONY: migrations clean
