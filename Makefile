# Makefile

# Compiler options
CC = gcc
CFLAGS = -Wall -g

# PostgreSQL library and includes
LIBS = -lpq
INCLUDES = -I/usr/include/postgresql

# Source files and executable
SOURCES = sock.c utils.c
EXECUTABLE = server

# Directory containing migration SQL files
MIGRATIONS_DIR = migrations

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
    $(CC) $(CFLAGS) $(INCLUDES) -o $(EXECUTABLE) $(SOURCES) $(LIBS)

migrations:
    @echo "Running migrations..."
    for file in $(MIGRATIONS_DIR)/*.sql; do \
        echo "Executing $$file"; \
        psql -U $(DBUSER) -d $(DBNAME) -h $(DBHOST) -p $(DBPORT) -f $$file; \
    done
    @echo "Migrations completed."

clean:
    rm -f $(EXECUTABLE)

.PHONY: migrations clean

