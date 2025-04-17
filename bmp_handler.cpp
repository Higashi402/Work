#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include <omp.h>
#include <vector>
#include <cstring>

#include "bmp_handler.h"


#define BITS_PER_BYTE 8

#define BLUE 0
#define GREEN 1
#define RED 2
#define ALPHA 3

#define SIZE_BYTES 4
#define SIZE_OFFSET 2

#define PIXEL_ARRAY_START_BYTES 4
#define PIXEL_ARRAY_START_OFFSET 10

#define WIDTH_BYTES 4
#define WIDTH_OFFSET 18

#define HEIGHT_BYTES 4
#define HEIGHT_OFFSET 22

#define DEPTH_BYTES 2
#define DEPTH_OFFSET 28

#define COLORS_BYTES 2
#define COLORS_OFFSET 50

#define PALLETE_BYTES 1024
#define PALLETE_OFFSET 54

BMP* bopen(char* file_path)
{
    FILE* fp = fopen(file_path, "rb");

    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    BMP* bmp = (BMP*)malloc(sizeof(BMP));
    bmp->file_byte_number = _get_file_byte_number(fp);
    bmp->file_byte_contents = _get_file_byte_contents(fp, bmp->file_byte_number);
    fclose(fp);

    if (!_validate_file_type(bmp->file_byte_contents))
    {
        _throw_error((char*)"Invalid file type");
    }

    bmp->pixel_array_start = _get_pixel_array_start(bmp->file_byte_contents);
    bmp->width = _get_width(bmp->file_byte_contents);
    bmp->height = _get_height(bmp->file_byte_contents);
    bmp->depth = _get_depth(bmp->file_byte_contents);

    _populate_pixel_array(bmp);

    return bmp;
}

void _update_file_byte_contents(BMP* bmp, int index, int offset, int channel)
{
    char value;
    switch (channel)
    {
    case BLUE:
        value = bmp->pixels[index].blue;
        break;
    case GREEN:
        value = bmp->pixels[index].green;
        break;
    case RED:
        value = bmp->pixels[index].red;
        break;
    case ALPHA:
        value = bmp->pixels[index].alpha;
        break;
    }
    bmp->file_byte_contents[offset + channel] = value;
}

void bwrite(BMP* bmp, char* file_name)
{
    _map(bmp, _update_file_byte_contents);

    FILE* fp = fopen(file_name, "wb");
    fwrite(bmp->file_byte_contents, sizeof(char), bmp->file_byte_number, fp);
    fclose(fp);
}

void _throw_error(char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int get_width(BMP* bmp)
{
    return bmp->width;
}

int get_height(BMP* bmp)
{
    return bmp->height;
}

unsigned int get_depth(BMP* bmp)
{
    return bmp->depth;
}

void get_pixel_rgb(BMP* bmp, int x, int y, int* int_r, int* int_g, int* int_b)
{
    int index = y * bmp->width + x;
    *int_r = bmp->pixels[index].red;
    *int_g = bmp->pixels[index].green;
    *int_b = bmp->pixels[index].blue;
}

void get_pixel_r(BMP* bmp, int x, int y, int* int_r) {
    int index = y * bmp->width + x;
    *int_r = bmp->pixels[index].red;
}

void get_pixel_g(BMP* bmp, int x, int y, int* int_g) {
    int index = y * bmp->width + x;
    *int_g = bmp->pixels[index].green;
}

void get_pixel_b(BMP* bmp, int x, int y, int* int_b) {
    int index = y * bmp->width + x;
    *int_b = bmp->pixels[index].blue;
}

void set_pixel_rgb(BMP* bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
    int index = y * bmp->width + x;
    bmp->pixels[index].red = r;
    bmp->pixels[index].green = g;
    bmp->pixels[index].blue = b;
}

void bclose(BMP* bmp)
{
    free(bmp->pixels);
    bmp->pixels = NULL;
    //free(bmp->file_byte_contents);
    bmp->file_byte_contents = NULL;
    free(bmp);
    bmp = NULL;
}

unsigned int _get_int_from_buffer(unsigned int bytes,
                                  unsigned int offset,
                                  unsigned char* buffer)
{
    unsigned int value = 0;
    int i;

    for (i = bytes - 1; i >= 0; --i)
    {
        value <<= 8;
        value += buffer[i + offset];
    }

    return value;
}

void _write_int_to_buffer(unsigned int value,
                          unsigned int bytes,
                          unsigned int offset,
                          unsigned char* buffer)
{
    for (int i = 0; i < bytes; ++i)
    {
        buffer[i + offset] = (value >> (8 * i)) & 0xFF;
    }
}

unsigned int _get_file_byte_number(FILE* fp)
{
    unsigned int byte_number;
    fseek(fp, 0, SEEK_END);
    byte_number = ftell(fp);
    rewind(fp);
    return byte_number;
}

unsigned char* _get_file_byte_contents(FILE* fp, unsigned int file_byte_number)
{
    unsigned char* buffer = (unsigned char*)malloc(file_byte_number * sizeof(char));
    unsigned int result = fread(buffer, 1, file_byte_number, fp);

    const char* message = "There was a problem reading the file";
    if (result != file_byte_number)
    {
        _throw_error(const_cast<char*>(message));
    }
    return buffer;
}

int _validate_file_type(unsigned char* file_byte_contents)
{
    return file_byte_contents[0] == 'B' && file_byte_contents[1] == 'M';
}


unsigned int _get_pixel_array_start(unsigned char* file_byte_contents)
{
    return _get_int_from_buffer(PIXEL_ARRAY_START_BYTES, PIXEL_ARRAY_START_OFFSET, file_byte_contents);
}

int _get_width(unsigned char* file_byte_contents)
{
    return (int)_get_int_from_buffer(WIDTH_BYTES, WIDTH_OFFSET, file_byte_contents);
}

int _get_height(unsigned char* file_byte_contents)
{
    return (int)_get_int_from_buffer(HEIGHT_BYTES, HEIGHT_OFFSET, file_byte_contents);
}

unsigned int _get_depth(unsigned char* file_byte_contents)
{
    return _get_int_from_buffer(DEPTH_BYTES, DEPTH_OFFSET, file_byte_contents);
}

void _populate_pixel_array(BMP* bmp)
{
    bmp->pixels = (pixel*)malloc(bmp->width * bmp->height * sizeof(pixel));
    if (bmp->depth == 8) {
        _map(bmp, _get_pixel_one_channel);
    }
    else {
        _map(bmp, _get_pixel);
    }
}

void _map(BMP* bmp, void (*f)(BMP*, int, int, int))
{
    int channels = bmp->depth / (sizeof(unsigned char) * BITS_PER_BYTE);
    int row_size = ((int)(bmp->depth * bmp->width + 31) / 32) * 4;
    int padding = row_size - bmp->width * channels;

    int c;
    unsigned int x, index, offset;
    int y;

#pragma omp parallel for collapse(2) private(c, x, y, index, offset) shared(bmp, f)
    for (y = 0; y < bmp->height; y++)
    {
        for (x = 0; x < bmp->width; x++)
        {
            index = y * bmp->width + x;
            offset = bmp->pixel_array_start + index * channels + y * padding;
            for (c = 0; c < channels; c++)
            {
                (*f)(bmp, index, offset, c);
            }
        }
    }
}

void _get_pixel(BMP* bmp, int index, int offset, int channel)
{
    unsigned char value = _get_int_from_buffer(sizeof(unsigned char), offset + channel, bmp->file_byte_contents);
    switch (channel)
    {
    case BLUE:
        bmp->pixels[index].blue = value;
        break;
    case GREEN:
        bmp->pixels[index].green = value;
        break;
    case RED:
        bmp->pixels[index].red = value;
        break;
    case ALPHA:
        bmp->pixels[index].alpha = value;
        break;
    }
}

void _get_pixel_one_channel(BMP* bmp, int index, int offset, int channel)
{
    unsigned char value = _get_int_from_buffer(sizeof(unsigned char), offset + channel, bmp->file_byte_contents);
    bmp->pixels[index].blue = value;
    bmp->pixels[index].green = value;
    bmp->pixels[index].red = value;
    bmp->pixels[index].alpha = value;

}

void get_MO(BMP* bmp, char channel) {
    int height = get_height(bmp);
    int width = get_width(bmp);
    int pixelsAmount = height * width;
    int current;
    int total = 0;

    if (channel == 'r') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_r(bmp, x, y, &current);
                total += current;
            }
        }
        total /= pixelsAmount;
        bmp->MO_r = total;
    }
    else if (channel == 'g') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_g(bmp, x, y, &current);
                total += current;
            }
        }
        total /= pixelsAmount;
        bmp->MO_g = total;
    }
    else if (channel == 'b') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_b(bmp, x, y, &current);
                total += current;
            }
        }
        total /= pixelsAmount;
        bmp->MO_b = total;
    }
}

