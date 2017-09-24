#define MAKE_AVR_PORT(portName, dirName, pinName, className) \
    class className {\
        public:\
            typedef uint8_t DataT;\
            static DataT read() {\
                return portName;\
            }\
            static void dirWrite(DataT value) {\
                dirName = value;\
            }\
            static void dirClear(DataT mask) {\
                dirName &= ~mask;\
            }\
            static void dirSet(DataT value) {\
                dirName |= value;\
            }\
            static void write(DataT value) {\
                portName = value;\
            }\
            static void clear(DataT mask) {\
                portName &= ~mask;\
            }\
            static void set(DataT value) {\
                portName |= value;\
            }\
            static void toggle(DataT value) {\
                portName ^= value;\
            }\
            static void clearSet(DataT mask, DataT value) {\
                printf("%s.clearSet: %X, %X\n", #portName, (DataT)~mask, value);\
                portName = (portName & ~mask) | value;\
            }\
            enum { width = sizeof(DataT) * 8 };\
    };\
    typedef TPin<className, 0> className ## 0;\
    typedef TPin<className, 1> className ## 1;\
    typedef TPin<className, 2> className ## 2;\
    typedef TPin<className, 3> className ## 3;\
    typedef TPin<className, 4> className ## 4;\
    typedef TPin<className, 5> className ## 5;\
    typedef TPin<className, 6> className ## 6;\
    typedef TPin<className, 7> className ## 7;\

template<class PORT, size_t PinNumber>
class TPin {
    public:
        static void set() {
            PORT::set(1 << PinNumber);
        }

        static void clr() {
            PORT::clear(1 << PinNumber);
        }

        static void tgl() {
            PORT::toggle(1 << PinNumber);
        }

        static void out() {
            PORT::dirSet(1 << PinNumber);
        }

        template<typename T>
        static void send(const T value) {
            if (value) set();
            else clr();
        }

        typedef PORT Port;
        enum { number = PinNumber };
};

MAKE_AVR_PORT(PORTA, DDRA, PINA, PortA)
MAKE_AVR_PORT(PORTB, DDRB, PINB, PortB)
MAKE_AVR_PORT(PORTC, DDRC, PINC, PortC)
MAKE_AVR_PORT(PORTD, DDRD, PIND, PortD)

