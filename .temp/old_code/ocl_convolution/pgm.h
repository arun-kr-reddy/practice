#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	uint8_t version[3];
	uint32_t height;
	uint32_t width;
	uint32_t max_gray;
	uint8_t *ptr;
} PGM;

void readPGM(const char *file_name, PGM *data)
{
	FILE *pgmFile;
	pgmFile = fopen(file_name, "rb");
	assert(pgmFile != NULL);

	fgets(data->version, sizeof(data->version), pgmFile);
	assert(strcmp(data->version, "P5") == 0);
	fscanf(pgmFile, "%d", &data->width);
	fscanf(pgmFile, "%d", &data->height);
	fscanf(pgmFile, "%d", &data->max_gray);
	fgetc(pgmFile);
	data->ptr = malloc(data->height * data->width);
	fread(data->ptr, data->height * data->width, 1, pgmFile);

	fclose(pgmFile);
}

void writePGM(const char *filename, const PGM *data)
{
	FILE *pgmFile;
	pgmFile = fopen(filename, "wb");
	assert(pgmFile != NULL);

	fprintf(pgmFile, "P5\n");
	fprintf(pgmFile, "%d %d\n", data->width, data->height);
	fprintf(pgmFile, "%d\n", data->max_gray);
	fwrite(data->ptr, data->height * data->width, 1, pgmFile);

	fclose(pgmFile);
}