#ifndef Encoder_h
#define Encoder_h

template<typename A, typename B>
struct Encoder {
private:
    typedef VirtualPort<A, B> Bus;

    volatile static uint8_t prevState;
    volatile static uint8_t prevValue;
    volatile static uint8_t currValue;

public:
    static void init() {
        Bus::pullUp(0xFF);
    }

    static void capture() {
        auto currState = Bus::template read<uint8_t>();

        switch (currState | prevState << 2) {
            case 0x2: case 0x4: case 0xB: case 0xD:
                currValue--;
                break;
            case 0x1: case 0x7: case 0x8: case 0xE:
                currValue++;
                break;
        }

        prevState = currState;
    }

    static inline uint8_t read() {
        return currValue >> 2;
    }

    static inline uint8_t readDiff() {
        auto curr = read();
        auto diff = curr - prevValue;
        prevValue = curr;
        return diff;
    }
};

template<typename A, typename B>
volatile uint8_t Encoder<A, B>::prevState = 0;

template<typename A, typename B>
volatile uint8_t Encoder<A, B>::prevValue = 0;

template<typename A, typename B>
volatile uint8_t Encoder<A, B>::currValue = 0;

#endif
