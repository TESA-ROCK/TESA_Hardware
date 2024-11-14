#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>

// Function to get the current timestamp
char* get_timestamp() {
    time_t now = time(NULL);
    char *timestamp = malloc(sizeof(char) * 20);
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    return timestamp;
}

// Function to execute an SQL statement
void execute_sql(sqlite3 *db, const char *sql) {
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Function to initialize the database and create the table
void init_db(sqlite3 **db) {
    int rc = sqlite3_open("Machine_timestamps.db", db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        exit(1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS SoundEvents ("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "Event TEXT, "
                      "Timestamp TEXT);";
    execute_sql(*db, sql);
}

// Function to insert a timestamp into the database
void insert_timestamp(sqlite3 *db, const char *event) {
    char *timestamp = get_timestamp();
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO SoundEvents (Event, Timestamp) VALUES ('%s', '%s');", event, timestamp);
    execute_sql(db, sql);
    free(timestamp);
}

// Simulated function to detect sound appearance and disappearance
void detect_sound(sqlite3 *db) {
    // Simulate sound appearance
    printf("Sound appeared\n");
    insert_timestamp(db, "Start");

    // Simulate some delay
    sleep(5);

    // Simulate sound disappearance
    printf("Sound disappeared\n");
    insert_timestamp(db, "Stop");
}

int main() {
    sqlite3 *db;
    init_db(&db);

    // Simulate sound detection
    detect_sound(db);

    sqlite3_close(db);
    return 0;
}