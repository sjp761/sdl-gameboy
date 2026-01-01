#include "dma.h"

void DMA::start(uint8_t value)
{
    ctx.active = true;
    ctx.start_addr = value;
    ctx.current_iter = 0;
    ctx.start_delay = 1; // We are using one because we tick DMA in m-cycles
}

void DMA::tick()
{
    if (ctx.start_delay > 0) {
        ctx.start_delay--;
        return;
    }

    // Perform one byte transfer per tick, shift by 8 is multiplying by 256 or 0x100
    uint16_t source_address = (ctx.start_addr << 8) + ctx.current_iter;
    uint16_t dest_address = 0xFE00 + ctx.current_iter; // OAM memory starts at 0xFE00

    uint8_t data = bus->bus_read(source_address);
    bus->bus_write(dest_address, data);

    ctx.current_iter++;

    // Check if DMA transfer is complete
    ctx.active = (ctx.current_iter < 160); // 160 bytes to transfer
}

bool DMA::is_active()
{
    return ctx.active;
}
