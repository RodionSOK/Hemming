#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define FILESIZE 100
const int TWO_ST[] = { 1, 2, 4, 8, 16, 32, 64 };


int get_size(char filename[]) {
    FILE* fin_gs = fopen(filename, "rb");
    fseek(fin_gs, 0, SEEK_END);
    int size = ftell(fin_gs);
    return size;
}



int sum(int arr[], int size) {
    int suma = 0;
    for (int i = 0; i < size; i++) {
        suma += arr[i];
    }
    return suma;
}



int actions() {
    int choice;
    printf("1 - Encode.\n2 - Decode.\n");
    scanf("%d", &choice);
    if (choice == 1) {
        return 1;
    }
    else {
        return 2;
    }
}


void write_bit(int encode_text[], int bit_count) {
    FILE* fout_wb = fopen("text.out.bin", "wb");
    uint8_t buffer = 0;
    int bit_position = 0;
    for (int i = 0; i < bit_count; i++) {
        buffer |= (encode_text[i] & 1) << (7 - bit_position);
        bit_position++;
        if (bit_position == 8) {
            fwrite(&buffer, sizeof(uint8_t), 1, fout_wb);
            buffer = 0;
            bit_position = 0;
        }
    }
    if (bit_position > 0) {
        fwrite(&buffer, sizeof(uint8_t), 1, fout_wb);
    }
    fclose(fout_wb);
}


void read_bit(int encode_text[], int bit_count) {
    FILE* fin_rb = fopen("text.out.bin", "rb");
    uint8_t buffer = 0;
    int bit_position = 0;
    for (int i = 0; i < bit_count; i++) {
        if (bit_position == 0) {
            fread(&buffer, sizeof(uint8_t), 1, fin_rb);
        }
        encode_text[i] = (int)(buffer >> (7 - bit_position)) & 1;
        bit_position++;
        if (bit_position == 8) {
            bit_position = 0;
        }
    }
}


void text_to_bin(int text_bin[]) {
    FILE* fin_ttb = fopen("text.txt", "r");
    char text_str[FILESIZE + 1];
    fgets(text_str, sizeof(text_str), fin_ttb);
    for (int i = 0; i < FILESIZE; i++) {
        for (int j = 7; j >= 0; j--) {
            int bit = (text_str[i] >> j) & 1;
            text_bin[(7 - j) + i * 8] = bit;           
        }
    }
    fclose(fin_ttb);

}


void bin_to_text(int text_bin[], int bit_count) {
    FILE* fout_btt = fopen("text.decode.txt", "w");
    char text_str[FILESIZE + 1];
    int byte_count = bit_count / 8;
    for (int i = 0; i < byte_count; i++) {
        text_str[i] = 0;
        for (int j = 0; j < 8; j++) {
            text_str[i] |= (text_bin[i * 8 + j] & 1) << (7 - j);
        }
    }
    text_str[bit_count] = '\0';
    fprintf(fout_btt, "%s", text_str);
    fclose(fout_btt);
}


int count_control_bits(int size) {
    int count = 0;
    int two_st = 1;
    while (two_st <= size) {
        count += 1;
        two_st *= 2;
    }
    return count;
}


