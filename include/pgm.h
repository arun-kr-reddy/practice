#pragma once

typedef struct {
    uint8_t version[3];
    uint32_t height;
    uint32_t width;
    uint32_t max_gray;
    uint8_t *ptr;
} pgm_t;

void readPGM(const char *file_name, pgm_t *img) {
    FILE *fp;
    fp = fopen(file_name, "rb");
    assert(fp != NULL);

    fgets(img->version, sizeof(img->version), fp);
    assert(strcmp(img->version, "P5") == 0);
    fscanf(fp, "%d", &img->width);
    fscanf(fp, "%d", &img->height);
    fscanf(fp, "%d", &img->max_gray);
    fgetc(fp);
    img->ptr = malloc(img->height * img->width);
    fread(img->ptr, img->height * img->width, 1, fp);
    
    fclose(fp);
}

void writePGM(const char *filename, const pgm_t *img) {
    FILE *fp;
    fp = fopen(filename, "wb");
    assert(fp != NULL);

    fprintf(fp, "P5\n");
    fprintf(fp, "%d %d\n", img->width, img->height);
    fprintf(fp, "%d\n", img->max_gray);
    fwrite(img->ptr, img->height * img->width, 1, fp);

    fclose(fp);
}