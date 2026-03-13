#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>

#define MAX_ROOM_NAME_LENGTH (32)
#define MAX_LINE_LENGTH (128)
#define MAX_ROWS (256)

typedef struct {
    float temperature;
    char room[MAX_ROOM_NAME_LENGTH];
} Measurement;

int process_file(FILE *file, Measurement data[]);
void print_temperature_graph(Measurement data[], int row_count);


int main(void) {
    setlocale(LC_NUMERIC, "C");
    Measurement data[MAX_ROWS];
    // For testing change file to optimal.csv, empty.csv, stress_rows.csv, long_line.csv
    // more long_room.csv, invalid_format.csv, edge_cases.csv or wrong_temp.csv
    const char *filepath = "../data_files/optimal.csv";

    // Attempt to open the file for reading
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("");
        return 1;
    }

    // File processing and store data of valid rows
    int row_count = process_file(file, data);
    fclose(file);

    // Only proceed to graph if data was successfully read
    if (row_count > 0) {
        print_temperature_graph(data, row_count);
    } else if (row_count == 0) {
        fprintf(stderr,"No data rows found to process.\n");
    }
    return 0;
}

// File processing
int process_file(FILE *file, Measurement data[]) {
    char line[MAX_LINE_LENGTH];
    int count = 0;

    // Read the header row and check if file is empty
    if (fgets(line, MAX_LINE_LENGTH, file) == NULL) {
        fprintf(stderr, "Error: File is empty!\n");
        return -1;
    }
    // Process data rows one by one
    while (fgets(line,MAX_LINE_LENGTH, file)) {

        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        // Row limit checking
        if (count >= MAX_ROWS) {
            fprintf(stderr, "Error: Maximum number of %d rows exceeded.\n", MAX_ROWS);
            return -1;
        }

        // Row length checking
        size_t len = strlen(line);
        if (len == MAX_LINE_LENGTH - 1 && line[len - 1] != '\n') {
            fprintf(stderr, "Error: Input line exceeds the maximum length of %d characters.\n", MAX_LINE_LENGTH);
            return -1;
        }

        // Parse CSV values
        char *temp_str = strtok(line, ",");
        char *room_str = strtok(NULL, ",\r\n");

        if (!temp_str || !room_str) {
            fprintf(stderr, "Error: Invalid data format on row %d.\n"
                "Ensure rows are in 'Temperature,Room' format.\n", count + 2);
            return -1;
        }

        // Validate room name length before copying to struct
        if (strlen(room_str) >= MAX_ROOM_NAME_LENGTH) {
            fprintf(stderr, "Error: Room name '%s' is too long.\n", room_str);
            return -1;
        }

        // Parse values
        char *end_er;
        errno = 0;
        double temp_er = strtod(temp_str, &end_er);

        if (end_er == temp_str || *end_er != '\0' || errno == ERANGE) {
            fprintf(stderr, "Error: Invalid temperature reading '%p' on row %d.\n", &temp_er, count + 3);
            return -1;
        }

        data[count].temperature = (float)temp_er;
        strcpy(data[count].room, room_str);

        count++;
    }
    return count;
}

// Generates room-specific temperature graph
void print_temperature_graph(Measurement data[], int row_count) {
    char search_room[MAX_ROOM_NAME_LENGTH];
    bool found = false;

    // Printing list with all temps and room names
    printf("--- All Measurements ---\n");
    for (int i = 0; i < row_count; i++) {
        printf("%.1f,%s\n", data[i].temperature, data[i].room);
    }
    printf("------------------------\n\n");

    while (!found) {
        // User input for room selection
        printf("Select a room: ");
        if (fgets(search_room, MAX_ROOM_NAME_LENGTH, stdin)) {
            search_room[strcspn(search_room, "\r\n")] = 0;
        }

        printf("Graph for: %s\n", search_room);
        for (int i = 0; i < row_count; i++) {
            if (strcasecmp(search_room, data[i].room) == 0) {
                found = 1;
                printf("%5.1f ", data[i].temperature);

                // Print bar graph if within valid range
                if (data[i].temperature >= 0 && data[i].temperature <= 30) {
                    int dashes = (int)(data[i].temperature / 0.5);
                    for (int j = 0; j < dashes; j++) printf("-");
                    printf("\n");
                } else {
                    printf("X\n");
                }
            }
        }
        if (!found) {
            printf("Warning: Room '%s' not found!. Please select room with name in the file!\n", search_room);
        }
    }
}