void encode(int size) {
    int text_bin[FILESIZE * 8];
    text_to_bin(text_bin);
    int control_count = count_control_bits(size);
    int* encode_text = (int*)calloc((FILESIZE * 8 / size + 1) * (size + control_count + 1), sizeof(int));
    for (int i = 0; i < FILESIZE * 8 / size + 1; i++) {
        *(encode_text + i * (size + control_count + 1) + 0) = 0;
        for (int j = 1, k = 0; j < (size + control_count + 1); j++) {
                int flag = 1;
                for (int z = 0; z < control_count; z++) {
                    if (j == pow(2, z)) {
                        flag = 0;
                        break;
                    }
                }
                if (flag) {
                    *(encode_text + i * (size + control_count + 1) + j) = *(text_bin + i * size + k);
                    k++;
                }
                else {
                    *(encode_text + i * (size + control_count + 1) + j) = 0;
                }
        }
    }
    for (int i = 0; i < FILESIZE * 8 / size + 1; i++) {
        int count_chet = 0;
        for (int j = 0; j < control_count; j++) {
            int index = TWO_ST[j];
            int count = 0;
            for (int k = index; k < (size + control_count + 1); k += (index) * 2) {
                for (int z = k; z < k + index; z++) {
                    if (z != index && z < (size + control_count + 1)) {
                        count ^= *(encode_text + i * (size + control_count + 1) + z);
                    }
                }
            }
            *(encode_text + i * (size + control_count + 1) + index) = count;
        }
        for (int j = 0; j < (size + control_count + 1); j++) {
            count_chet ^= *(encode_text + i * (size + control_count + 1) + j);
        }
        *(encode_text + i * (size + control_count + 1) + 0) = count_chet;
    }
    int bit_count = (FILESIZE * 8 / size + 1) * (size + control_count + 1);
    write_bit(encode_text, bit_count);
    free(encode_text);
}

void decode(int size) {
    int file_size = get_size("text.out.bin");
    int* encode_text = (int*)calloc(file_size * 8, sizeof(int));
    read_bit(encode_text, file_size * 8);
    int control_count = count_control_bits(size);
    int* text_bin = (int*)calloc(FILESIZE * 8 + 1, sizeof(int));
    for (int i = 0; i < FILESIZE * 8 / size + 1; i++) {
        int control_summ = 0;
        int* syndrome = (int*)calloc(control_count, sizeof(int));
        for (int j = 0; j < control_count; j++) {
            int index = TWO_ST[j];
            for (int k = index + (size + control_count + 1) * i; k < (size + control_count + 1) * (i + 1); k += (index) * 2) {
                for (int z = k; z < k + index; z++) {
                    if (z < (size + control_count + 1) * (i + 1)) {
                        syndrome[j] ^= *(encode_text + z);
                    }
                    
                }
        
            }
        }
        for (int j = 0; j < (control_count + size + 1); j++) {
            control_summ ^= *(encode_text + i * (size + control_count + 1) + j);
        }
        if ((control_summ == 0) || (control_summ != 0 && sum(syndrome, control_count) == 0)) {
            for (int j = 0, k = 0; j < (size + control_count + 1); j++) {
                int flag = 1;
                for (int z = 0; z < control_count; z++) {
                    if (j == pow(2, z)) {
                        flag = 0;
                    }
                }
                if (j != 0 && flag) {
                    *(text_bin + k + size * i) = *(encode_text + i * (size + control_count + 1) + j);
                    k++;
                }
            }
        }
        else {
            int index = 0;
            for (int j = 0; j < control_count; j++) {
                if (syndrome[j]) {
                    index += pow(2, j);
                }
            }
            if (*(encode_text + i * (size + control_count + 1) + index) == 0) {
                *(encode_text + i * (size + control_count + 1) + index) = 1;
            }
            else {
                *(encode_text + i * (size + control_count + 1) + index) = 0;
            }
            for (int j = 0, k = 0; j < (size + control_count + 1); j++) {
                int flag = 1;
                for (int z = 0; z < control_count; z++) {
                    if (j == pow(2, z)) {
                        flag = 0;
                    }
                }
                if (j != 0 && flag) {
                    *(text_bin + k + size * i) = *(encode_text + i * (size + control_count + 1) + j);
                    k++;
                }
            }
        }
        free(syndrome);
    }
    bin_to_text(text_bin, FILESIZE * 8);
    free(text_bin);
}


int main(int argc, char *argv[])
{
    const int SIZE = atoi(argv[1]);

    if (actions() == 1) {
        encode(SIZE);
    }
    else {
        decode(SIZE);
    }
}