void get_SKO(BMP* bmp, char channel) {
    int height = get_height(bmp);
    int width = get_width(bmp);
    int pixelsAmount = height * width;
    int current;
    float total = 0;
    if (channel == 'r') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_r(bmp, x, y, &current);
                total += pow((current - bmp->MO_r), 2);
            }
        }
        total /= pixelsAmount;
        total = sqrt(total);
        bmp->SKO_r = total;
    }
    else if (channel == 'g') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_g(bmp, x, y, &current);
                total += pow((current - bmp->MO_g), 2);
            }
        }
        total /= pixelsAmount;
        total = sqrt(total);
        bmp->SKO_g = total;
    }
    else if (channel == 'b') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_b(bmp, x, y, &current);
                total += pow((current - bmp->MO_b), 2);
            }
        }
        total /= pixelsAmount;
        total = sqrt(total);
        bmp->SKO_b = total;
    }
}

void get_SKO_modifyed(BMP* bmp, char channel) {
    int height = get_height(bmp);
    int width = get_width(bmp);
    int pixelsAmount = height * width;
    int current;
    float MO1 = 0;
    float MO2 = 0;
    if (channel == 'r') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_r(bmp, x, y, &current);
                MO1 += pow(current, 2);
                MO2 += current;
            }
        }
        MO1 /= pixelsAmount;
        MO2 /= pixelsAmount;
        bmp->SKO_r = sqrt(MO1 - pow(MO2, 2));
    }
    else if (channel == 'g') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_g(bmp, x, y, &current);
                MO1 += pow(current, 2);
                MO2 += current;
            }
        }
        MO1 /= pixelsAmount;
        MO2 /= pixelsAmount;
        bmp->SKO_g = sqrt(MO1 - pow(MO2, 2));
    }
    else if (channel == 'b') {
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                get_pixel_b(bmp, x, y, &current);
                MO1 += pow(current, 2);
                MO2 += current;
            }
        }
        MO1 /= pixelsAmount;
        MO2 /= pixelsAmount;
        bmp->SKO_b = sqrt(MO1 - pow(MO2, 2));
    }
}

BMP* b_deep_copy(BMP* to_copy)
{
    BMP* copy = (BMP*)malloc(sizeof(BMP));
    copy->file_byte_number = to_copy->file_byte_number;
    copy->pixel_array_start = to_copy->pixel_array_start;
    copy->width = to_copy->width;
    copy->height = to_copy->height;
    copy->depth = to_copy->depth;

    copy->file_byte_contents = (unsigned char*)malloc(copy->file_byte_number * sizeof(unsigned char));

    unsigned int i;
    for (i = 0; i < copy->file_byte_number; i++)
    {
        copy->file_byte_contents[i] = to_copy->file_byte_contents[i];
    }

    copy->pixels = (pixel*)malloc(copy->width * copy->height * sizeof(pixel));

    unsigned int x, y;
    int index;
    for (y = 0; y < copy->height; y++)
    {
        for (x = 0; x < copy->width; x++)
        {
            index = y * copy->width + x;
            copy->pixels[index].red = to_copy->pixels[index].red;
            copy->pixels[index].green = to_copy->pixels[index].green;
            copy->pixels[index].blue = to_copy->pixels[index].blue;
            //copy->pixels[index].alpha = to_copy->pixels[index].alpha;
        }
    }

    return to_copy;
}

