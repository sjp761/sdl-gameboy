#include "dma.h"

void DMA::start(uint8_t value)
{
    ctx.active = true;
    ctx.value = value;
    ctx.byte = 0;
    ctx.start_delay = 2; // Typically, there's a small delay before starting DMA
}

void DMA::tick()
{
    if (!ctx.active) return;

    if (ctx.start_delay > 0) {
        ctx.start_delay--;
        return;
    }

    // Perform one byte transfer per tick, shift by 8 is multiplying by 256 or 0x100
    uint16_t source_address = (ctx.value << 8) + ctx.byte;
    uint16_t dest_address = 0xFE00 + ctx.byte; // OAM memory starts at 0xFE00

    uint8_t data = bus->bus_read(source_address);
    bus->bus_write(dest_address, data);

    ctx.byte++;

    // Check if DMA transfer is complete
    ctx.active = (ctx.byte < 0xA0); // 160 bytes to transfer
}

bool DMA::is_active()
{
    return ctx.active;
}
