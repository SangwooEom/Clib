#ifndef CSV_H
#define CSV_H

typedef struct CSV CSV;

CSV* read_csv(const char* csv_path);
void write_csv(CSV* csv, const char* csv_path);

struct CSV {
    char*** data;
    int* numOfFieldsMap;
    int numOfLines;
};

#endif