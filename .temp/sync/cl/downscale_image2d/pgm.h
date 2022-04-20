#ifndef __PGM_H
#define __PGM_H

typedef struct
{
    int height;
    int width;
    int max_gray;
    char *ptr;
} PGMData;

void skipcomments(FILE *fp)
{
    int ch;
    char line[100];
    while ((ch = fgetc(fp)) != EOF && isspace(ch))
    {
        ;
    }

    if (ch == '#')
    {
        fgets(line, sizeof(line), fp);
        skipcomments(fp);
    }
    else
    {
        fseek(fp, -1, SEEK_CUR);
    }
}

PGMData *readPGM(const char *file_name, PGMData *data)
{
    FILE *pgmFile;
    char version[3];
    int i, j;
    int lo, hi;
    pgmFile = fopen(file_name, "rb");
    if (pgmFile == NULL)
    {
        perror("cannot open file to read");
        exit(EXIT_FAILURE);
    }
    fgets(version, sizeof(version), pgmFile);
    if (strcmp(version, "P5"))
    {
        fprintf(stderr, "Wrong file type!\n");
        exit(EXIT_FAILURE);
    }
    skipcomments(pgmFile);
    fscanf(pgmFile, "%d", &data->width);
    skipcomments(pgmFile);
    fscanf(pgmFile, "%d", &data->height);
    skipcomments(pgmFile);
    fscanf(pgmFile, "%d", &data->max_gray);
    fgetc(pgmFile);

    data->ptr = malloc(data->height * data->width);
    fread(data->ptr, data->height * data->width, 1, pgmFile);

    fclose(pgmFile);
    return data;
}

void writePGM(const char *filename, const PGMData *data)
{
    FILE *pgmFile;
    int i, j;
    int hi, lo;

    pgmFile = fopen(filename, "wb");
    if (pgmFile == NULL)
    {
        perror("cannot open file to write");
        exit(EXIT_FAILURE);
    }

    fprintf(pgmFile, "P5\n");
    fprintf(pgmFile, "%d %d\n", data->width, data->height);
    fprintf(pgmFile, "%d\n", data->max_gray);

    fwrite(data->ptr, data->height * data->width, 1, pgmFile);

    fclose(pgmFile);
}

#endif