void downscale_bmp(BMP* bmp, char* file_path, int n, char* output_file, int method) {
    BMP* downscaled_bmp = (BMP*)malloc(sizeof(BMP));
    downscaled_bmp->pixel_array_start = bmp->pixel_array_start;
    downscaled_bmp->width = bmp->width / n;
    downscaled_bmp->height = bmp->height / n;
    downscaled_bmp->depth = bmp->depth;
    int deleted_bytes = bmp->file_byte_number - downscaled_bmp->width * downscaled_bmp->height * (downscaled_bmp->depth) / 8 - 10000;
    downscaled_bmp->file_byte_number = bmp->file_byte_number - deleted_bytes;
    downscaled_bmp->file_byte_contents = (unsigned char*)malloc(downscaled_bmp->file_byte_number * sizeof(unsigned char));
    _write_int_to_buffer(downscaled_bmp->file_byte_number, SIZE_BYTES, SIZE_OFFSET, downscaled_bmp->file_byte_contents);
    _write_int_to_buffer(downscaled_bmp->pixel_array_start, PIXEL_ARRAY_START_BYTES, PIXEL_ARRAY_START_OFFSET, downscaled_bmp->file_byte_contents);

    unsigned int i;
    for (i = 0; i < downscaled_bmp->file_byte_number; i++)
    {
        downscaled_bmp->file_byte_contents[i] = bmp->file_byte_contents[i];
    }

    _write_int_to_buffer(downscaled_bmp->width, WIDTH_BYTES, WIDTH_OFFSET, downscaled_bmp->file_byte_contents);
    _write_int_to_buffer(downscaled_bmp->height, HEIGHT_BYTES, HEIGHT_OFFSET, downscaled_bmp->file_byte_contents);

    downscaled_bmp->pixels = (pixel*)malloc(downscaled_bmp->width * downscaled_bmp->height * sizeof(pixel));

    unsigned int x, y;
    int index;

    if (method == 1) {
        for (y = 0; y < downscaled_bmp->height; y++)
        {
            for (x = 0; x < downscaled_bmp->width; x++)
            {
                int original_x = x * n + n / 2;
                int original_y = y * n + n / 2;

                original_x = original_x < bmp->width ? original_x : bmp->width - 1;
                original_y = original_y < bmp->height ? original_y : bmp->height - 1;

                int original_index = original_y * bmp->width + original_x;

                index = y * downscaled_bmp->width + x;
                downscaled_bmp->pixels[index].red = bmp->pixels[original_index].red;
                downscaled_bmp->pixels[index].green = bmp->pixels[original_index].green;
                downscaled_bmp->pixels[index].blue = bmp->pixels[original_index].blue;
            }
        }
    }
    else {
        for (y = 0; y < downscaled_bmp->height; y++)
        {
            for (x = 0; x < downscaled_bmp->width; x++)
            {
                int tile_start_x = x * n;
                int tile_start_y = y * n;

                int tile_end_x = tile_start_x + n;
                int tile_end_y = tile_start_y + n;

                tile_end_x = tile_end_x < bmp->width ? tile_end_x : bmp->width;
                tile_end_y = tile_end_y < bmp->height ? tile_end_y : bmp->height;

                int sum_red = 0;
                int sum_green = 0;
                int sum_blue = 0;

                int pixel_count = 0;

                for (int i = tile_start_y; i < tile_end_y; i++)
                {
                    for (int j = tile_start_x; j < tile_end_x; j++)
                    {
                        int original_index = i * bmp->width + j;

                        sum_red += bmp->pixels[original_index].red;
                        sum_green += bmp->pixels[original_index].green;
                        sum_blue += bmp->pixels[original_index].blue;

                        pixel_count++;
                    }
                }

                int average_red = sum_red / pixel_count;
                int average_green = sum_green / pixel_count;
                int average_blue = sum_blue / pixel_count;

                index = y * downscaled_bmp->width + x;
                downscaled_bmp->pixels[index].red = average_red;
                downscaled_bmp->pixels[index].green = average_green;
                downscaled_bmp->pixels[index].blue = average_blue;
            }
        }
    }

    bwrite(downscaled_bmp, output_file);

    bclose(downscaled_bmp);
}

void scale_bmp_nearest_neighbor(BMP* bmp, BMP* scaled_bmp, int new_width, int new_height, float inverse_matrix[2][2]) {
    for (int i = 0; i < new_height; ++i) {
        for (int j = 0; j < new_width; ++j) {
            float src_x = inverse_matrix[0][0] * j + inverse_matrix[0][1] * i;
            float src_y = inverse_matrix[1][0] * j + inverse_matrix[1][1] * i;

            int px = static_cast<int>(floor(src_x));
            int py = static_cast<int>(floor(src_y));

            if (px >= 0 && px < bmp->width && py >= 0 && py < bmp->height) {
                scaled_bmp->pixels[i * new_width + j] = bmp->pixels[py * bmp->width + px];
            } else {
                scaled_bmp->pixels[i * new_width + j] = {0, 0, 0, 0};
            }
        }
    }
}

void scale_bmp_bilinear(BMP* bmp, BMP* scaled_bmp, int new_width, int new_height, float inverse_matrix[2][2]) {
    for (int i = 0; i < new_height; ++i) {
        for (int j = 0; j < new_width; ++j) {
            float src_x = inverse_matrix[0][0] * j + inverse_matrix[0][1] * i;
            float src_y = inverse_matrix[1][0] * j + inverse_matrix[1][1] * i;

            int x = static_cast<int>(src_x);
            int y = static_cast<int>(src_y);
            float x_diff = src_x - x;
            float y_diff = src_y - y;

            if (x >= 0 && x < bmp->width - 1 && y >= 0 && y < bmp->height - 1) {
                pixel a = bmp->pixels[y * bmp->width + x];
                pixel b = bmp->pixels[y * bmp->width + (x + 1)];
                pixel c = bmp->pixels[(y + 1) * bmp->width + x];
                pixel d = bmp->pixels[(y + 1) * bmp->width + (x + 1)];

                pixel p;
                p.red = a.red * (1 - x_diff) * (1 - y_diff) + b.red * x_diff * (1 - y_diff) +
                        c.red * y_diff * (1 - x_diff) + d.red * x_diff * y_diff;
                p.green = a.green * (1 - x_diff) * (1 - y_diff) + b.green * x_diff * (1 - y_diff) +
                          c.green * y_diff * (1 - x_diff) + d.green * x_diff * y_diff;
                p.blue = a.blue * (1 - x_diff) * (1 - y_diff) + b.blue * x_diff * (1 - y_diff) +
                         c.blue * y_diff * (1 - x_diff) + d.blue * x_diff * y_diff;

                scaled_bmp->pixels[i * new_width + j] = p;
            } else {
                scaled_bmp->pixels[i * new_width + j] = {0, 0, 0, 0};
            }
        }
    }
}

void initialize_bmp(BMP* bmp,BMP* initial_bmp, int width, int height, int depth) {
    bmp->width = width;
    bmp->height = height;
    bmp->depth = depth;
    bmp->pixel_array_start = _get_pixel_array_start(initial_bmp->file_byte_contents);
    int row_size = ((depth * width + 31) / 32) * 4;
    int pixel_array_size = row_size * height;
    bmp->file_byte_number = bmp->pixel_array_start + pixel_array_size;
    bmp->file_byte_contents = (unsigned char*)malloc(bmp->file_byte_number * sizeof(unsigned char));
    bmp->pixels = (pixel*)malloc(width * height * sizeof(pixel));

    for (int i = 0; i <= 1078; i++)
    {
        bmp->file_byte_contents[i] = initial_bmp->file_byte_contents[i];
    }
    _write_int_to_buffer(bmp->file_byte_number, 4, 2, bmp->file_byte_contents);
    _write_int_to_buffer(width, 4, 18, bmp->file_byte_contents);
    _write_int_to_buffer(height, 4, 22, bmp->file_byte_contents);
    _write_int_to_buffer(depth, 2, 28, bmp->file_byte_contents);
}

