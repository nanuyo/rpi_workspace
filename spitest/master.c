#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

#define SPI_DEVICE "/dev/spidev0.0"

int main() {
    int spi_fd;
    //uint8_t tx_buf[3] = {0x01, 0x02, 0x03}; // Example data to be sent
    uint8_t tx_buf[3] = {'a' , 'b', 'c'}; // Example data to be sent
    uint8_t rx_buf[3] = {0}; // Initialize receive buffer

    // Open SPI device
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        return -1;
    }

    // Transfer data
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = sizeof(tx_buf),
        .speed_hz = 1000000, // Set SPI speed to 1 MHz
        .delay_usecs = 0,
        .bits_per_word = 8,
    };

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Error transmitting/receiving SPI data");
        return -1;
    }

    // Print received data
    printf("Received data from slave: ");
    for (int i = 0; i < sizeof(rx_buf); i++) {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");

    // Close SPI device
    close(spi_fd);

    return 0;
}
