#include "sdkconfig.h"
#include <cstring>
#include <esp_log.h>
#include <soc/soc_memory_types.h>  // for esp_ptr_byte_accessible

#include "miscellaneous.hpp"

//print number of bytes per line for esp_log_buffer_char and esp_log_buffer_hex
#define BYTES_PER_LINE 16

/**
 * @brief Function to dump a buffer : ISR compatible as it use esp_rom_printf function
 * 
 * @param tag Associated TAG
 * @param buffer pointer on buffer
 * @param buff_len length to print in byte
 */
void IRAM_ATTR OPTIMIZE_SPEED_O3 esp_log_buffer_hexdump(const char *tag, const char *buffer, uint16_t buff_len, esp_log_level_t log_level)
{
    if ((buffer==nullptr) ||(buff_len == 0)) {
        return;
    }
    char temp_buffer[BYTES_PER_LINE + 3]; //for not-byte-accessible memory
    const char *ptr_line;
    //format: field[length]
    // ADDR[10]+"   "+DATA_HEX[8*3]+" "+DATA_HEX[8*3]+"  |"+DATA_CHAR[8]+"|"
    char hd_buffer[10 + 3 + BYTES_PER_LINE * 3 + 3 + BYTES_PER_LINE + 1 + 1];
    char *ptr_hd;
    int bytes_cur_line;
    do {
        if (buff_len > BYTES_PER_LINE) {
            bytes_cur_line = BYTES_PER_LINE;
        } else {
            bytes_cur_line = buff_len;
        }
        if (!esp_ptr_byte_accessible(buffer)) {
            //use memcpy to get around alignment issue
            memcpy(temp_buffer, buffer, (bytes_cur_line + 3) / 4 * 4);
            ptr_line = temp_buffer;
        } else {
            ptr_line = buffer; // workaround to avoid compilation error
        }
        ptr_hd = hd_buffer;

        ptr_hd += sprintf(ptr_hd, "%p ", buffer);
        for (int i = 0; i < BYTES_PER_LINE; ++i) {
            if ((i & 7) == 0) {
                ptr_hd += sprintf(ptr_hd, " ");
            }
            if (i < bytes_cur_line) {
                ptr_hd += sprintf(ptr_hd, " %02x", ptr_line[i]);
            } else {
                ptr_hd += sprintf(ptr_hd, "   ");
            }
        }
        ptr_hd += sprintf(ptr_hd, "  |");
        for (int i = 0; i < bytes_cur_line; ++i) {
            if (isprint((int)ptr_line[i])) {
                ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
            } else {
                ptr_hd += sprintf(ptr_hd, ".");
            }
        }
        ptr_hd += sprintf(ptr_hd, "|");
        esp_rom_printf(DRAM_STR(LOG_FORMAT_ISR_SAFE("%s")),tag,hd_buffer);
        //esp_rom_printf("%s\n", hd_buffer);
        buffer = buffer + bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}


/*
void hexDumpISR(char *desc, void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *)addr;

    // Output description if given.
    if (desc != NULL)
        esp_rom_printf("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                esp_rom_printf("  %s\n", buff);

            // Output the offset.
            esp_rom_printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        esp_rom_printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
        {
            buff[i % 16] = '.';
        }
        else
        {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        esp_rom_printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    esp_rom_printf("  %s\n", buff);
}
*/