void print_matrix(float matrix[2][2]) {
    for (int i = 0; i < 2; ++i) {
        std::cout << "|";
        for (int j = 0; j < 2; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "|";
        std::cout << std::endl;
    }
}

void create_inverse_scaling_matrix(float Sx, float Sy, float inverse_matrix[2][2]) {
    inverse_matrix[0][0] = 1 / Sx;
    inverse_matrix[0][1] = 0;
    inverse_matrix[1][0] = 0;
    inverse_matrix[1][1] = 1 / Sy;
}

/*void create_inverse_rotating_matrix(float angle, float inverse_matrix[2][2], int width, int height) {
    float cx = width / 2.0;
    float cy = height / 2.0;

    float rad = M_PI / 180;

    float rotate[2][2] = {
        {cos(angle * rad), -sin(angle * rad)},
        {sin(angle * rad), cos(angle * rad)}
    };

    inverse_matrix[0][0] = rotate[0][0];
    inverse_matrix[0][1] = rotate[0][1];
    inverse_matrix[1][0] = rotate[1][0];
    inverse_matrix[1][1] = rotate[1][1];
}


void rotate_bmp_nearest_neighbor(BMP* bmp, BMP* rotated_bmp, float inverse_matrix[2][2]) {
    float cx = bmp->width / 2.0;
    float cy = bmp->height / 2.0;

    for (int i = 0; i < bmp->height; ++i) {
        for (int j = 0; j < bmp->width; ++j) {
            float x = j - cx;
            float y = i - cy;

            float src_x = inverse_matrix[0][0] * x + inverse_matrix[0][1] * y + cx;
            float src_y = inverse_matrix[1][0] * x + inverse_matrix[1][1] * y + cy;

            int px = static_cast<int>(floor(src_x));
            int py = static_cast<int>(floor(src_y));

            if (px >= 0 && px < bmp->width && py >= 0 && py < bmp->height) {
                rotated_bmp->pixels[i * bmp->width + j] = bmp->pixels[py * bmp->width + px];
            }
            else {
                rotated_bmp->pixels[i * bmp->width + j] = { 0, 0, 0, 0 };
            }
        }
    }
}

void rotate_bmp_bilinear(BMP* bmp, BMP* rotated_bmp, float inverse_matrix[2][2]) {
    float cx = bmp->width / 2.0;
    float cy = bmp->height / 2.0;

    for (int i = 0; i < bmp->height; ++i) {
        for (int j = 0; j < bmp->width; ++j) {
            float x = j - cx;
            float y = i - cy;

            float src_x = inverse_matrix[0][0] * x + inverse_matrix[0][1] * y + cx;
            float src_y = inverse_matrix[1][0] * x + inverse_matrix[1][1] * y + cy;

            int px = static_cast<int>(src_x);
            int py = static_cast<int>(src_y);
            float x_diff = src_x - px;
            float y_diff = src_y - py;

            if (px >= 0 && px < bmp->width - 1 && py >= 0 && py < bmp->height - 1) {
                pixel a = bmp->pixels[py * bmp->width + px];
                pixel b = bmp->pixels[py * bmp->width + (px + 1)];
                pixel c = bmp->pixels[(py + 1) * bmp->width + px];
                pixel d = bmp->pixels[(py + 1) * bmp->width + (px + 1)];

                pixel p;
                p.red = a.red * (1 - x_diff) * (1 - y_diff) + b.red * x_diff * (1 - y_diff) +
                    c.red * y_diff * (1 - x_diff) + d.red * x_diff * y_diff;
                p.green = a.green * (1 - x_diff) * (1 - y_diff) + b.green * x_diff * (1 - y_diff) +
                    c.green * y_diff * (1 - x_diff) + d.green * x_diff * y_diff;
                p.blue = a.blue * (1 - x_diff) * (1 - y_diff) + b.blue * x_diff * (1 - y_diff) +
                    c.blue * y_diff * (1 - x_diff) + d.blue * x_diff * y_diff;

                rotated_bmp->pixels[i * bmp->width + j] = p;
            }
            else {
                rotated_bmp->pixels[i * bmp->width + j] = { 0, 0, 0, 0 };
            }
        }
    }
}*/

void find_min_max_brightness(BMP* bmp, int* min_brightness, int* max_brightness) {
    *min_brightness = 255;
    *max_brightness = 0;

    for (int y = 0; y < bmp->height; ++y) {
        for (int x = 0; x < bmp->width; ++x) {
            pixel p = bmp->pixels[y * bmp->width + x];
            int brightness = (p.red + p.green + p.blue) / 3;
            if (brightness < *min_brightness) {
                *min_brightness = brightness;
            }
            if (brightness > *max_brightness) {
                *max_brightness = brightness;
            }
        }
    }
}

void apply_sobel_filter(BMP* bmp, PaddingType padding_type, int threshold) {
    BMP* edged_bmp = (BMP*)malloc(sizeof(BMP));
    initialize_bmp(edged_bmp, bmp, bmp->width, bmp->height, bmp->depth);

    int k = 3;  //Размер окна оператора Собеля
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);

    int new_width = padded_bmp->width;
    int new_height = padded_bmp->height;

    //Ядра Собела для горизонтального и вертикального направлений
    int sobel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int sobel_y[3][3] = {
        {-1, -2, -1},
        {0,  0,  0},
        {1,  2,  1}
    };

#pragma omp parallel for collapse(2)
    for (int y = pad; y < bmp->height + pad; ++y) {
        for (int x = pad; x < bmp->width + pad; ++x) {
            int gx = 0, gy = 0;

            //Применение ядра Собела
            for (int ky = -pad; ky <= pad; ++ky) {
                for (int kx = -pad; kx <= pad; ++kx) {
                    pixel p = padded_bmp->pixels[(y + ky) * new_width + (x + kx)];
                    int intensity = (p.red + p.green + p.blue) / 3;
                    gx += intensity * sobel_x[ky + pad][kx + pad];
                    gy += intensity * sobel_y[ky + pad][kx + pad];
                }
            }

            //Вычисление градиента
            int mag = sqrt(gx * gx + gy * gy);

            //Порог
            if (mag < threshold) {
                mag = 0;
            }
            else {
                mag = 255;
            }

            edged_bmp->pixels[(y - pad) * bmp->width + (x - pad)] = { (unsigned char)mag, (unsigned char)mag, (unsigned char)mag, 255 };
        }
    }

    char output_file[256];
    strcpy(output_file, "C:\\Users\\Higashi\\Desktop\\Фотон\\Images\\edged.bmp");
    bwrite(edged_bmp, output_file);
    bclose(edged_bmp);

    bclose(padded_bmp);
}

