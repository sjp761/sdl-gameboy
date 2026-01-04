#include <cstdint>
#include <bus.h>

struct dma_ctx
{
    bool active;
    uint8_t current_iter;
    uint8_t start_addr;
    uint8_t start_delay;
};

class DMA
{
    public:
        void set_cmp(Bus* bus_ptr)
        {
            this->bus = bus_ptr;
        }
        void start(uint8_t value);
        void tick();
        bool is_active() const;
    private:
        Bus* bus;
        dma_ctx ctx;


};