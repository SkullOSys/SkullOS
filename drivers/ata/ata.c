#include "ata.h"
#include "../../kernel/util.h" // For inb/outb
#include "../../kernel/vga_manager.h" // For vga_manager_puts

// TODO: Implement a better way to handle delays
void sleep(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 100000; i++) {
        __asm__ volatile("nop");
    }
}

static void ata_wait_busy() {
    // Wait for BSY to be 0
    while (inb(ATA_PRIMARY_STATUS) & ATA_SR_BSY);
}

static void ata_wait_drq() {
    // Wait for DRQ to be 1
    while (!(inb(ATA_PRIMARY_STATUS) & ATA_SR_DRQ));
}

void ata_init() {
    // Select drive 0 (master)
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xA0);
    
    // Send IDENTIFY command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY_DEVICE);
    
    // Poll for status
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status) {
        ata_wait_busy();
        
        // Check for non-ATA device
        if ((inb(ATA_PRIMARY_LBA_MID) != 0) || (inb(ATA_PRIMARY_LBA_HIGH) != 0)) {
            vga_manager_puts("Not an ATA device.\n");
            return;
        }

        ata_wait_drq();

        // Read the IDENTIFY data (256 words)
        uint16_t identify_data[256];
        for(int i = 0; i < 256; i++) {
            identify_data[i] = inw(ATA_PRIMARY_DATA);
        }

        vga_manager_puts("ATA drive detected.\n");

    } else {
        vga_manager_puts("No drive detected.\n");
    }
}

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    // Select drive (Master/Slave) and set LBA mode
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Send sector count and LBA address
    outb(ATA_PRIMARY_SECTORS, 1); // We are reading 1 sector
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Send the READ PIO command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);
    
    // Wait for the drive to be ready
    ata_wait_busy();
    ata_wait_drq();
    
    // Read the sector data (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_PRIMARY_DATA);
        buffer[i*2] = data & 0xFF; // Low byte
        buffer[i*2+1] = (data >> 8) & 0xFF; // High byte
    }
}

void ata_write_sector(uint32_t lba, uint8_t* buffer) {
    // Select drive (Master/Slave) and set LBA mode
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Send sector count and LBA address
    outb(ATA_PRIMARY_SECTORS, 1); // We are writing 1 sector
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Send the WRITE PIO command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);
    
    // Wait for the drive to be ready
    ata_wait_busy();
    ata_wait_drq();
    
    // Write the sector data (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        uint16_t data = (buffer[i*2+1] << 8) | buffer[i*2];
        outw(ATA_PRIMARY_DATA, data);
    }
}