void apply_contrast(BMP* bmp, float alpha) {
    BMP* contrasted_bmp = (BMP*)malloc(sizeof(BMP));
    initialize_bmp(contrasted_bmp, bmp, bmp->width, bmp->height, bmp->depth);

    for (int y = 0; y < bmp->height; ++y) {
        for (int x = 0; x < bmp->width; ++x) {
            pixel p = bmp->pixels[y * bmp->width + x];

            int r = (int)(alpha * (p.red - 128) + 128);
            int g = (int)(alpha * (p.green - 128) + 128);
            int b = (int)(alpha * (p.blue - 128) + 128);

            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            contrasted_bmp->pixels[y * bmp->width + x] = { (unsigned char)r, (unsigned char)g, (unsigned char)b, p.alpha };
        }
    }

    char output_file[256];
    strcpy(output_file, "C:\\Users\\Higashi\\Desktop\\Фотон\\Images\\contrasted.bmp");
    bwrite(contrasted_bmp, output_file);
    bclose(contrasted_bmp);
}

void calculate_new_dimensions(int width, int height, float angle, int& new_width, int& new_height) {
    float rad_angle = angle * M_PI / 180.0;
    float abs_cos = std::abs(std::cos(rad_angle));
    float abs_sin = std::abs(std::sin(rad_angle));

    new_width = static_cast<int>(width * abs_cos + height * abs_sin);
    new_height = static_cast<int>(width * abs_sin + height * abs_cos);
}

void create_combined_matrix(float angle, float inverse_matrix[2][2], int width, int height) {
    float cx = width / 2.0;
    float cy = height / 2.0;

    float rad_angle = angle * M_PI / 180.0;
    float cos_a = std::cos(rad_angle);
    float sin_a = std::sin(rad_angle);

    inverse_matrix[0][0] = cos_a;
    inverse_matrix[0][1] = sin_a;
    inverse_matrix[1][0] = -sin_a;
    inverse_matrix[1][1] = cos_a;
}

void rotate_bmp_nearest_neighbor_full(BMP* bmp, BMP* rotated_bmp, float inverse_matrix[2][2], int new_width, int new_height, int threads) {
    float cx = bmp->width / 2.0;
    float cy = bmp->height / 2.0;
    float ncx = new_width / 2.0;
    float ncy = new_height / 2.0;

#pragma omp parallel for num_threads(threads) collapse(2)
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            float nx = x - ncx;
            float ny = y - ncy;

            float src_x = inverse_matrix[0][0] * nx + inverse_matrix[0][1] * ny + cx;
            float src_y = inverse_matrix[1][0] * nx + inverse_matrix[1][1] * ny + cy;

            int px = static_cast<int>(floor(src_x));
            int py = static_cast<int>(floor(src_y));

            if (px >= 0 && px < bmp->width && py >= 0 && py < bmp->height) {
                rotated_bmp->pixels[y * new_width + x] = bmp->pixels[py * bmp->width + px];
            }
            else {
                rotated_bmp->pixels[y * new_width + x] = { 0, 0, 0, 0 };
            }
        }
    }
}

void rotate_bmp_bilinear_full(BMP* bmp, BMP* rotated_bmp, float inverse_matrix[2][2], int new_width, int new_height, int threads) {
    float cx = bmp->width / 2.0;
    float cy = bmp->height / 2.0;
    float ncx = new_width / 2.0;
    float ncy = new_height / 2.0;

#pragma omp parallel for num_threads(threads) collapse(2)
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            float nx = x - ncx;
            float ny = y - ncy;

            float src_x = inverse_matrix[0][0] * nx + inverse_matrix[0][1] * ny + cx;
            float src_y = inverse_matrix[1][0] * nx + inverse_matrix[1][1] * ny + cy;

            int px = static_cast<int>(src_x);
            int py = static_cast<int>(src_y);
            float x_diff = src_x - px;
            float y_diff = src_y - py;

            if (px >= 0 && px < bmp->width - 1 && py >= 0 && py < bmp->height - 1) {
                pixel a = bmp->pixels[py * bmp->width + px];
                pixel b = bmp->pixels[py * bmp->width + (px + 1)];
                pixel c = bmp->pixels[(py + 1) * bmp->width + px];
                pixel d = bmp->pixels[(py + 1) * bmp->width + (px + 1)];

                pixel p;
                p.red = a.red * (1 - x_diff) * (1 - y_diff) + b.red * x_diff * (1 - y_diff) +
                        c.red * y_diff * (1 - x_diff) + d.red * x_diff * y_diff;
                p.green = a.green * (1 - x_diff) * (1 - y_diff) + b.green * x_diff * (1 - y_diff) +
                          c.green * y_diff * (1 - x_diff) + d.green * x_diff * y_diff;
                p.blue = a.blue * (1 - x_diff) * (1 - y_diff) + b.blue * x_diff * (1 - y_diff) +
                         c.blue * y_diff * (1 - x_diff) + d.blue * x_diff * y_diff;

                rotated_bmp->pixels[y * new_width + x] = p;
            }
            else {
                rotated_bmp->pixels[y * new_width + x] = { 0, 0, 0, 0 };
            }
        }
    }
}

BMP* pad_image(BMP* bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    int new_width = bmp->width + 2 * pad;
    int new_height = bmp->height + 2 * pad;

    BMP* padded_bmp = (BMP*)malloc(sizeof(BMP));
    initialize_bmp(padded_bmp, bmp, new_width, new_height, bmp->depth);

    memset(padded_bmp->pixels, 0, new_width * new_height * sizeof(pixel));

    for (int y = 0; y < bmp->height; ++y) {
        for (int x = 0; x < bmp->width; ++x) {
            padded_bmp->pixels[(y + pad) * new_width + (x + pad)] = bmp->pixels[y * bmp->width + x];
        }
    }

    //Верх и низ
    for (int y = 0; y < pad; ++y) {
        for (int x = 0; x < new_width; ++x) {
            int reflected_y = pad - y;
            switch (padding_type) {
            case REPLICATE:
                padded_bmp->pixels[y * new_width + x] = padded_bmp->pixels[pad * new_width + x];
                padded_bmp->pixels[(new_height - 1 - y) * new_width + x] = padded_bmp->pixels[(new_height - 1 - pad) * new_width + x];
                break;
            case REFLECT:
                while (reflected_y < 0 || reflected_y >= bmp->height) {
                    if (reflected_y < 0) reflected_y = -reflected_y;
                    if (reflected_y >= bmp->height) reflected_y = 2 * bmp->height - reflected_y - 1;
                }
                padded_bmp->pixels[y * new_width + x] = padded_bmp->pixels[(reflected_y + pad) * new_width + x];

                reflected_y = new_height - 1 - pad + (pad - y);
                while (reflected_y < 0 || reflected_y >= bmp->height) {
                    if (reflected_y < 0) reflected_y = -reflected_y;
                    if (reflected_y >= bmp->height) reflected_y = 2 * bmp->height - reflected_y - 1;
                }
                padded_bmp->pixels[(new_height - 1 - y) * new_width + x] = padded_bmp->pixels[(reflected_y + pad) * new_width + x];
                break;
            case ZERO:
                padded_bmp->pixels[y * new_width + x] = { 0, 0, 0, 0 };
                padded_bmp->pixels[(new_height - 1 - y) * new_width + x] = { 0, 0, 0, 0 };
                break;
            }
        }
    }

    //Слева и справа
    for (int y = pad; y < new_height - pad; ++y) {
        for (int x = 0; x < pad; ++x) {
            int reflected_x = pad - x;
            switch (padding_type) {
            case REPLICATE:
                padded_bmp->pixels[y * new_width + x] = padded_bmp->pixels[y * new_width + pad];
                padded_bmp->pixels[y * new_width + (new_width - 1 - x)] = padded_bmp->pixels[y * new_width + (new_width - 1 - pad)];
                break;
            case REFLECT:
                while (reflected_x < 0 || reflected_x >= bmp->width) {
                    if (reflected_x < 0) reflected_x = -reflected_x;
                    if (reflected_x >= bmp->width) reflected_x = 2 * bmp->width - reflected_x - 1;
                }
                padded_bmp->pixels[y * new_width + x] = padded_bmp->pixels[y * new_width + (reflected_x + pad)];

                reflected_x = new_width - 1 - pad + (pad - x);
                while (reflected_x < 0 || reflected_x >= bmp->width) {
                    if (reflected_x < 0) reflected_x = -reflected_x;
                    if (reflected_x >= bmp->width) reflected_x = 2 * bmp->width - reflected_x - 1;
                }
                padded_bmp->pixels[y * new_width + (new_width - 1 - x)] = padded_bmp->pixels[y * new_width + (reflected_x + pad)];
                break;
            case ZERO:
                padded_bmp->pixels[y * new_width + x] = { 0, 0, 0, 0 };
                padded_bmp->pixels[y * new_width + (new_width - 1 - x)] = { 0, 0, 0, 0 };
                break;
            }
        }
    }

    //Углы
    for (int y = 0; y < pad; ++y) {
        for (int x = 0; x < pad; ++x) {
            int reflected_x = pad - x;
            int reflected_y = pad - y;
            switch (padding_type) {
            case REPLICATE:
                padded_bmp->pixels[y * new_width + x] = padded_bmp->pixels[pad * new_width + pad];
                padded_bmp->pixels[y * new_width + (new_width - 1 - x)] = padded_bmp->pixels[pad * new_width + (new_width - 1 - pad)];
                padded_bmp->pixels[(new_height - 1 - y) * new_width + x] = padded_bmp->pixels[(new_height - 1 - pad) * new_width + pad];
                padded_bmp->pixels[(new_height - 1 - y) * new_width + (new_width - 1 - x)] = padded_bmp->pixels[(new_height - 1 - pad) * new_width + (new_width - 1 - pad)];
                break;
            case REFLECT:
                while (reflected_x < 0 || reflected_x >= bmp->width) {
                    if (reflected_x < 0) reflected_x = -reflected_x;
                    if (reflected_x >= bmp->width) reflected_x = 2 * bmp->width - reflected_x - 1;
                }
                while (reflected_y < 0 || reflected_y >= bmp->height) {
                    if (reflected_y < 0) reflected_y = -reflected_y;
                    if (reflected_y >= bmp->height) reflected_y = 2 * bmp->height - reflected_y - 1;
                }
                padded_bmp->pixels[y * new_width + x] = padded_bmp->pixels[(reflected_y + pad) * new_width + (reflected_x + pad)];

                reflected_x = new_width - 1 - pad + (pad - x);
                reflected_y = pad - y;
                while (reflected_x < 0 || reflected_x >= bmp->width) {
                    if (reflected_x < 0) reflected_x = -reflected_x;
                    if (reflected_x >= bmp->width) reflected_x = 2 * bmp->width - reflected_x - 1;
                }
                while (reflected_y < 0 || reflected_y >= bmp->height) {
                    if (reflected_y < 0) reflected_y = -reflected_y;
                    if (reflected_y >= bmp->height) reflected_y = 2 * bmp->height - reflected_y - 1;
                }
                padded_bmp->pixels[y * new_width + (new_width - 1 - x)] = padded_bmp->pixels[(reflected_y + pad) * new_width + (reflected_x + pad)];

                reflected_x = pad - x;
                reflected_y = new_height - 1 - pad + (pad - y);
                while (reflected_x < 0 || reflected_x >= bmp->width) {
                    if (reflected_x < 0) reflected_x = -reflected_x;
                    if (reflected_x >= bmp->width) reflected_x = 2 * bmp->width - reflected_x - 1;
                }
                while (reflected_y < 0 || reflected_y >= bmp->height) {
                    if (reflected_y < 0) reflected_y = -reflected_y;
                    if (reflected_y >= bmp->height) reflected_y = 2 * bmp->height - reflected_y - 1;
                }
                padded_bmp->pixels[(new_height - 1 - y) * new_width + x] = padded_bmp->pixels[(reflected_y + pad) * new_width + (reflected_x + pad)];

                reflected_x = new_width - 1 - pad + (pad - x);
                reflected_y = new_height - 1 - pad + (pad - y);
                while (reflected_x < 0 || reflected_x >= bmp->width) {
                    if (reflected_x < 0) reflected_x = -reflected_x;
                    if (reflected_x >= bmp->width) reflected_x = 2 * bmp->width - reflected_x - 1;
                }
                while (reflected_y < 0 || reflected_y >= bmp->height) {
                    if (reflected_y < 0) reflected_y = -reflected_y;
                    if (reflected_y >= bmp->height) reflected_y = 2 * bmp->height - reflected_y - 1;
                }
                padded_bmp->pixels[(new_height - 1 - y) * new_width + (new_width - 1 - x)] = padded_bmp->pixels[(reflected_y + pad) * new_width + (reflected_x + pad)];
                break;
            case ZERO:
                padded_bmp->pixels[y * new_width + x] = { 0, 0, 0, 0 };
                padded_bmp->pixels[y * new_width + (new_width - 1 - x)] = { 0, 0, 0, 0 };
                padded_bmp->pixels[(new_height - 1 - y) * new_width + x] = { 0, 0, 0, 0 };
                padded_bmp->pixels[(new_height - 1 - y) * new_width + (new_width - 1 - x)] = { 0, 0, 0, 0 };
                break;
            }
        }
    }

    return padded_bmp;
}

void apply_average_filter(BMP* bmp, BMP* filtered_bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);

    int new_width = bmp->width + 2 * pad;
    int new_height = bmp->height + 2 * pad;

    BMP* buffer_bmp = (BMP*)malloc(sizeof(BMP));
    initialize_bmp(buffer_bmp, bmp, bmp->width, bmp->height, bmp->depth);

#pragma omp parallel for collapse(2)
    for (int y = 0; y < bmp->height; ++y) {
        for (int x = 0; x < bmp->width; ++x) {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            for (int ky = -pad; ky <= pad; ++ky) {
                for (int kx = -pad; kx <= pad; ++kx) {
                    pixel p = padded_bmp->pixels[(y + pad + ky) * new_width + (x + pad + kx)];
                    sum_r += p.red;
                    sum_g += p.green;
                    sum_b += p.blue;
                }
            }
            int area = k * k;
            pixel p;
            p.red = sum_r / area;
            p.green = sum_g / area;
            p.blue = sum_b / area;
            p.alpha = 255;

            buffer_bmp->pixels[y * bmp->width + x] = p;
        }
    }

    memcpy(filtered_bmp->pixels, buffer_bmp->pixels, bmp->width * bmp->height * sizeof(pixel));

    bclose(padded_bmp);
    bclose(buffer_bmp);
}

void apply_average_filter_float(BMP* bmp, BMP* filtered_bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);

    int new_width = bmp->width + 2 * pad;
    int new_height = bmp->height + 2 * pad;

    BMP* buffer_bmp = (BMP*)malloc(sizeof(BMP));
    initialize_bmp(buffer_bmp, bmp, bmp->width, bmp->height, bmp->depth);

#pragma omp parallel for
    for (int y = 0; y < bmp->height; ++y) {
        int sum_r = 0, sum_g = 0, sum_b = 0;
        for (int ky = -pad; ky <= pad; ++ky) {
            for (int kx = -pad; kx <= pad; ++kx) {
                pixel p = padded_bmp->pixels[(y + pad + ky) * new_width + (kx + pad)];
                sum_r += p.red;
                sum_g += p.green;
                sum_b += p.blue;
            }
        }

        for (int x = 0; x < bmp->width; ++x) {
            int area = k * k;
            pixel p;
            p.red = sum_r / area;
            p.green = sum_g / area;
            p.blue = sum_b / area;
            p.alpha = 255;

            buffer_bmp->pixels[y * bmp->width + x] = p;

            if (x + 1 < bmp->width) {
                for (int ky = -pad; ky <= pad; ++ky) {
                    pixel p_remove = padded_bmp->pixels[(y + pad + ky) * new_width + (x + pad - pad)];
                    pixel p_add = padded_bmp->pixels[(y + pad + ky) * new_width + (x + pad + pad + 1)];

                    sum_r -= p_remove.red;
                    sum_r += p_add.red;
                    sum_g -= p_remove.green;
                    sum_g += p_add.green;
                    sum_b -= p_remove.blue;
                    sum_b += p_add.blue;
                }
            }
        }
    }

    memcpy(filtered_bmp->pixels, buffer_bmp->pixels, bmp->width * bmp->height * sizeof(pixel));

    bclose(padded_bmp);
    bclose(buffer_bmp);
}

void apply_median_filter_sort(BMP* bmp, BMP* filtered_bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);
    int block_size = 64;
    int new_width = bmp->width + 2 * pad;

#pragma omp parallel for collapse(2)
    for (int y_block = 0; y_block < bmp->height; y_block += block_size) {
        for (int x_block = 0; x_block < bmp->width; x_block += block_size) {

            for (int y = y_block; y < y_block + block_size && y < bmp->height; ++y) {
                std::multiset<int> reds, greens, blues;

                for (int ky = -pad; ky <= pad; ++ky) {
                    for (int kx = -pad; kx <= pad; ++kx) {
                        pixel p = padded_bmp->pixels[(y + pad + ky) * new_width + (kx + pad + x_block)];
                        reds.insert(p.red);
                        greens.insert(p.green);
                        blues.insert(p.blue);
                    }
                }

                int median_index = reds.size() / 2;

                pixel p;
                auto it_red = std::next(reds.begin(), median_index);
                auto it_green = std::next(greens.begin(), median_index);
                auto it_blue = std::next(blues.begin(), median_index);

                p.red = *it_red;
                p.green = *it_green;
                p.blue = *it_blue;
                p.alpha = 255;

                filtered_bmp->pixels[y * bmp->width + x_block] = p;

                for (int x = x_block + 1; x < x_block + block_size && x < bmp->width; ++x) {

                    for (int ky = -pad; ky <= pad; ++ky) {
                        int old_col_pos = (y + pad + ky) * new_width + (x + pad - pad - 1);
                        pixel old_pixel = padded_bmp->pixels[old_col_pos];

                        reds.erase(reds.find(old_pixel.red));
                        greens.erase(greens.find(old_pixel.green));
                        blues.erase(blues.find(old_pixel.blue));
                    }

                    for (int ky = -pad; ky <= pad; ++ky) {
                        int new_col_pos = (y + pad + ky) * new_width + (x + pad + pad);
                        pixel new_pixel = padded_bmp->pixels[new_col_pos];

                        reds.insert(new_pixel.red);
                        greens.insert(new_pixel.green);
                        blues.insert(new_pixel.blue);
                    }

                    auto it_red = std::next(reds.begin(), median_index);
                    auto it_green = std::next(greens.begin(), median_index);
                    auto it_blue = std::next(blues.begin(), median_index);

                    p.red = *it_red;
                    p.green = *it_green;
                    p.blue = *it_blue;
                    p.alpha = 255;

                    filtered_bmp->pixels[y * bmp->width + x] = p;
                }
            }
        }
    }

    bclose(padded_bmp);
}

void apply_median_filter_nth_element(BMP* bmp, BMP* filtered_bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);

    int new_width = bmp->width + 2 * pad;
    int new_height = bmp->height + 2 * pad;

    //#pragma omp parallel for
    for (int y = 0; y < bmp->height; ++y) {
        std::vector<int> reds, greens, blues;

        for (int ky = -pad; ky <= pad; ++ky) {
            for (int kx = -pad; kx <= pad; ++kx) {
                pixel p = padded_bmp->pixels[(y + pad + ky) * new_width + (kx + pad)];
                reds.push_back(p.red);
                greens.push_back(p.green);
                blues.push_back(p.blue);
            }
        }

        int median_index = reds.size() / 2;

        std::nth_element(reds.begin(), reds.begin() + median_index, reds.end());
        std::nth_element(greens.begin(), greens.begin() + median_index, greens.end());
        std::nth_element(blues.begin(), blues.begin() + median_index, blues.end());

        pixel p;
        p.red = reds[median_index];
        p.green = greens[median_index];
        p.blue = blues[median_index];
        p.alpha = 255;

        filtered_bmp->pixels[y * bmp->width] = p;

        for (int x = 1; x < bmp->width; ++x) {
            for (int ky = -pad; ky <= pad; ++ky) {
                int old_col_pos = (y + pad + ky) * new_width + (x + pad - pad - 1);
                pixel old_pixel = padded_bmp->pixels[old_col_pos];

                reds.erase(std::find(reds.begin(), reds.end(), old_pixel.red));
                greens.erase(std::find(greens.begin(), greens.end(), old_pixel.green));
                blues.erase(std::find(blues.begin(), blues.end(), old_pixel.blue));
            }

            for (int ky = -pad; ky <= pad; ++ky) {
                int new_col_pos = (y + pad + ky) * new_width + (x + pad + pad);
                pixel new_pixel = padded_bmp->pixels[new_col_pos];

                reds.push_back(new_pixel.red);
                greens.push_back(new_pixel.green);
                blues.push_back(new_pixel.blue);
            }

            std::nth_element(reds.begin(), reds.begin() + median_index, reds.end());
            std::nth_element(greens.begin(), greens.begin() + median_index, greens.end());
            std::nth_element(blues.begin(), blues.begin() + median_index, blues.end());

            p.red = reds[median_index];
            p.green = greens[median_index];
            p.blue = blues[median_index];
            p.alpha = 255;

            filtered_bmp->pixels[y * bmp->width + x] = p;
        }
    }

    bclose(padded_bmp);
}

void apply_median_filter_set(BMP* bmp, BMP* filtered_bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);

    int new_width = bmp->width + 2 * pad;
    int new_height = bmp->height + 2 * pad;

#pragma omp parallel for
    for (int y = 0; y < bmp->height; ++y) {
        std::multiset<int> reds, greens, blues;

        for (int ky = -pad; ky <= pad; ++ky) {
            for (int kx = -pad; kx <= pad; ++kx) {
                pixel p = padded_bmp->pixels[(y + pad + ky) * new_width + (kx + pad)];
                reds.insert(p.red);
                greens.insert(p.green);
                blues.insert(p.blue);
            }
        }

        int median_index = reds.size() / 2;

        pixel p;
        auto it_red = std::next(reds.begin(), median_index);
        auto it_green = std::next(greens.begin(), median_index);
        auto it_blue = std::next(blues.begin(), median_index);

        p.red = *it_red;
        p.green = *it_green;
        p.blue = *it_blue;
        p.alpha = 255;

        filtered_bmp->pixels[y * bmp->width] = p;

        for (int x = 1; x < bmp->width; ++x) {

            for (int ky = -pad; ky <= pad; ++ky) {
                int old_col_pos = (y + pad + ky) * new_width + (x + pad - pad - 1);
                pixel old_pixel = padded_bmp->pixels[old_col_pos];

                reds.erase(reds.find(old_pixel.red));
                greens.erase(greens.find(old_pixel.green));
                blues.erase(blues.find(old_pixel.blue));
            }

            for (int ky = -pad; ky <= pad; ++ky) {
                int new_col_pos = (y + pad + ky) * new_width + (x + pad + pad);
                pixel new_pixel = padded_bmp->pixels[new_col_pos];

                reds.insert(new_pixel.red);
                greens.insert(new_pixel.green);
                blues.insert(new_pixel.blue);
            }

            auto it_red = std::next(reds.begin(), median_index);
            auto it_green = std::next(greens.begin(), median_index);
            auto it_blue = std::next(blues.begin(), median_index);

            p.red = *it_red;
            p.green = *it_green;
            p.blue = *it_blue;
            p.alpha = 255;

            filtered_bmp->pixels[y * bmp->width + x] = p;
        }
    }

    bclose(padded_bmp);
}

void insert_sorted(std::vector<int>& vec, const int& value) {
    int low = 0;
    int high = vec.size();

    while (low < high) {
        int mid = low + (high - low) / 2;
        if (vec[mid] < value) {
            low = mid + 1;
        }
        else {
            high = mid;
        }
    }

    vec.insert(vec.begin() + low, value);
}


void remove_sorted(std::vector<int>& vec, const int& value) {
    int low = 0;
    int high = vec.size();

    while (low < high) {
        int mid = low + (high - low) / 2;
        if (vec[mid] < value) {
            low = mid + 1;
        }
        else {
            high = mid;
        }
    }

    if (low < vec.size() && vec[low] == value) {
        vec.erase(vec.begin() + low);
    }
}

void apply_median_filter_sort_binary(BMP* bmp, BMP* filtered_bmp, int k, PaddingType padding_type) {
    int pad = k / 2;
    BMP* padded_bmp = pad_image(bmp, k, padding_type);

    int new_width = bmp->width + 2 * pad;

#pragma omp parallel for
    for (int y = 0; y < bmp->height; ++y) {
        std::vector<int> reds, greens, blues;

        for (int ky = -pad; ky <= pad; ++ky) {
            for (int kx = -pad; kx <= pad; ++kx) {
                pixel p = padded_bmp->pixels[(y + pad + ky) * new_width + (kx + pad)];
                reds.push_back(p.red);
                greens.push_back(p.green);
                blues.push_back(p.blue);
            }
        }

        std::sort(reds.begin(), reds.end());
        std::sort(greens.begin(), greens.end());
        std::sort(blues.begin(), blues.end());

        int median_index = reds.size() / 2;

        pixel p;
        p.red = reds[median_index];
        p.green = greens[median_index];
        p.blue = blues[median_index];
        p.alpha = 255;

        filtered_bmp->pixels[y * bmp->width] = p;

        for (int x = 1; x < bmp->width; ++x) {
            for (int ky = -pad; ky <= pad; ++ky) {
                int old_col_pos = (y + pad + ky) * new_width + (x + pad - pad - 1);
                pixel old_pixel = padded_bmp->pixels[old_col_pos];

                remove_sorted(reds, old_pixel.red);
                remove_sorted(greens, old_pixel.green);
                remove_sorted(blues, old_pixel.blue);
            }

            for (int ky = -pad; ky <= pad; ++ky) {
                int new_col_pos = (y + pad + ky) * new_width + (x + pad + pad);
                pixel new_pixel = padded_bmp->pixels[new_col_pos];

                insert_sorted(reds, new_pixel.red);
                insert_sorted(greens, new_pixel.green);
                insert_sorted(blues, new_pixel.blue);
            }

            p.red = reds[median_index];
            p.green = greens[median_index];
            p.blue = blues[median_index];
            p.alpha = 255;

            filtered_bmp->pixels[y * bmp->width + x] = p;
        }
    }

    bclose(padded_bmp);
}
